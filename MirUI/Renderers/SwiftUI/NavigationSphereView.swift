import SwiftUI

/// Classic CAD navigation sphere (NX / CATIA / Creo style), rendered as a
/// 3D-look globe. It visualizes the camera state (azimuth theta, polar
/// angle phi, distance) and emits camera requests through the Event Bus;
/// it does not own camera state or viewport handles.
///
/// Features:
/// - 8 equatorial direction zones (standard views + diagonals) that rotate
///   together with the camera azimuth, like the NX compass;
/// - north/south poles (Верх / Низ);
/// - a meridian with a live altitude marker;
/// - trackball drag on the sphere surface (free orbit);
/// - a rotation ring around the sphere (azimuth only);
/// - magnetic snapping to standard views when a drag ends near one;
/// - smooth animated camera transitions;
/// - view history (back button);
/// - context menu (all views, Fit All, projection toggle);
/// - keyboard shortcuts (F8 isometric, Cmd+1...6 standard views).
struct NavigationSphereView: View {
    var theta: Double
    var phi: Double
    var distance: Double
    var isOrthographic: Bool

    @State private var history: [OrbitSnapshot] = []
    @State private var hoveredZone: Int?

    private static let pi = Double.pi

    /// Azimuth angles (degrees) of the 8 equatorial zones, front first.
    private let zoneAzimuths: [Double] = [0, 45, 90, 135, 180, 225, 270, 315]

    /// Preset per equatorial zone (same order as zoneAzimuths).
    private let zonePresets: [MirCameraPreset] = [
        .front, .frontRight, .right, .backRight,
        .back, .backLeft, .left, .frontLeft
    ]

    var body: some View {
        VStack(spacing: 4) {
            sphere

            HStack(spacing: 6) {
                homeButton
                historyBackButton
                fitButton
            }
            .frame(height: 16)
        }
        .contentShape(Rectangle())
        .contextMenu { sphereMenu }
        .accessibilityElement(children: .combine)
        .accessibilityLabel("Навигационная сфера")
        .accessibilityValue(orientationDescription)
        .overlay { keyboardShortcutButtons }
    }

    // MARK: - Sphere

    /// Six direction surfaces: 4 compass belts (North / East / South / West)
    /// and 2 polar caps, separated by meridian arcs. The belts rotate with the
    /// camera azimuth like longitude bands; the labels show scene orientation.
    private struct CompassSector {
        let azimuth: Double
        let label: String
        let color: Color
    }

