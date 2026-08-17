import SwiftUI

/// Selection model for the immersive radial menu. The first orbit selects a
/// category; continuing outward exposes that category's tools on orbit two.
struct MIR4DRadialSelectionState: Equatable {
    var panelIndex: Int?
    var toolIndex: Int?
    var orbit: Int = 0
}

enum MIR4DRadialSelectionEngine {
    static func state(vector: CGVector, settings: RadialMenuSettings) -> MIR4DRadialSelectionState {
        let panels = settings.panels.filter(\.enabled)
        guard !panels.isEmpty else { return .init() }
        let distance = hypot(vector.dx, vector.dy)
        guard distance >= settings.deadZone else { return .init() }

        let raw = normalizedAngle(atan2(-vector.dy, vector.dx) + .pi / 2)
        let panelIndex = nearestSector(raw, count: panels.count, magneticStrength: settings.magneticStrength)
        guard panels.indices.contains(panelIndex) else { return .init() }
        let panel = panels[panelIndex]

        guard distance >= settings.activationRadius, !panel.tools.isEmpty else {
            return .init(panelIndex: panelIndex, toolIndex: nil, orbit: 1)
        }

        let toolAngle = raw
        let toolIndex = nearestSector(toolAngle, count: panel.tools.count, magneticStrength: settings.magneticStrength)
        return .init(panelIndex: panelIndex, toolIndex: toolIndex, orbit: 2)
    }

    static func normalizedAngle(_ value: Double) -> Double {
        let full = Double.pi * 2
        let result = value.truncatingRemainder(dividingBy: full)
        return result < 0 ? result + full : result
    }

    static func nearestSector(_ angle: Double, count: Int, magneticStrength: Double) -> Int {
        guard count > 0 else { return 0 }
        let sector = Double.pi * 2 / Double(count)
        let raw = normalizedAngle(angle)
        let index = Int(floor((raw + sector / 2) / sector)) % count
        _ = magneticStrength // strength is applied to the visual snap below
        return index
    }

    static func snappedAngle(_ angle: Double, count: Int, strength: Double) -> Double {
        guard count > 0 else { return angle }
        let sector = Double.pi * 2 / Double(count)
        let raw = normalizedAngle(angle)
        let index = nearestSector(raw, count: count, magneticStrength: strength)
        let target = Double(index) * sector
        let delta = atan2(sin(target - raw), cos(target - raw))
        return normalizedAngle(raw + delta * min(max(strength, 0), 1))
    }
}

struct MIR4DRadialSelectionOverlay: View {
    @ObservedObject var store: RadialMenuSettingsStore
    let center: CGPoint
    let vector: CGVector
    let onToolActivated: (RadialMenuTool) -> Void
    let onSettings: () -> Void

    private var panels: [RadialMenuPanel] { store.settings.panels.filter(\.enabled) }
    private var state: MIR4DRadialSelectionState { MIR4DRadialSelectionEngine.state(vector: vector, settings: store.settings) }
    private var selectedPanel: RadialMenuPanel? {
        guard let index = state.panelIndex, panels.indices.contains(index) else { return nil }
        return panels[index]
    }
    private var selectedTool: RadialMenuTool? {
        guard let panel = selectedPanel, let index = state.toolIndex, panel.tools.indices.contains(index) else { return nil }
        return panel.tools[index]
    }
    private var distance: Double { hypot(vector.dx, vector.dy) }

