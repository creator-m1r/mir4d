import SwiftUI

/// Spatial orientation instrument.
/// The sphere visualizes camera state and emits camera-preset requests;
/// it does not own camera state or viewport handles.
struct NavigationSphereView: View {
    var theta: Double
    var phi: Double
    var distance: Double

    var body: some View {
        ZStack {
            sphereBody
            latitudeLines
            longitudeLines
            orientationMarker
            labels
            navigationHitZones
        }
        .frame(width: 120, height: 120)
        .contentShape(Circle())
        .accessibilityElement(children: .combine)
        .accessibilityLabel("Навигационная сфера")
        .accessibilityValue(orientationDescription)
    }

    private var sphereBody: some View {
        Circle()
            .fill(
                RadialGradient(
                    colors: [
                        Color.white.opacity(0.16),
                        MirTheme.Colors.viewport.opacity(0.82),
                        Color.black.opacity(0.48)
                    ],
                    center: .topLeading,
                    startRadius: 4,
                    endRadius: 60
                )
            )
            .overlay(
                Circle()
                    .stroke(MirTheme.Colors.accentBright.opacity(0.7), lineWidth: 1)
            )
            .shadow(color: .black.opacity(0.35), radius: 10, y: 5)
            .allowsHitTesting(false)
    }

    private var latitudeLines: some View {
        GeometryReader { proxy in
            Path { path in
                let center = CGPoint(x: proxy.size.width / 2, y: proxy.size.height / 2)
                let radius = min(proxy.size.width, proxy.size.height) * 0.39
                for factor in [-0.65, -0.32, 0.0, 0.32, 0.65] {
                    let y = center.y + CGFloat(factor) * radius
                    let halfWidth = sqrt(max(radius * radius - pow(y - center.y, 2), 0))
                    path.move(to: CGPoint(x: center.x - halfWidth, y: y))
                    path.addLine(to: CGPoint(x: center.x + halfWidth, y: y))
                }
            }
            .stroke(Color.white.opacity(0.12), lineWidth: 0.8)
        }
        .clipShape(Circle())
        .allowsHitTesting(false)
    }

    private var longitudeLines: some View {
        GeometryReader { proxy in
            Path { path in
                let center = CGPoint(x: proxy.size.width / 2, y: proxy.size.height / 2)
                let radius = min(proxy.size.width, proxy.size.height) * 0.39
                for factor in [-0.68, -0.34, 0.0, 0.34, 0.68] {
                    let width = max(radius * (1.0 - abs(factor) * 0.9), 3)
                    path.move(to: CGPoint(x: center.x + CGFloat(factor) * radius, y: center.y - radius))
                    path.addCurve(
                        to: CGPoint(x: center.x + CGFloat(factor) * radius, y: center.y + radius),
                        control1: CGPoint(x: center.x + CGFloat(factor) * radius + width, y: center.y - radius * 0.35),
                        control2: CGPoint(x: center.x + CGFloat(factor) * radius - width, y: center.y + radius * 0.35)
                    )
                }
            }
            .stroke(Color.white.opacity(0.1), lineWidth: 0.8)
        }
        .clipShape(Circle())
        .allowsHitTesting(false)
    }

    private var orientationMarker: some View {
        let normalizedTheta = theta.truncatingRemainder(dividingBy: Double.pi * 2)
        let normalizedPhi = max(0.25, min(Double.pi - 0.25, phi))
        let x = CGFloat(cos(normalizedTheta) * sin(normalizedPhi))
        let y = CGFloat(-cos(normalizedPhi))

        return GeometryReader { proxy in
            let center = CGPoint(x: proxy.size.width / 2, y: proxy.size.height / 2)
            let scale = min(proxy.size.width, proxy.size.height) * 0.32
            Circle()
                .fill(MirTheme.Colors.selection)
                .frame(width: 9, height: 9)
                .shadow(color: MirTheme.Colors.selection.opacity(0.7), radius: 5)
                .position(
                    x: center.x + x * scale,
                    y: center.y + y * scale
                )
        }
        .allowsHitTesting(false)
    }

    private var labels: some View {
        ZStack {
            Text("TOP").position(x: 60, y: 10)
            Text("FRONT").position(x: 60, y: 110)
            Text("L").position(x: 10, y: 60)
            Text("R").position(x: 110, y: 60)
            Text("ISO").position(x: 92, y: 25)
        }
        .font(.system(size: 8, weight: .semibold, design: .rounded))
        .foregroundStyle(Color.white.opacity(0.62))
        .allowsHitTesting(false)
    }

    private var navigationHitZones: some View {
        GeometryReader { proxy in
            ZStack {
                presetButton(.top, at: CGPoint(x: proxy.size.width / 2, y: 10), size: CGSize(width: 56, height: 20))
                presetButton(.front, at: CGPoint(x: proxy.size.width / 2, y: proxy.size.height - 10), size: CGSize(width: 60, height: 20))
                presetButton(.left, at: CGPoint(x: 11, y: proxy.size.height / 2), size: CGSize(width: 24, height: 32))
                presetButton(.right, at: CGPoint(x: proxy.size.width - 11, y: proxy.size.height / 2), size: CGSize(width: 24, height: 32))
                presetButton(.isometric, at: CGPoint(x: proxy.size.width - 29, y: 24), size: CGSize(width: 40, height: 22))
            }
        }
    }

    private func presetButton(_ preset: MirCameraPreset, at point: CGPoint, size: CGSize) -> some View {
        Color.clear
            .frame(width: size.width, height: size.height)
            .contentShape(Rectangle())
            .position(point)
            .onTapGesture {
                Mir4DSetActiveCameraPreset(preset)
                NotificationCenter.default.post(
                    name: .mir4DCameraPresetRequested,
                    object: nil,
                    userInfo: ["preset": preset.rawValue]
                )
            }
            .accessibilityLabel(preset.titleRU)
    }

    private var orientationDescription: String {
        String(format: "θ %.2f°, φ %.2f°, d %.2f", theta * 180.0 / .pi, phi * 180.0 / .pi, distance)
    }
}
