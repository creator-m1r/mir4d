import SwiftUI

/// МИР 4D navigation sphere.
///
/// The sphere is a navigation instrument, not a second camera controller.
/// World directions are projected from the actual camera basis derived from
/// theta/phi, so the sphere remains aligned with the viewport at every angle.
struct NavigationSphereView: View {
    var theta: Double
    var phi: Double
    var distance: Double
    var isOrthographic: Bool

    @State private var history: [OrbitSnapshot] = []
    @State private var dragStartTheta = 0.0
    @State private var dragStartPhi = Double.pi / 2
    @State private var dragStartRingTheta = 0.0
    @State private var draggingSphere = false
    @State private var draggingRing = false
    @State private var hoveredPreset: MirCameraPreset?

    private let directions: [(MirCameraPreset, SIMD3<Double>)] = [
        (.front, SIMD3(0, 0, 1)),
        (.frontRight, SIMD3(1, 0, 1).normalized),
        (.right, SIMD3(1, 0, 0)),
        (.backRight, SIMD3(1, 0, -1).normalized),
        (.back, SIMD3(0, 0, -1)),
        (.backLeft, SIMD3(-1, 0, -1).normalized),
        (.left, SIMD3(-1, 0, 0)),
        (.frontLeft, SIMD3(-1, 0, 1).normalized)
    ]