    private let sectors: [CompassSector] = [
        CompassSector(azimuth: 0, label: "СЕВЕР", color: Color(red: 0.30, green: 0.55, blue: 1.0).opacity(0.11)),
        CompassSector(azimuth: 90, label: "ВОСТОК", color: Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.09)),
        CompassSector(azimuth: 180, label: "ЮГ", color: Color(red: 0.55, green: 0.35, blue: 0.85).opacity(0.10)),
        CompassSector(azimuth: 270, label: "ЗАПАД", color: Color(red: 1.0, green: 0.70, blue: 0.20).opacity(0.09))
    ]

    private var sphere: some View {
        GeometryReader { proxy in
            let size = proxy.size
            let center = CGPoint(x: size.width / 2, y: size.height / 2)
            let radius = min(size.width / 2.55, size.height / 2.45)
            let active = activePreset

            ZStack {
                globe(radius: radius, center: center)
                compassLayer(radius: radius, center: center)
                meridian(radius: radius, center: center)
                latitudeLines(radius: radius, center: center)
                equator(radius: radius, center: center)
                altitudeMarker(radius: radius, center: center)
                ForEach(Array(zoneAzimuths.enumerated()), id: \.offset) { index, azimuth in
                    zoneView(
                        index: index,
                        azimuth: azimuth,
                        radius: radius,
                        center: center,
                        isActive: zonePresets[index] == active
                    )
                }
                northPoleButton(radius: radius, center: center)
                southPoleButton(radius: radius, center: center)
            }
            .frame(width: size.width, height: size.height)
        }
        .frame(width: 190, height: 152)
        .overlay(alignment: .topTrailing) {
            if let index = hoveredZone {
                zoneTooltip(index: index)
                    .padding(.top, 6)
                    .padding(.trailing, 18)
            }
        }
    }

    private func globe(radius: CGFloat, center: CGPoint) -> some View {
        Circle()
            .fill(
                RadialGradient(
                    colors: [
                        Color(red: 0.34, green: 0.44, blue: 0.62),
                        Color(red: 0.16, green: 0.22, blue: 0.34),
                        Color(red: 0.07, green: 0.10, blue: 0.17)
                    ],
                    center: UnitPoint(x: 0.36, y: 0.30),
                    startRadius: 2,
                    endRadius: radius
                )
            )
            .overlay {
                Circle()
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.95), lineWidth: 1)
            }
            .overlay {
                Ellipse()
                    .fill(Color.white.opacity(0.10))
                    .frame(width: radius * 0.55, height: radius * 0.28)
                    .offset(x: -radius * 0.42, y: -radius * 0.42)
                    .blur(radius: 3)
                    .allowsHitTesting(false)
            }
            .overlay {
                rotationRing(radius: radius)
            }
            .contentShape(Circle())
            .gesture(
                DragGesture(minimumDistance: 1)
                    .onChanged { value in
                        updateTrackball(value.translation)
                    }
                    .onEnded { _ in
                        snapToNearestViewIfClose()
                    }
            )
    }

    /// Outer ring: dragging it orbits the camera around the vertical axis.
    private func rotationRing(radius: CGFloat) -> some View {
        Circle()
            .stroke(Color.white.opacity(0.05), lineWidth: 10)
            .frame(width: (radius + 9) * 2, height: (radius + 9) * 2)
            .overlay {
                Circle()
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.55), lineWidth: 1)
                    .frame(width: (radius + 9) * 2, height: (radius + 9) * 2)
            }
            .contentShape(Circle())
            .gesture(
                DragGesture(minimumDistance: 1)
                    .onChanged { value in
                        updateAzimuth(value.translation.width)
                    }
                    .onEnded { _ in
                        snapToNearestViewIfClose()
                    }
            )
    }

    private func meridian(radius: CGFloat, center: CGPoint) -> some View {
        Ellipse()
            .stroke(Color.white.opacity(0.14), lineWidth: 1)
            .frame(width: radius * 2 * 0.40, height: radius * 2)
            .position(center)
            .allowsHitTesting(false)
    }

    /// Horizontal latitude ellipses (clock bands) at ±30°.
    private func latitudeLines(radius: CGFloat, center: CGPoint) -> some View {
        ZStack {
            latitudeLine(radius: radius, center: center, phi: 0.52, sign: -1)
            latitudeLine(radius: radius, center: center, phi: 0.52, sign: 1)
        }
        .allowsHitTesting(false)
    }

    private func latitudeLine(radius: CGFloat, center: CGPoint, phi: Double, sign: Double) -> some View {
        let pole = radius
        let rx = radius * CGFloat(cos(phi))
        let ry = radius * 0.40 * CGFloat(cos(phi))
        let cy = center.y - CGFloat(sign * sin(phi)) * pole
        return Ellipse()
            .stroke(Color.white.opacity(0.11), lineWidth: 0.8)
            .frame(width: rx * 2, height: ry * 2)
            .position(x: center.x, y: cy)
    }

    /// Six direction surfaces: 4 compass belts with labels and 2 polar caps.
    private func compassLayer(radius: CGFloat, center: CGPoint) -> some View {
        ZStack {
            ForEach(sectors, id: \.azimuth) { sector in
                sectorPath(
                    from: sector.azimuth - 45 * .pi / 180,
                    to: sector.azimuth + 45 * .pi / 180,
                    radius: radius,
                    center: center
                )
                .fill(sector.color)

                sectorPath(
                    from: sector.azimuth - 45 * .pi / 180,
                    to: sector.azimuth + 45 * .pi / 180,
                    radius: radius,
                    center: center
                )
                .stroke(Color.white.opacity(0.16), lineWidth: 0.8)

                Text(sector.label)
                    .font(.system(size: 8, weight: .bold, design: .rounded))
                    .foregroundStyle(.white.opacity(0.62))
                    .position(sectorLabelPoint(sector.azimuth, radius: radius, center: center))
            }

            polarCap(radius: radius, center: center, sign: -1)
            polarCap(radius: radius, center: center, sign: 1)
        }
        .allowsHitTesting(false)
    }

    /// A longitude belt between two meridians (full height, pole to pole).
    private func sectorPath(from a0: Double, to a1: Double, radius: CGFloat, center: CGPoint) -> Path {
        let rx = radius
        let ry = radius * 0.40
        let pole = radius
        return Path { path in
            func point(_ a: Double, _ phi: Double) -> CGPoint {
                CGPoint(
                    x: center.x + CGFloat(sin(a - theta)) * rx * CGFloat(cos(phi)),
                    y: center.y - ry * CGFloat(cos(phi)) * CGFloat(cos(a - theta)) - CGFloat(sin(phi)) * pole
                )
            }
            let steps = 12
            for i in 0...steps {
                let phi = -.pi / 2 + Double(i) * .pi / Double(steps)
                let p = point(a0, phi)
                if i == 0 { path.move(to: p) } else { path.addLine(to: p) }
            }
            for i in 0...steps {
                let phi = .pi / 2 - Double(i) * .pi / Double(steps)
                path.addLine(to: point(a1, phi))
            }
            path.closeSubpath()
        }
    }

    private func sectorLabelPoint(_ azimuth: Double, radius: CGFloat, center: CGPoint) -> CGPoint {
        let rx = radius
        let ry = radius * 0.40
        let pole = radius
        let phi = 0.30
        return CGPoint(
            x: center.x + CGFloat(sin(azimuth - theta)) * rx * CGFloat(cos(phi)),
            y: center.y - ry * CGFloat(cos(phi)) * CGFloat(cos(azimuth - theta)) - CGFloat(sin(phi)) * pole
        )
    }

    /// Polar cap surface (top/bottom), one of the six direction surfaces.
    private func polarCap(radius: CGFloat, center: CGPoint, sign: CGFloat) -> some View {
        Ellipse()
            .fill(Color.white.opacity(0.06))
            .frame(width: radius * 0.72, height: radius * 0.16)
            .position(x: center.x, y: center.y - sign * radius * 0.94)
    }

    private func equator(radius: CGFloat, center: CGPoint) -> some View {
        Ellipse()
            .stroke(Color.white.opacity(0.30), lineWidth: 1)
            .frame(width: radius * 2, height: radius * 2 * 0.40)
            .position(center)
            .allowsHitTesting(false)
    }

    /// Live marker of the current polar angle on the meridian.
    private func altitudeMarker(radius: CGFloat, center: CGPoint) -> some View {
        let y = center.y - CGFloat(cos(phi)) * radius
        let point = CGPoint(x: center.x, y: y)
        return ZStack {
            Path { path in
                path.move(to: center)
                path.addLine(to: point)
            }
            .stroke(MirTheme.Colors.accentBright.opacity(0.5), lineWidth: 1)
            Circle()
                .fill(MirTheme.Colors.accentBright)
                .frame(width: 6, height: 6)
                .overlay {
                    Circle()
                        .stroke(.white.opacity(0.8), lineWidth: 1)
                }
        }
        .position(point)
        .allowsHitTesting(false)
    }

    /// One of the 8 equatorial direction zones (world azimuth).
    private func zoneView(index: Int, azimuth: Double, radius: CGFloat, center: CGPoint, isActive: Bool) -> some View {
        let angle = deg2rad(azimuth) - theta
        let rx = radius
        let ry = radius * 0.40
        let point = CGPoint(
            x: center.x + CGFloat(sin(angle)) * rx,
            y: center.y - CGFloat(cos(angle)) * ry
        )
        let isHovered = hoveredZone == index
        let isCardinal = azimuth.truncatingRemainder(dividingBy: 180) == 0

        return ZStack {
            Circle()
                .fill(
                    isActive
                        ? MirTheme.Colors.accentBright
                        : (isHovered ? Color.white.opacity(0.75) : Color.white.opacity(0.22))
                )
                .frame(width: isActive ? 11 : (isHovered ? 10 : 7), height: isActive ? 11 : (isHovered ? 10 : 7))
                .overlay {
                    Circle()
                        .stroke(
                            isActive ? .white.opacity(0.9) : MirTheme.Colors.panelBorder.opacity(0.8),
                            lineWidth: isActive ? 1.4 : 0.8
                        )
                }
            if isCardinal {
                Text("\(Int(azimuth))°")
                    .font(.system(size: 6, weight: .semibold))
                    .foregroundStyle(isActive ? MirTheme.Colors.accentBright : Color.white.opacity(0.5))
                    .position(
                        x: center.x + CGFloat(sin(angle)) * (radius + 11),
                        y: center.y - CGFloat(cos(angle)) * (radius * 0.40 + 7)
                    )
            }
        }
        .position(point)
        .contentShape(Circle())
        .onHover { hovering in
            hoveredZone = hovering ? index : nil
        }
        .onTapGesture {
            pushHistory()
            Mir4DSetActiveCameraPreset(zonePresets[index], animated: true)
        }
        .help(zonePresets[index].titleRU)
        .accessibilityLabel(zonePresets[index].titleRU)
    }

    private func zoneTooltip(index: Int) -> some View {
        Text(zonePresets[index].titleRU)
            .font(.system(size: 7.5, weight: .semibold, design: .rounded))
            .padding(.horizontal, 6)
            .padding(.vertical, 2)
            .background(.black.opacity(0.75), in: RoundedRectangle(cornerRadius: 4))
            .foregroundStyle(.white)
            .allowsHitTesting(false)
    }

    private func poleButton(_ label: String, center: CGPoint, yOffset: CGFloat, preset: MirCameraPreset) -> some View {
        Circle()
            .fill(MirTheme.Colors.accentBright.opacity(0.28))
            .frame(width: 13, height: 13)
            .overlay {
                Circle()
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.8), lineWidth: 1)
            }
            .overlay {
                Text(label)
                    .font(.system(size: 6.5, weight: .bold))
                    .foregroundStyle(Color.white.opacity(0.9))
            }
            .position(x: center.x, y: center.y + yOffset)
            .contentShape(Circle())
            .onTapGesture {
                pushHistory()
                Mir4DSetActiveCameraPreset(preset, animated: true)
            }
            .help(preset.titleRU)
    }

    private func northPoleButton(radius: CGFloat, center: CGPoint) -> some View {
        poleButton("В", center: center, yOffset: -radius - 8, preset: .top)
    }

    private func southPoleButton(radius: CGFloat, center: CGPoint) -> some View {
        poleButton("Н", center: center, yOffset: radius + 8, preset: .bottom)
    }

    // MARK: - Trackball

    private func updateTrackball(_ translation: CGSize) {
        let sensitivity = 0.013
        let newTheta = theta + Double(translation.width) * sensitivity
        let newPhi = min(
            Double.pi - 0.03,
            max(0.03, phi - Double(translation.height) * sensitivity)
        )
        Mir4DSetCameraOrbit(theta: newTheta, phi: newPhi, distance: distance, animated: false)
    }

    private func updateAzimuth(_ deltaX: CGFloat) {
        let sensitivity = 0.017
        let newTheta = theta + Double(deltaX) * sensitivity
        Mir4DSetCameraOrbit(theta: newTheta, phi: phi, distance: distance, animated: false)
    }

    /// Magnetic snapping: when the drag ends near a standard azimuth or
    /// polar angle, smoothly settle onto the nearest preset view.
    private func snapToNearestViewIfClose() {
        let nearestAzimuth = (theta / (Double.pi / 4)).rounded() * (Double.pi / 4)
        let azimuthDelta = abs(nearestAzimuth - theta)
        let snapThreshold = deg2rad(6.0)

        let phiSnapValues: [Double] = [
            1e-4,
            0.6154797086703874,  // isometric
            0.7853981633974483,  // edge up
            0.9553166181245093,  // corner up
            1.1,                 // faces
            2.186276035465284,   // corner down
            2.356194490192345,   // edge down
            Double.pi - 1e-4
        ]
        let nearestPhi = phiSnapValues.min {
            abs($0 - phi) < abs($1 - phi)
        } ?? phi
        let phiDelta = abs(nearestPhi - phi)

        guard azimuthDelta < snapThreshold || phiDelta < snapThreshold else {
            return
        }

        pushHistory()
        Mir4DSetActiveCameraPreset(activePreset, animated: true)
    }

    // MARK: - History

    private func pushHistory() {
        history.append(OrbitSnapshot(theta: theta, phi: phi, distance: distance))
        if history.count > 24 {
            history.removeFirst(history.count - 24)
        }
    }

    // MARK: - Active view detection

    /// Closest preset direction to the current camera orientation.
    private var activePreset: MirCameraPreset {
        let cameraDir = SIMD3<Double>(
            sin(phi) * sin(theta),
            cos(phi),
            sin(phi) * cos(theta)
        )
        guard cameraDir.x.isFinite, cameraDir.y.isFinite, cameraDir.z.isFinite else {
            return .isometric
        }
        let normalized = cameraDir.normalized
        var best = MirCameraPreset.front
        var bestDot = -Double.infinity
        for candidate in MirCameraPreset.allCases {
            let dot = normalized.x * candidate.direction.x
                + normalized.y * candidate.direction.y
                + normalized.z * candidate.direction.z
            if dot > bestDot {
                bestDot = dot
                best = candidate
            }
        }
        return best
    }

    private var orientationDescription: String {
        String(format: "θ %.2f°, φ %.2f°, d %.2f", theta * 180.0 / .pi, phi * 180.0 / .pi, distance)
    }

    private func deg2rad(_ degrees: Double) -> Double {
        degrees * .pi / 180.0
    }

    // MARK: - Buttons

    private func roundButton(icon: String, color: Color, help: String, action: @escaping () -> Void) -> some View {
        Image(systemName: icon)
            .font(.system(size: 10, weight: .semibold))
            .foregroundStyle(color)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(
                RoundedRectangle(cornerRadius: 5)
                    .fill(color.opacity(0.22))
            )
            .overlay {
                RoundedRectangle(cornerRadius: 5)
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 0.8)
            }
            .contentShape(RoundedRectangle(cornerRadius: 5))
            .onTapGesture(perform: action)
            .help(help)
    }

    private var homeButton: some View {
        roundButton(
            icon: "house.fill",
            color: activePreset == .isometric ? MirTheme.Colors.accentBright : Color.white.opacity(0.85),
            help: "Домой — изометрия (F8)"
        ) {
            pushHistory()
            Mir4DSetActiveCameraPreset(.isometric, animated: true)
        }
    }

    private var historyBackButton: some View {
        let enabled = !history.isEmpty
        return Image(systemName: "arrow.uturn.backward.circle")
            .font(.system(size: 10, weight: .semibold))
            .foregroundStyle(enabled ? Color.white.opacity(0.85) : Color.white.opacity(0.3))
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(
                RoundedRectangle(cornerRadius: 5)
                    .fill(enabled ? Color.white.opacity(0.10) : Color.white.opacity(0.03))
            )
            .overlay {
                RoundedRectangle(cornerRadius: 5)
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 0.8)
            }
            .contentShape(RoundedRectangle(cornerRadius: 5))
            .onTapGesture {
                guard let snapshot = history.popLast() else { return }
                Mir4DSetCameraOrbit(
                    theta: snapshot.theta,
                    phi: snapshot.phi,
                    distance: snapshot.distance,
                    animated: true
                )
            }
            .help("Предыдущий вид")
            .accessibilityLabel("Предыдущий вид")
    }

    private var fitButton: some View {
        roundButton(
            icon: "arrow.up.left.and.down.right.magnifyingglass",
            color: Color.white.opacity(0.85),
            help: "Вписать всё"
        ) {
            Mir4DRequestCameraFit()
        }
    }

    // MARK: - Context menu

    @ViewBuilder
    private var sphereMenu: some View {
        menuItem("Спереди", preset: .front)
        menuItem("Сзади", preset: .back)
        menuItem("Слева", preset: .left)
        menuItem("Справа", preset: .right)
        menuItem("Сверху", preset: .top)
        menuItem("Снизу", preset: .bottom)
        menuItem("Изометрия", preset: .isometric)

        Divider()

        Button("Вписать всё") {
            Mir4DRequestCameraFit()
        }
        Button(isOrthographic ? "Перспективная проекция" : "Ортографическая проекция") {
            toggleProjection()
        }
    }

    private func menuItem(_ title: String, preset: MirCameraPreset) -> some View {
        Button(title) {
            pushHistory()
            Mir4DSetActiveCameraPreset(preset, animated: true)
        }
    }

    private func toggleProjection() {
        NotificationCenter.default.post(
            name: .mir4DCameraProjectionRequested,
            object: nil,
            userInfo: ["projection": isOrthographic ? 0 : 1]
        )
    }

    // MARK: - Keyboard shortcuts

    /// Invisible buttons that surface the sphere commands as keyboard
    /// shortcuts: F8 = isometric, Cmd+1...6 = standard views.
    private var keyboardShortcutButtons: some View {
        ZStack {
            shortcutButton(.isometric, key: KeyEquivalent("8"), modifiers: [])
            shortcutButton(.front, key: KeyEquivalent("1"), modifiers: .command)
            shortcutButton(.back, key: KeyEquivalent("2"), modifiers: .command)
            shortcutButton(.left, key: KeyEquivalent("3"), modifiers: .command)
            shortcutButton(.right, key: KeyEquivalent("4"), modifiers: .command)
            shortcutButton(.top, key: KeyEquivalent("5"), modifiers: .command)
            shortcutButton(.bottom, key: KeyEquivalent("6"), modifiers: .command)
        }
        .frame(width: 1, height: 1)
        .opacity(0.001)
        .allowsHitTesting(false)
    }

    private func shortcutButton(_ preset: MirCameraPreset, key: KeyEquivalent, modifiers: EventModifiers) -> some View {
        Button("") {
            pushHistory()
            Mir4DSetActiveCameraPreset(preset, animated: true)
        }
        .keyboardShortcut(key, modifiers: modifiers)
        .frame(width: 1, height: 1)
    }
}

/// A camera orbit snapshot used by the view history.
private struct OrbitSnapshot {
    let theta: Double
    let phi: Double
    let distance: Double
}
