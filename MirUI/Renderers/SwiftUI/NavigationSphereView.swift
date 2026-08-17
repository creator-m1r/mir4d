import SwiftUI

/// МИР 4D navigation sphere.
///
/// The sphere is a navigation instrument, not a second camera controller.
/// It visualizes the current camera orientation and emits camera requests
/// through the existing camera/event boundary.
///
/// UX rules:
/// - the sphere has one clear center and stable world directions;
/// - labels never drift vertically with latitude math;
/// - trackball drag uses the drag-start camera state, avoiding cumulative drift;
/// - the outer ring controls azimuth only;
/// - standard-view snapping chooses the actual nearest preset;
/// - history stores camera states before explicit navigation changes;
/// - visual layers never intercept pointer input intended for controls.
struct NavigationSphereView: View {
    var theta: Double
    var phi: Double
    var distance: Double
    var isOrthographic: Bool

    @State private var history: [OrbitSnapshot] = []
    @State private var hoveredZone: Int?
    @State private var isDraggingSphere = false
    @State private var isDraggingRing = false
    @State private var dragStartTheta = 0.0
    @State private var dragStartPhi = Double.pi / 2
    @State private var dragStartRingTheta = 0.0

    private let zoneAzimuths: [Double] = [0, 45, 90, 135, 180, 225, 270, 315]
    private let zonePresets: [MirCameraPreset] = [
        .front, .frontRight, .right, .backRight,
        .back, .backLeft, .left, .frontLeft
    ]

    var body: some View {
        VStack(spacing: 5) {
            sphere

            HStack(spacing: 5) {
                homeButton
                historyBackButton
                fitButton
            }
            .frame(height: 18)
        }
        .frame(width: 204)
        .contentShape(Rectangle())
        .contextMenu { sphereMenu }
        .accessibilityElement(children: .combine)
        .accessibilityLabel("Навигационная сфера")
        .accessibilityValue(orientationDescription)
        .overlay { keyboardShortcutButtons }
    }

    // MARK: - Sphere

    private var sphere: some View {
        GeometryReader { proxy in
            let size = proxy.size
            let center = CGPoint(x: size.width / 2, y: size.height / 2)
            let radius = min(size.width * 0.39, size.height * 0.43)

            ZStack {
                sphereBody(radius: radius, center: center)
                longitudeLines(radius: radius, center: center)
                latitudeLines(radius: radius, center: center)
                compassLabels(radius: radius, center: center)
                equator(radius: radius, center: center)
                altitudeMarker(radius: radius, center: center)

                ForEach(Array(zoneAzimuths.enumerated()), id: \.offset) { index, azimuth in
                    zoneView(
                        index: index,
                        azimuth: azimuth,
                        radius: radius,
                        center: center,
                        isActive: zonePresets[index] == activePreset
                    )
                }

                poleButton("В", center: center, yOffset: -radius - 9, preset: .top)
                poleButton("Н", center: center, yOffset: radius + 9, preset: .bottom)
            }
            .frame(width: size.width, height: size.height)
        }
        .frame(width: 204, height: 158)
        .overlay(alignment: .topTrailing) {
            if let index = hoveredZone {
                zoneTooltip(index: index)
                    .padding(.top, 5)
                    .padding(.trailing, 8)
            }
        }
    }

