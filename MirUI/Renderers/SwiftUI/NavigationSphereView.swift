import SwiftUI

struct NavigationSphereView: View {
    var theta: Double
    var phi: Double
    var distance: Double
    var isOrthographic: Bool

    @State private var dragStartTheta: Double = 0
    @State private var dragStartPhi: Double = .pi / 2
    @State private var ringStartTheta: Double = 0
    @State private var draggingSphere = false
    @State private var draggingRing = false
    @State private var history: [SphereCameraSnapshot] = []
    @State private var hoveredPreset: MirCameraPreset?

    private let sphereSize: CGFloat = 190

    var body: some View {
        VStack(spacing: 7) {
            GeometryReader { proxy in
                let size = min(proxy.size.width, proxy.size.height)
                let radius = size * 0.40
                let center = CGPoint(x: proxy.size.width / 2, y: proxy.size.height / 2)

                ZStack {
                    sphereBody(radius: radius)
                    sphereLatitudeLongitude(radius: radius)
                    worldMarkers(center: center, radius: radius)
                    orientationIndicator(center: center, radius: radius)
                    poleControls(center: center, radius: radius)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .contentShape(Rectangle())
                .gesture(trackballGesture)
                .overlay { azimuthRing(radius: radius) }
            }
            .frame(width: sphereSize, height: 170)

            controls
        }
        .frame(width: sphereSize)
        .contextMenu { contextMenu }
        .accessibilityElement(children: .contain)
        .accessibilityLabel("Навигационная сфера МИР 4D")
        .accessibilityValue(orientationDescription)
    }

    private func sphereBody(radius: CGFloat) -> some View {
        Circle()
            .fill(
                RadialGradient(
                    colors: [
                        Color(red: 0.25, green: 0.39, blue: 0.59),
                        Color(red: 0.10, green: 0.17, blue: 0.28),
                        Color(red: 0.035, green: 0.055, blue: 0.09)
                    ],
                    center: UnitPoint(x: 0.32, y: 0.26),
                    startRadius: 2,
                    endRadius: radius * 1.15
                )
            )
            .overlay(Circle().stroke(MirTheme.Colors.panelBorder.opacity(0.95), lineWidth: 1))
            .overlay {
                Circle().stroke(.white.opacity(0.08), lineWidth: 1).padding(radius * 0.08)
            }
            .overlay(alignment: .topLeading) {
                Ellipse()
                    .fill(.white.opacity(0.07))
                    .frame(width: radius * 0.62, height: radius * 0.27)
                    .offset(x: radius * 0.12, y: radius * 0.13)
                    .blur(radius: 3)
                    .allowsHitTesting(false)
            }
    }

    private func sphereLatitudeLongitude(radius: CGFloat) -> some View {
        ZStack {
            Ellipse().stroke(.white.opacity(0.16), lineWidth: 0.8)
                .frame(width: radius * 2, height: radius * 0.72)
            Ellipse().stroke(.white.opacity(0.075), lineWidth: 0.7)
                .frame(width: radius * 2, height: radius * 0.42)
                .offset(y: -radius * 0.36)
            Ellipse().stroke(.white.opacity(0.075), lineWidth: 0.7)
                .frame(width: radius * 2, height: radius * 0.42)
                .offset(y: radius * 0.36)
            Ellipse().stroke(.white.opacity(0.11), lineWidth: 0.75)
                .frame(width: radius * 0.62, height: radius * 2)
            Ellipse().stroke(.white.opacity(0.075), lineWidth: 0.7)
                .frame(width: radius * 1.15, height: radius * 2)
            Rectangle()
                .fill(MirTheme.Colors.accentBright.opacity(0.20))
                .frame(width: 1, height: radius * 1.82)
        }
        .allowsHitTesting(false)
    }

    private var directionPresets: [(MirCameraPreset, SIMD3<Double>)] {
        [
            (.front, SIMD3(0, -1, 0)),
            (.frontRight, normalized(SIMD3(-1, -1, 0))),
            (.right, SIMD3(-1, 0, 0)),
            (.backRight, normalized(SIMD3(-1, 1, 0))),
            (.back, SIMD3(0, 1, 0)),
            (.backLeft, normalized(SIMD3(1, 1, 0))),
            (.left, SIMD3(1, 0, 0)),
            (.frontLeft, normalized(SIMD3(1, -1, 0)))
        ]
    }

    private func worldMarkers(center: CGPoint, radius: CGFloat) -> some View {
        ZStack {
            ForEach(Array(directionPresets.enumerated()), id: \.offset) { _, item in
                let projection = project(item.1, radius: radius * 0.91)
                let depth = max(0, min(1, (projection.depth + 1) * 0.5))
                let active = nearestHorizontalPreset == item.0
                let hovered = hoveredPreset == item.0

                Button { navigate(item.0) } label: {
                    Circle()
                        .fill(active ? MirTheme.Colors.accentBright : .white.opacity(0.22 + 0.38 * depth))
                        .frame(width: active ? 12 : hovered ? 10 : 7,
                               height: active ? 12 : hovered ? 10 : 7)
                        .overlay(Circle().stroke(.white.opacity(active ? 0.95 : 0.35), lineWidth: active ? 1.1 : 0.6))
                }
                .buttonStyle(.plain)
                .position(x: center.x + projection.x, y: center.y - projection.y)
                .opacity(0.45 + 0.55 * depth)
                .onHover { hovering in
                    hoveredPreset = hovering ? item.0 : (hoveredPreset == item.0 ? nil : hoveredPreset)
                }
                .help(item.0.titleRU)
                .accessibilityLabel(item.0.titleRU)
            }

            compassLabel("П", direction: SIMD3(0, -1, 0), center: center, radius: radius * 0.92)
            compassLabel("ПР", direction: SIMD3(-1, 0, 0), center: center, radius: radius * 0.92)
            compassLabel("З", direction: SIMD3(0, 1, 0), center: center, radius: radius * 0.92)
            compassLabel("Л", direction: SIMD3(1, 0, 0), center: center, radius: radius * 0.92)
        }
    }

    private func compassLabel(_ text: String, direction: SIMD3<Double>, center: CGPoint, radius: CGFloat) -> some View {
        let p = project(direction, radius: radius)
        let depth = max(0, min(1, (p.depth + 1) * 0.5))
        return Text(text)
            .font(.system(size: text.count > 1 ? 7 : 8, weight: .bold, design: .rounded))
            .foregroundStyle(.white.opacity(0.42 + 0.45 * depth))
            .position(x: center.x + p.x, y: center.y - p.y)
            .allowsHitTesting(false)
    }

    private func orientationIndicator(center: CGPoint, radius: CGFloat) -> some View {
        let worldUp = SIMD3<Double>(0, 0, 1)
        let projection = project(worldUp, radius: radius * 0.55)
        let end = CGPoint(x: center.x + projection.x, y: center.y - projection.y)
        let nearPole = abs(dot(cameraForward, worldUp)) > 0.96

        return ZStack {
            Path { path in
                path.move(to: center)
                path.addLine(to: end)
            }
            .stroke(MirTheme.Colors.accentBright.opacity(0.72), style: StrokeStyle(lineWidth: 1.4, dash: [3, 2]))

            Circle()
                .fill(MirTheme.Colors.accentBright)
                .frame(width: nearPole ? 9 : 7, height: nearPole ? 9 : 7)
                .overlay(Circle().stroke(.white.opacity(0.95), lineWidth: 1))
                .position(end)

            Circle()
                .fill(.black.opacity(0.30))
                .frame(width: 12, height: 12)
                .overlay(Circle().stroke(.white.opacity(0.70), lineWidth: 0.8))
                .position(center)
        }
        .allowsHitTesting(false)
    }

    private func poleControls(center: CGPoint, radius: CGFloat) -> some View {
        ZStack {
            pole("В", preset: .top, direction: SIMD3(0, 0, -1), center: center, radius: radius * 0.90)
            pole("Н", preset: .bottom, direction: SIMD3(0, 0, 1), center: center, radius: radius * 0.90)
        }
    }

    private func pole(_ label: String, preset: MirCameraPreset, direction: SIMD3<Double>, center: CGPoint, radius: CGFloat) -> some View {
        let p = project(direction, radius: radius)
        let depth = max(0, min(1, (p.depth + 1) * 0.5))
        return Button { navigate(preset) } label: {
            Circle()
                .fill(MirTheme.Colors.accentBright.opacity(0.20 + 0.16 * depth))
                .frame(width: 17, height: 17)
                .overlay(Circle().stroke(.white.opacity(0.58 + 0.25 * depth), lineWidth: 0.8))
                .overlay(Text(label).font(.system(size: 7, weight: .bold)).foregroundStyle(.white))
        }
        .buttonStyle(.plain)
        .position(x: center.x + p.x, y: center.y - p.y)
        .opacity(0.50 + 0.50 * depth)
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
                    pushHistory()
                }
                let sensitivity = 0.0105
                let newTheta = normalizeAngle(dragStartTheta + Double(value.translation.width) * sensitivity)
                let newPhi = clamp(dragStartPhi - Double(value.translation.height) * sensitivity,
                                   min: 0.035, max: .pi - 0.035)
                Mir4DSetCameraOrbit(theta: newTheta, phi: newPhi, distance: distance, animated: false)
            }
            .onEnded { _ in
                draggingSphere = false
                snapToNearestViewIfClose()
            }
    }

    private func azimuthRing(radius: CGFloat) -> some View {
        Circle()
            .stroke(.white.opacity(0.045), lineWidth: 12)
            .frame(width: (radius + 10) * 2, height: (radius + 10) * 2)
            .overlay(
                Circle()
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.55), lineWidth: 1)
                    .frame(width: (radius + 10) * 2, height: (radius + 10) * 2)
            )
            .contentShape(Circle())
            .gesture(
                DragGesture(minimumDistance: 2)
                    .onChanged { value in
                        if !draggingRing {
                            draggingRing = true
                            ringStartTheta = theta
                            pushHistory()
                        }
                        let newTheta = normalizeAngle(ringStartTheta + Double(value.translation.width) * 0.014)
                        Mir4DSetCameraOrbit(theta: newTheta, phi: phi, distance: distance, animated: false)
                    }
                    .onEnded { _ in
                        draggingRing = false
                        snapToNearestViewIfClose()
                    }
            )
    }

    private var controls: some View {
        HStack(spacing: 5) {
            sphereButton(icon: "cube", help: "Изометрия (F8)") { navigate(.isometric) }
            sphereButton(icon: "arrow.uturn.backward.circle", help: "Предыдущий вид", disabled: history.isEmpty) {
                guard let snapshot = history.popLast() else { return }
                Mir4DSetCameraOrbit(theta: snapshot.theta, phi: snapshot.phi, distance: snapshot.distance, animated: true)
            }
            sphereButton(icon: "arrow.up.left.and.down.right.magnifyingglass", help: "Вписать всё") {
                Mir4DRequestCameraFit()
            }
        }
        .frame(height: 20)
    }

    private func sphereButton(icon: String, help: String, disabled: Bool = false, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: icon)
                .font(.system(size: 10, weight: .semibold))
                .foregroundStyle(disabled ? .white.opacity(0.25) : .white.opacity(0.88))
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .background(RoundedRectangle(cornerRadius: 5).fill(.white.opacity(disabled ? 0.025 : 0.09)))
                .overlay(RoundedRectangle(cornerRadius: 5).stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 0.8))
        }
        .buttonStyle(.plain)
        .disabled(disabled)
        .help(help)
    }

    private var cameraForward: SIMD3<Double> {
        normalized(SIMD3(-sin(phi) * sin(theta), -sin(phi) * cos(theta), -cos(phi)))
    }

    private func project(_ world: SIMD3<Double>, radius: CGFloat) -> ProjectedDirection {
        let forward = cameraForward
        let worldUp = SIMD3<Double>(0, 0, 1)
        let reference = abs(dot(forward, worldUp)) > 0.9999 ? SIMD3<Double>(0, 1, 0) : worldUp
        let right = normalized(cross(forward, reference))
        let up = normalized(cross(right, forward))
        let direction = normalized(world)
        return ProjectedDirection(
            x: CGFloat(dot(direction, right)) * radius,
            y: CGFloat(dot(direction, up)) * radius,
            depth: dot(direction, forward)
        )
    }

    private var nearestHorizontalPreset: MirCameraPreset {
        directionPresets.min { lhs, rhs in
            dot(cameraForward, lhs.1) > dot(cameraForward, rhs.1)
        }?.0 ?? .front
    }

    private var nearestPreset: MirCameraPreset {
        let horizontal = nearestHorizontalPreset
        let horizontalScore = dot(cameraForward, normalized(horizontal.direction))
        let verticalCandidates: [MirCameraPreset] = [.top, .bottom, .isometric]
        return verticalCandidates.max { lhs, rhs in
            dot(cameraForward, normalized(lhs.direction)) < dot(cameraForward, normalized(rhs.direction))
        }.map { candidate in
            dot(cameraForward, normalized(candidate.direction)) > horizontalScore ? candidate : horizontal
        } ?? horizontal
    }

    private func snapToNearestViewIfClose() {
        let nearest = nearestPreset
        let similarity = dot(cameraForward, normalized(nearest.direction))
        guard similarity >= cos(8.0 * .pi / 180) else { return }
        Mir4DSetActiveCameraPreset(nearest, animated: true)
    }

    private func navigate(_ preset: MirCameraPreset) {
        pushHistory()
        Mir4DSetActiveCameraPreset(preset, animated: true)
    }

    private func pushHistory() {
        history.append(SphereCameraSnapshot(theta: theta, phi: phi, distance: distance))
        if history.count > 24 { history.removeFirst(history.count - 24) }
    }

    @ViewBuilder
    private var contextMenu: some View {
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
            NotificationCenter.default.post(
                name: .mir4DCameraProjectionRequested,
                object: nil,
                userInfo: ["projection": isOrthographic ? 0 : 1]
            )
        }
    }

    private var orientationDescription: String {
        String(format: "Азимут %.0f°, наклон %.0f°, расстояние %.2f",
               theta * 180 / .pi, phi * 180 / .pi, distance)
    }

    private func normalized(_ value: SIMD3<Double>) -> SIMD3<Double> {
        let length = sqrt(value.x * value.x + value.y * value.y + value.z * value.z)
        guard length > 0.000001 else { return SIMD3(0, 0, 1) }
        return value / length
    }

    private func dot(_ a: SIMD3<Double>, _ b: SIMD3<Double>) -> Double {
        a.x * b.x + a.y * b.y + a.z * b.z
    }

    private func cross(_ a: SIMD3<Double>, _ b: SIMD3<Double>) -> SIMD3<Double> {
        SIMD3(a.y * b.z - a.z * b.y,
              a.z * b.x - a.x * b.z,
              a.x * b.y - a.y * b.x)
    }

    private func clamp(_ value: Double, min minimum: Double, max maximum: Double) -> Double {
        Swift.min(maximum, Swift.max(minimum, value))
    }

    private func normalizeAngle(_ value: Double) -> Double {
        var result = value.truncatingRemainder(dividingBy: 2 * .pi)
        if result <= -.pi { result += 2 * .pi }
        if result > .pi { result -= 2 * .pi }
        return result
    }
}

private struct ProjectedDirection {
    let x: CGFloat
    let y: CGFloat
    let depth: Double
}

private struct SphereCameraSnapshot {
    let theta: Double
    let phi: Double
    let distance: Double
}