    var body: some View {
        ZStack {
            Color.black.opacity(0.22)
                .ignoresSafeArea()
                .blur(radius: 1)
                .allowsHitTesting(false)

            ZStack {
                Circle().fill(.ultraThinMaterial).frame(width: 92, height: 92)
                    .overlay(Circle().stroke(MirTheme.Colors.accentBright.opacity(0.75), lineWidth: 1.5))
                    .overlay {
                        VStack(spacing: 3) {
                            Image(systemName: selectedTool?.icon ?? selectedPanel?.icon ?? "scope")
                                .font(.system(size: 20, weight: .semibold))
                            Text(selectedTool?.title ?? selectedPanel?.title ?? "МИР")
                                .font(.system(size: 10, weight: .bold))
                                .lineLimit(1)
                        }.foregroundStyle(.white)
                    }

                ForEach(Array(panels.enumerated()), id: \.element.id) { index, panel in
                    let selected = state.panelIndex == index
                    radialNode(title: panel.title, icon: panel.icon, selected: selected,
                               radius: store.settings.panelRadius,
                               angle: sectorAngle(index, panels.count))
                }

                if let panel = selectedPanel {
                    let angle = atan2(vector.dy, vector.dx)
                    let submenuCenter = CGPoint(x: CGFloat(cos(angle)) * store.settings.submenuOffset,
                                                y: CGFloat(sin(angle)) * store.settings.submenuOffset)
                    Circle().stroke(MirTheme.Colors.selection.opacity(0.35), lineWidth: 1)
                        .frame(width: CGFloat(store.settings.submenuOffset * 2), height: CGFloat(store.settings.submenuOffset * 2))
                        .position(x: 260 + submenuCenter.x, y: 260 + submenuCenter.y)

                    ForEach(Array(panel.tools.enumerated()), id: \.element.id) { index, tool in
                        let selected = state.toolIndex == index
                        let toolAngle = sectorAngle(index, panel.tools.count)
                        radialNode(title: store.settings.showLabels ? tool.title : "", icon: tool.icon,
                                   selected: selected, radius: 52,
                                   angle: toolAngle,
                                   origin: submenuCenter)
                    }
                }

                Path { path in
                    path.move(to: CGPoint(x: 260, y: 260))
                    path.addLine(to: CGPoint(x: 260 + vector.dx * 0.65, y: 260 + vector.dy * 0.65))
                }
                .stroke(MirTheme.Colors.selection.opacity(0.55), style: StrokeStyle(lineWidth: 2, lineCap: .round, dash: [4, 8]))
            }
            .frame(width: 520, height: 520)
            .position(center)
            .transition(.opacity.combined(with: .scale(scale: 0.9)))
            .animation(.interactiveSpring(response: 0.22, dampingFraction: 0.78), value: state)

            Button(action: onSettings) { Image(systemName: "gearshape.fill") }
                .buttonStyle(.plain)
                .foregroundStyle(.white.opacity(0.7))
                .padding(10)
                .background(.black.opacity(0.45), in: Circle())
                .position(x: center.x + 42, y: center.y + 42)

            if distance >= store.settings.activationRadius, let tool = selectedTool {
                Text("Отпустите — \(tool.title)")
                    .font(.system(size: 12, weight: .semibold))
                    .foregroundStyle(.white)
                    .padding(.horizontal, 14).padding(.vertical, 8)
                    .background(.ultraThinMaterial, in: Capsule())
                    .position(x: center.x, y: center.y + 190)
                    .transition(.opacity)
            }
        }
        .allowsHitTesting(false)
    }

    private func radialNode(title: String, icon: String, selected: Bool, radius: Double, angle: Double, origin: CGPoint = .zero) -> some View {
        let x = 260 + origin.x + CGFloat(cos(angle) * radius)
        let y = 260 + origin.y + CGFloat(sin(angle) * radius)
        return VStack(spacing: 3) {
            Image(systemName: icon).font(.system(size: selected ? 16 : 13, weight: .semibold))
            if !title.isEmpty { Text(title).font(.system(size: 8, weight: .semibold)).lineLimit(1) }
        }
        .foregroundStyle(selected ? .white : .white.opacity(0.72))
        .frame(width: selected ? 82 : 68, height: selected ? 50 : 44)
        .background(selected ? MirTheme.Colors.selection.opacity(0.86) : Color.black.opacity(0.48), in: RoundedRectangle(cornerRadius: 13))
        .overlay(RoundedRectangle(cornerRadius: 13).stroke(selected ? MirTheme.Colors.accentBright : .white.opacity(0.12), lineWidth: selected ? 1.5 : 0.8))
        .scaleEffect(selected ? 1.08 : 1)
        .position(x: x, y: y)
    }

    private func sectorAngle(_ index: Int, _ count: Int) -> Double {
        -.pi / 2 + Double.pi * 2 / Double(max(count, 1)) * Double(index)
    }
}