    private func sphereBody(radius: CGFloat, center: CGPoint) -> some View {
        Circle()
            .fill(
                RadialGradient(
                    colors: [
                        Color(red: 0.32, green: 0.43, blue: 0.62),
                        Color(red: 0.15, green: 0.21, blue: 0.33),
                        Color(red: 0.055, green: 0.08, blue: 0.14)
                    ],
                    center: UnitPoint(x: 0.34, y: 0.27),
                    startRadius: 1,
                    endRadius: radius
                )
            )
            .overlay {
                Circle()
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.95), lineWidth: 1)
            }
            .overlay(alignment: .topLeading) {
                Ellipse()
                    .fill(.white.opacity(0.10))
                    .frame(width: radius * 0.58, height: radius * 0.27)
                    .offset(x: radius * 0.16, y: radius * 0.16)
                    .blur(radius: 3)
                    .allowsHitTesting(false)
            }
            .contentShape(Circle())
            .gesture(trackballGesture)
            .overlay {
                rotationRing(radius: radius)
            }
    }

    private var trackballGesture: some Gesture {
        DragGesture(minimumDistance: 2)
            .onChanged { value in
                if !isDraggingSphere {
                    isDraggingSphere = true
                    dragStartTheta = theta
                    dragStartPhi = phi
                }

                let sensitivity = 0.0105
                let newTheta = normalizedAngle(
                    dragStartTheta + Double(value.translation.width) * sensitivity
                )
                let newPhi = min(
                    Double.pi - 0.035,
                    max(0.035, dragStartPhi - Double(value.translation.height) * sensitivity)
                )

                Mir4DSetCameraOrbit(
                    theta: newTheta,
                    phi: newPhi,
                    distance: distance,
                    animated: false
                )
            }
            .onEnded { _ in
                isDraggingSphere = false
                snapToNearestViewIfClose()
            }
    }

    /// The ring is intentionally separate from the trackball: horizontal drag
    /// rotates only the azimuth and cannot unexpectedly change camera altitude.
    private func rotationRing(radius: CGFloat) -> some View {
        Circle()
            .stroke(Color.white.opacity(0.045), lineWidth: 11)
            .frame(width: (radius + 9) * 2, height: (radius + 9) * 2)
            .overlay {
                Circle()
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.52), lineWidth: 1)
                    .frame(width: (radius + 9) * 2, height: (radius + 9) * 2)
            }
            .contentShape(Circle().strokeBorder(lineWidth: 11))
            .gesture(
                DragGesture(minimumDistance: 2)
                    .onChanged { value in
                        if !isDraggingRing {
                            isDraggingRing = true
                            dragStartRingTheta = theta
                        }

                        let sensitivity = 0.014
                        Mir4DSetCameraOrbit(
                            theta: normalizedAngle(dragStartRingTheta + Double(value.translation.width) * sensitivity),
                            phi: phi,
                            distance: distance,
                            animated: false
                        )
                    }
                    .onEnded { _ in
                        isDraggingRing = false
                        snapToNearestViewIfClose()
                    }
            )
            .allowsHitTesting(true)
    }

    // MARK: - Globe grid

    private func longitudeLines(radius: CGFloat, center: CGPoint) -> some View {
        ZStack {
            ForEach([0.28, 0.52, 0.76, 1.0], id: \.self) { width in
                Ellipse()
                    .stroke(.white.opacity(width == 1.0 ? 0.16 : 0.075), lineWidth: width == 1.0 ? 1 : 0.7)
                    .frame(width: radius * 2 * CGFloat(width), height: radius * 2)
                    .position(center)
            }
        }
        .allowsHitTesting(false)
    }

    private func latitudeLines(radius: CGFloat, center: CGPoint) -> some View {
        ZStack {
            latitudeLine(radius: radius, center: center, latitude: 30)
            latitudeLine(radius: radius, center: center, latitude: -30)
        }
        .allowsHitTesting(false)
    }

    private func latitudeLine(radius: CGFloat, center: CGPoint, latitude: Double) -> some View {
        let lat = deg2rad(latitude)
        let verticalOffset = -CGFloat(sin(lat)) * radius * 0.78
        let width = radius * 2 * CGFloat(cos(lat))
        let height = max(7, radius * 0.24 * CGFloat(cos(lat)))

        return Ellipse()
            .stroke(.white.opacity(0.095), lineWidth: 0.7)
            .frame(width: width, height: height)
            .position(x: center.x, y: center.y + verticalOffset)
    }

    private func equator(radius: CGFloat, center: CGPoint) -> some View {
        Ellipse()
            .stroke(.white.opacity(0.26), lineWidth: 0.9)
            .frame(width: radius * 2, height: radius * 0.70)
            .position(center)
            .allowsHitTesting(false)
    }

    private func compassLabels(radius: CGFloat, center: CGPoint) -> some View {
        ZStack {
            compassLabel("С", azimuth: 0, radius: radius, center: center)
            compassLabel("В", azimuth: 90, radius: radius, center: center)
            compassLabel("Ю", azimuth: 180, radius: radius, center: center)
            compassLabel("З", azimuth: 270, radius: radius, center: center)
        }
        .allowsHitTesting(false)
    }

    private func compassLabel(_ text: String, azimuth: Double, radius: CGFloat, center: CGPoint) -> some View {
        let a = deg2rad(azimuth) - theta
        let x = center.x + CGFloat(sin(a)) * radius * 0.91
        let y = center.y - CGFloat(cos(a)) * radius * 0.40

        return Text(text)
            .font(.system(size: 8, weight: .bold, design: .rounded))
            .foregroundStyle(.white.opacity(0.72))
            .position(x: x, y: y)
    }

    /// Current altitude marker. The marker is drawn in the sphere's local
    /// coordinate system and is no longer accidentally translated twice.
    private func altitudeMarker(radius: CGFloat, center: CGPoint) -> some View {
        let normalizedPhi = min(Double.pi, max(0, phi))
        let y = center.y - CGFloat(cos(normalizedPhi)) * radius * 0.92
        let point = CGPoint(x: center.x, y: y)

        return ZStack {
            Path { path in
                path.move(to: center)
                path.addLine(to: point)
            }
            .stroke(MirTheme.Colors.accentBright.opacity(0.42), lineWidth: 1)

            Circle()
                .fill(MirTheme.Colors.accentBright)
                .frame(width: 6, height: 6)
                .overlay {
                    Circle().stroke(.white.opacity(0.85), lineWidth: 1)
                }
                .position(point)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .allowsHitTesting(false)
    }

    // MARK: - Direction controls

    private func zoneView(
        index: Int,
        azimuth: Double,
        radius: CGFloat,
        center: CGPoint,
        isActive: Bool
    ) -> some View {
        let angle = deg2rad(azimuth) - theta
        let point = CGPoint(
            x: center.x + CGFloat(sin(angle)) * radius,
            y: center.y - CGFloat(cos(angle)) * radius * 0.40
        )
        let isHovered = hoveredZone == index
        let isCardinal = azimuth.truncatingRemainder(dividingBy: 90) == 0

        return Circle()
            .fill(
                isActive
                    ? MirTheme.Colors.accentBright
                    : (isHovered ? .white.opacity(0.85) : .white.opacity(0.25))
            )
            .frame(width: isActive ? 12 : (isHovered ? 10 : 7), height: isActive ? 12 : (isHovered ? 10 : 7))
            .overlay {
                Circle()
                    .stroke(isActive ? .white.opacity(0.95) : MirTheme.Colors.panelBorder.opacity(0.8), lineWidth: isActive ? 1.4 : 0.8)
            }
            .overlay {
                if isCardinal {
                    Text("\(Int(azimuth))°")
                        .font(.system(size: 5.5, weight: .semibold))
                        .foregroundStyle(.white.opacity(0.42))
                        .offset(y: -10)
                }
            }
            .position(point)
            .contentShape(Circle())
            .onHover { hoveredZone = $0 ? index : nil }
            .onTapGesture {
                pushHistory()
                Mir4DSetActiveCameraPreset(zonePresets[index], animated: true)
            }
            .help(zonePresets[index].titleRU)
            .accessibilityLabel(zonePresets[index].titleRU)
    }

    private func poleButton(_ label: String, center: CGPoint, yOffset: CGFloat, preset: MirCameraPreset) -> some View {
        Circle()
            .fill(MirTheme.Colors.accentBright.opacity(0.28))
            .frame(width: 15, height: 15)
            .overlay {
                Circle().stroke(MirTheme.Colors.panelBorder.opacity(0.85), lineWidth: 1)
            }
            .overlay {
                Text(label)
                    .font(.system(size: 7, weight: .bold))
                    .foregroundStyle(.white.opacity(0.92))
            }
            .position(x: center.x, y: center.y + yOffset)
            .contentShape(Circle())
            .onTapGesture {
                pushHistory()
                Mir4DSetActiveCameraPreset(preset, animated: true)
            }
            .help(preset.titleRU)
            .accessibilityLabel(preset.titleRU)
    }

    private func zoneTooltip(index: Int) -> some View {
        Text(zonePresets[index].titleRU)
            .font(.system(size: 7.5, weight: .semibold, design: .rounded))
            .padding(.horizontal, 6)
            .padding(.vertical, 2)
            .background(.black.opacity(0.78), in: RoundedRectangle(cornerRadius: 4))
            .foregroundStyle(.white)
            .allowsHitTesting(false)
    }

    // MARK: - Camera interaction

    private func snapToNearestViewIfClose() {
        let current = cameraDirection(theta: theta, phi: phi).normalized
        var nearest: MirCameraPreset = .front
        var bestDot = -Double.infinity

        for preset in MirCameraPreset.allCases {
            let direction = preset.direction.normalized
            let dot = current.x * direction.x + current.y * direction.y + current.z * direction.z
            if dot > bestDot {
                bestDot = dot
                nearest = preset
            }
        }

        // ~8 degrees angular threshold. The old implementation could call
        // activePreset instead of the calculated nearest preset, so it often
        // snapped to the current view and appeared not to work.
        guard bestDot >= cos(deg2rad(8)) else { return }

        if nearest != activePreset {
            pushHistory()
            Mir4DSetActiveCameraPreset(nearest, animated: true)
        }
    }

    private func cameraDirection(theta: Double, phi: Double) -> SIMD3<Double> {
        SIMD3(
            sin(phi) * sin(theta),
            cos(phi),
            sin(phi) * cos(theta)
        )
    }

    private func normalizedAngle(_ value: Double) -> Double {
        var angle = value.truncatingRemainder(dividingBy: Double.pi * 2)
        if angle <= -Double.pi { angle += Double.pi * 2 }
        if angle > Double.pi { angle -= Double.pi * 2 }
        return angle
    }

    private func deg2rad(_ degrees: Double) -> Double {
        degrees * .pi / 180
    }

    // MARK: - History / state

    private func pushHistory() {
        history.append(OrbitSnapshot(theta: theta, phi: phi, distance: distance))
        if history.count > 24 {
            history.removeFirst(history.count - 24)
        }
    }

    private var activePreset: MirCameraPreset {
        let current = cameraDirection(theta: theta, phi: phi).normalized
        var best = MirCameraPreset.front
        var bestDot = -Double.infinity

        for candidate in MirCameraPreset.allCases {
            let d = candidate.direction.normalized
            let dot = current.x * d.x + current.y * d.y + current.z * d.z
            if dot > bestDot {
                bestDot = dot
                best = candidate
            }
        }
        return best
    }

    private var orientationDescription: String {
        String(
            format: "Азимут %.0f°, наклон %.0f°, расстояние %.2f",
            theta * 180 / .pi,
            phi * 180 / .pi,
            distance
        )
    }

    // MARK: - Buttons

    private func roundButton(icon: String, color: Color, help: String, action: @escaping () -> Void) -> some View {
        Image(systemName: icon)
            .font(.system(size: 10, weight: .semibold))
            .foregroundStyle(color)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(RoundedRectangle(cornerRadius: 5).fill(color.opacity(0.16)))
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
            color: activePreset == .isometric ? MirTheme.Colors.accentBright : .white.opacity(0.85),
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
            .foregroundStyle(enabled ? .white.opacity(0.85) : .white.opacity(0.3))
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(RoundedRectangle(cornerRadius: 5).fill(enabled ? .white.opacity(0.10) : .white.opacity(0.03)))
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
            color: .white.opacity(0.85),
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

private struct OrbitSnapshot {
    let theta: Double
    let phi: Double
    let distance: Double
}