    var body: some View {
        VStack(spacing: 5) {
            GeometryReader { proxy in
                let center = CGPoint(x: proxy.size.width / 2, y: proxy.size.height / 2)
                let radius = min(proxy.size.width * 0.38, proxy.size.height * 0.43)

                ZStack {
                    sphereSurface(radius: radius)
                    sphereGrid(radius: radius)
                    worldDirectionMarkers(center: center, radius: radius)
                    currentCameraMarker(center: center, radius: radius)
                    poleControls(center: center, radius: radius)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .contentShape(Circle())
                .gesture(trackballGesture)
                .overlay { azimuthRing(radius: radius) }
            }
            .frame(width: 204, height: 158)

            HStack(spacing: 5) {
                sphereButton(icon: "house.fill", help: "Изометрия (F8)") {
                    pushHistory()
                    Mir4DSetActiveCameraPreset(.isometric, animated: true)
                }
                sphereButton(icon: "arrow.uturn.backward.circle", help: "Предыдущий вид", disabled: history.isEmpty) {
                    guard let snapshot = history.popLast() else { return }
                    Mir4DSetCameraOrbit(theta: snapshot.theta, phi: snapshot.phi, distance: snapshot.distance, animated: true)
                }
                sphereButton(icon: "arrow.up.left.and.down.right.magnifyingglass", help: "Вписать всё") {
                    Mir4DRequestCameraFit()
                }
            }
            .frame(height: 18)
        }
        .frame(width: 204)
        .contextMenu { menu }
        .accessibilityElement(children: .combine)
        .accessibilityLabel("Навигационная сфера")
        .accessibilityValue(orientationDescription)
    }

    private func sphereSurface(radius: CGFloat) -> some View {
        Circle()
            .fill(
                RadialGradient(
                    colors: [
                        Color(red: 0.30, green: 0.43, blue: 0.64),
                        Color(red: 0.13, green: 0.20, blue: 0.32),
                        Color(red: 0.045, green: 0.07, blue: 0.12)
                    ],
                    center: UnitPoint(x: 0.34, y: 0.28),
                    startRadius: 1,
                    endRadius: radius
                )
            )
            .overlay(Circle().stroke(MirTheme.Colors.panelBorder.opacity(0.95), lineWidth: 1))
            .overlay(alignment: .topLeading) {
                Ellipse()
                    .fill(.white.opacity(0.08))
                    .frame(width: radius * 0.55, height: radius * 0.25)
                    .offset(x: radius * 0.16, y: radius * 0.15)
                    .blur(radius: 3)
                    .allowsHitTesting(false)
            }
    }

    private func sphereGrid(radius: CGFloat) -> some View {
        ZStack {
            Circle().stroke(.white.opacity(0.13), lineWidth: 0.8)
            Ellipse().stroke(.white.opacity(0.08), lineWidth: 0.7).frame(width: radius * 0.58, height: radius * 2)
            Ellipse().stroke(.white.opacity(0.08), lineWidth: 0.7).frame(width: radius * 1.15, height: radius * 2)
            Ellipse().stroke(.white.opacity(0.12), lineWidth: 0.8).frame(width: radius * 2, height: radius * 0.72)
            Ellipse().stroke(.white.opacity(0.07), lineWidth: 0.7).frame(width: radius * 2, height: radius * 0.42).offset(y: -radius * 0.38)
            Ellipse().stroke(.white.opacity(0.07), lineWidth: 0.7).frame(width: radius * 2, height: radius * 0.42).offset(y: radius * 0.38)
        }
        .allowsHitTesting(false)
    }

    private func worldDirectionMarkers(center: CGPoint, radius: CGFloat) -> some View {
        ZStack {
            ForEach(Array(directions.enumerated()), id: \.offset) { _, item in
                let projected = project(item.1, radius: radius)
                let visible = max(0.18, min(1.0, (projected.depth + 1) * 0.5))
                let active = activePreset == item.0
                let hovered = hoveredPreset == item.0

                Circle()
                    .fill(active ? MirTheme.Colors.accentBright : .white.opacity(0.24 + 0.35 * visible))
                    .frame(width: active ? 11 : (hovered ? 10 : 7), height: active ? 11 : (hovered ? 10 : 7))
                    .overlay(Circle().stroke(active ? .white.opacity(0.95) : .white.opacity(0.35), lineWidth: active ? 1.2 : 0.6))
                    .position(x: center.x + projected.x, y: center.y - projected.y)
                    .opacity(visible)
                    .contentShape(Circle())
                    .onHover { hoveredPreset = $0 ? item.0 : (hoveredPreset == item.0 ? nil : hoveredPreset) }
                    .onTapGesture {
                        pushHistory()
                        Mir4DSetActiveCameraPreset(item.0, animated: true)
                    }
                    .help(item.0.titleRU)
                    .accessibilityLabel(item.0.titleRU)
            }

            compassLabel("С", direction: SIMD3(0, 0, 1), center: center, radius: radius)
            compassLabel("В", direction: SIMD3(1, 0, 0), center: center, radius: radius)
            compassLabel("Ю", direction: SIMD3(0, 0, -1), center: center, radius: radius)
            compassLabel("З", direction: SIMD3(-1, 0, 0), center: center, radius: radius)
        }
    }

    private func compassLabel(_ text: String, direction: SIMD3<Double>, center: CGPoint, radius: CGFloat) -> some View {
        let p = project(direction, radius: radius * 0.88)
        return Text(text)
            .font(.system(size: 8, weight: .bold, design: .rounded))
            .foregroundStyle(.white.opacity(0.45 + 0.4 * ((p.depth + 1) * 0.5)))
            .position(x: center.x + p.x, y: center.y - p.y)
            .allowsHitTesting(false)
    }

    private func currentCameraMarker(center: CGPoint, radius: CGFloat) -> some View {
        let direction = cameraDirection(theta: theta, phi: phi)
        let p = project(direction, radius: radius * 0.78)
        return ZStack {
            Path { path in
                path.move(to: center)
                path.addLine(to: CGPoint(x: center.x + p.x, y: center.y - p.y))
            }
            .stroke(MirTheme.Colors.accentBright.opacity(0.5), lineWidth: 1)
            Circle()
                .fill(MirTheme.Colors.accentBright)
                .frame(width: 7, height: 7)
                .overlay(Circle().stroke(.white.opacity(0.9), lineWidth: 1))
                .position(x: center.x + p.x, y: center.y - p.y)
        }
        .allowsHitTesting(false)
    }

    private func poleControls(center: CGPoint, radius: CGFloat) -> some View {
        ZStack {
            pole("В", preset: .top, direction: SIMD3(0, 1, 0), center: center, radius: radius)
            pole("Н", preset: .bottom, direction: SIMD3(0, -1, 0), center: center, radius: radius)
        }
    }

    private func pole(_ label: String, preset: MirCameraPreset, direction: SIMD3<Double>, center: CGPoint, radius: CGFloat) -> some View {
        let p = project(direction, radius: radius * 0.92)
        return Circle()
            .fill(MirTheme.Colors.accentBright.opacity(0.28))
            .frame(width: 16, height: 16)
            .overlay(Circle().stroke(.white.opacity(0.55), lineWidth: 0.8))
            .overlay(Text(label).font(.system(size: 7, weight: .bold)).foregroundStyle(.white))
            .position(x: center.x + p.x, y: center.y - p.y)
            .contentShape(Circle())
            .onTapGesture {
                pushHistory()
                Mir4DSetActiveCameraPreset(preset, animated: true)
            }
            .help(preset.titleRU)
            .accessibilityLabel(preset.titleRU)
    }

    private var trackballGesture: some Gesture {
        DragGesture(minimumDistance: 2)
            .onChanged { value in
                if !draggingSphere {
                    draggingSphere = true
                    dragStartTheta = theta
                    dragStartPhi = phi
                }
                let sensitivity = 0.0105
                let t = normalizedAngle(dragStartTheta + Double(value.translation.width) * sensitivity)
                let p = min(.pi - 0.035, max(0.035, dragStartPhi - Double(value.translation.height) * sensitivity))
                Mir4DSetCameraOrbit(theta: t, phi: p, distance: distance, animated: false)
            }
            .onEnded { _ in
                draggingSphere = false
                snapToNearestViewIfClose()
            }
    }

    private func azimuthRing(radius: CGFloat) -> some View {
        Circle()
            .stroke(.white.opacity(0.045), lineWidth: 11)
            .frame(width: (radius + 9) * 2, height: (radius + 9) * 2)
            .overlay(Circle().stroke(MirTheme.Colors.panelBorder.opacity(0.5), lineWidth: 1).frame(width: (radius + 9) * 2, height: (radius + 9) * 2))
            .contentShape(Circle().strokeBorder(lineWidth: 11))
            .gesture(
                DragGesture(minimumDistance: 2)
                    .onChanged { value in
                        if !draggingRing {
                            draggingRing = true
                            dragStartRingTheta = theta
                        }
                        Mir4DSetCameraOrbit(theta: normalizedAngle(dragStartRingTheta + Double(value.translation.width) * 0.014), phi: phi, distance: distance, animated: false)
                    }
                    .onEnded { _ in
                        draggingRing = false
                        snapToNearestViewIfClose()
                    }
            )
    }

    private func project(_ world: SIMD3<Double>, radius: CGFloat) -> ProjectedDirection {
        let forward = cameraDirection(theta: theta, phi: phi).normalized
        let reference = abs(forward.y) > 0.94 ? SIMD3(0, 0, 1) : SIMD3(0, 1, 0)
        let right = cross(reference, forward).normalized
        let up = cross(forward, right).normalized
        return ProjectedDirection(x: CGFloat(dot(world, right)) * radius, y: CGFloat(dot(world, up)) * radius, depth: dot(world, forward))
    }

    private func cameraDirection(theta: Double, phi: Double) -> SIMD3<Double> {
        SIMD3(sin(phi) * sin(theta), cos(phi), sin(phi) * cos(theta))
    }

    private func snapToNearestViewIfClose() {
        let current = cameraDirection(theta: theta, phi: phi).normalized
        var nearest = MirCameraPreset.front
        var best = -Double.infinity
        for preset in MirCameraPreset.allCases {
            let score = dot(current, preset.direction.normalized)
            if score > best { best = score; nearest = preset }
        }
        guard best >= cos(8.0 * .pi / 180), nearest != activePreset else { return }
        pushHistory()
        Mir4DSetActiveCameraPreset(nearest, animated: true)
    }

    private var activePreset: MirCameraPreset {
        let current = cameraDirection(theta: theta, phi: phi).normalized
        return MirCameraPreset.allCases.max { dot(current, $0.direction.normalized) < dot(current, $1.direction.normalized) } ?? .front
    }

    private func pushHistory() {
        history.append(OrbitSnapshot(theta: theta, phi: phi, distance: distance))
        if history.count > 24 { history.removeFirst(history.count - 24) }
    }

    private func normalizedAngle(_ value: Double) -> Double {
        var result = value.truncatingRemainder(dividingBy: 2 * .pi)
        if result <= -.pi { result += 2 * .pi }
        if result > .pi { result -= 2 * .pi }
        return result
    }

    private var orientationDescription: String {
        String(format: "Азимут %.0f°, наклон %.0f°, расстояние %.2f", theta * 180 / .pi, phi * 180 / .pi, distance)
    }

    @ViewBuilder
    private var menu: some View {
        Button("Спереди") { navigate(.front) }
        Button("Сзади") { navigate(.back) }
        Button("Слева") { navigate(.left) }
        Button("Справа") { navigate(.right) }
        Button("Сверху") { navigate(.top) }
        Button("Снизу") { navigate(.bottom) }
        Button("Изометрия") { navigate(.isometric) }
        Divider()
        Button("Вписать всё") { Mir4DRequestCameraFit() }
        Button(isOrthographic ? "Перспективная проекция" : "Ортографическая проекция") {
            NotificationCenter.default.post(name: .mir4DCameraProjectionRequested, object: nil, userInfo: ["projection": isOrthographic ? 0 : 1])
        }
    }

    private func navigate(_ preset: MirCameraPreset) {
        pushHistory()
        Mir4DSetActiveCameraPreset(preset, animated: true)
    }

    private func sphereButton(icon: String, help: String, disabled: Bool = false, action: @escaping () -> Void) -> some View {
        Image(systemName: icon)
            .font(.system(size: 10, weight: .semibold))
            .foregroundStyle(disabled ? .white.opacity(0.25) : .white.opacity(0.85))
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(RoundedRectangle(cornerRadius: 5).fill(.white.opacity(disabled ? 0.03 : 0.10)))
            .overlay(RoundedRectangle(cornerRadius: 5).stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 0.8))
            .contentShape(RoundedRectangle(cornerRadius: 5))
            .onTapGesture { if !disabled { action() } }
            .help(help)
    }

    private struct ProjectedDirection {
        let x: CGFloat
        let y: CGFloat
        let depth: Double
    }

    private struct OrbitSnapshot {
        let theta: Double
        let phi: Double
        let distance: Double
    }
}

private func dot(_ a: SIMD3<Double>, _ b: SIMD3<Double>) -> Double {
    a.x * b.x + a.y * b.y + a.z * b.z
}

private func cross(_ a: SIMD3<Double>, _ b: SIMD3<Double>) -> SIMD3<Double> {
    SIMD3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x)
}
