import MirUIHandGesture
import SwiftUI

struct MIR4DRadialSelectionState: Equatable {
    var panelIndex: Int?
    var toolIndex: Int?
    var orbit: Int = 0
    var angle: Double = 0
}

enum MIR4DRadialSelectionEngine {
    static func state(vector: CGVector, settings: RadialMenuSettings) -> MIR4DRadialSelectionState { state(vector: vector, settings: settings, previous: nil) }
    static func state(vector: CGVector, settings: RadialMenuSettings, previous: MIR4DRadialSelectionState?) -> MIR4DRadialSelectionState {
        let panels = settings.panels.filter(\.enabled)
        guard !panels.isEmpty else { return .init() }
        let distance = hypot(vector.dx, vector.dy)
        guard distance >= settings.deadZone else { return .init() }
        let rawAngle = normalizedAngle(atan2(vector.dy, vector.dx))
        let panelIndex = hystereticSector(angle: rawAngle, count: panels.count, previous: previous?.panelIndex, hysteresis: 0.16)
        guard panels.indices.contains(panelIndex) else { return .init() }
        let panel = panels[panelIndex]
        guard distance >= settings.activationRadius, !panel.tools.isEmpty else { return .init(panelIndex: panelIndex, toolIndex: nil, orbit: 1, angle: rawAngle) }
        let toolCount = panel.tools.count
        let panelCenter = sectorCenter(panelIndex, panels.count)
        let relative = shortestSignedAngle(from: panelCenter, to: rawAngle)
        let toolAngle = normalizedAngle(panelCenter + relative)
        let toolIndex = hystereticSector(angle: toolAngle, count: toolCount, previous: previous?.panelIndex == panelIndex ? previous?.toolIndex : nil, hysteresis: 0.20)
        return .init(panelIndex: panelIndex, toolIndex: toolIndex, orbit: 2, angle: toolAngle)
    }
    static func hystereticSector(angle: Double, count: Int, previous: Int?, hysteresis: Double) -> Int {
        guard count > 0 else { return 0 }
        let sector = Double.pi * 2 / Double(count)
        let candidate = nearestSector(angle, count: count)
        guard let previous, previous >= 0, previous < count else { return candidate }
        if candidate == previous { return previous }
        let previousCenter = sectorCenter(previous, count)
        let distance = abs(shortestSignedAngle(from: previousCenter, to: angle))
        let threshold = sector * (0.5 + min(max(hysteresis, 0), 0.45))
        return distance > threshold ? candidate : previous
    }
    static func normalizedAngle(_ value: Double) -> Double { let full = Double.pi * 2; let result = value.truncatingRemainder(dividingBy: full); return result < 0 ? result + full : result }
    static func shortestSignedAngle(from: Double, to: Double) -> Double { atan2(sin(to - from), cos(to - from)) }
    static func sectorCenter(_ index: Int, _ count: Int) -> Double { guard count > 0 else { return 0 }; return normalizedAngle((Double(index) + 0.5) * Double.pi * 2 / Double(count)) }
    static func nearestSector(_ angle: Double, count: Int) -> Int { guard count > 0 else { return 0 }; let sector = Double.pi * 2 / Double(count); return Int(floor((normalizedAngle(angle) + sector / 2) / sector)) % count }
    static func snappedAngle(_ angle: Double, count: Int, strength: Double) -> Double { guard count > 0 else { return angle }; let sector = Double.pi * 2 / Double(count); let raw = normalizedAngle(angle); let target = Double(nearestSector(raw, count: count)) * sector; let delta = atan2(sin(target - raw), cos(target - raw)); return normalizedAngle(raw + delta * min(max(strength, 0), 1)) }
}

struct MIR4DRadialSelectionOverlay: View {
    @ObservedObject var store: RadialMenuSettingsStore
    let center: CGPoint
    let vector: CGVector
    let onToolActivated: (RadialMenuTool) -> Void
    let onSettings: () -> Void
    @State private var previousState = MIR4DRadialSelectionState()
    @State private var committedTool: RadialMenuTool?
    @State private var isCommitting = false

    private var panels: [RadialMenuPanel] { store.settings.panels.filter(\.enabled) }
    private var state: MIR4DRadialSelectionState { MIR4DRadialSelectionEngine.state(vector: vector, settings: store.settings, previous: previousState) }
    private var selectedPanel: RadialMenuPanel? { guard let index = state.panelIndex, panels.indices.contains(index) else { return nil }; return panels[index] }
    private var selectedTool: RadialMenuTool? { guard let panel = selectedPanel, let index = state.toolIndex, panel.tools.indices.contains(index) else { return nil }; return panel.tools[index] }
    private var distance: Double { hypot(vector.dx, vector.dy) }
    private var direction: Double { state.angle }
    private var revealProgress: Double { guard state.orbit == 2 else { return 0 }; let start = max(1, store.settings.activationRadius); let travel = max(1, store.settings.submenuOffset - start); return min(max((distance - start) / travel, 0), 1) }
    private var revealEased: Double { let x = revealProgress; return x * x * (3 - 2 * x) }
    private var depthScale: Double { 0.92 + revealEased * 0.08 }
    private var blurAmount: Double { 1.5 + revealEased * 3.5 }

    var body: some View {
        ZStack {
            Rectangle().fill(.ultraThinMaterial).ignoresSafeArea().overlay(Color.black.opacity(0.14 + revealEased * 0.08).ignoresSafeArea()).blur(radius: blurAmount).allowsHitTesting(false)
            ZStack {
                Circle().fill(.ultraThinMaterial).frame(width: 92, height: 92).overlay(Circle().stroke(MirTheme.Colors.accentBright.opacity(0.75), lineWidth: 1.5)).overlay { VStack(spacing: 3) { Image(systemName: committedTool?.icon ?? selectedTool?.icon ?? selectedPanel?.icon ?? "scope").font(.system(size: 20, weight: .semibold)); Text(committedTool?.title ?? selectedTool?.title ?? selectedPanel?.title ?? "МИР").font(.system(size: 10, weight: .bold)).lineLimit(1) }.foregroundStyle(.white) }
                ForEach(Array(panels.enumerated()), id: \.element.id) { index, panel in
                    radialNode(title: panel.title, icon: panel.icon, selected: state.panelIndex == index && !isCommitting, radius: store.settings.panelRadius, angle: sectorAngle(index, panels.count), opacity: isCommitting ? 0 : 1)
                }
                if let panel = selectedPanel, !isCommitting {
                    let submenuRadius = max(store.settings.submenuOffset, store.settings.panelRadius + 58)
                    let reveal = revealEased
                    let submenuCenter = CGPoint(x: CGFloat(cos(direction) * submenuRadius * reveal), y: CGFloat(sin(direction) * submenuRadius * reveal))
                    let submenuScale = 0.72 + 0.28 * reveal
                    let submenuOpacity = 0.16 + 0.84 * reveal
                    Circle().stroke(MirTheme.Colors.selection.opacity(0.35 * submenuOpacity), lineWidth: 1).frame(width: CGFloat(store.settings.submenuOffset * 2 * submenuScale), height: CGFloat(store.settings.submenuOffset * 2 * submenuScale)).position(x: 260 + submenuCenter.x, y: 260 + submenuCenter.y)
                    ForEach(Array(panel.tools.enumerated()), id: \.element.id) { index, tool in
                        radialNode(title: store.settings.showLabels ? tool.title : "", icon: tool.icon, selected: state.toolIndex == index, radius: 62 * reveal, angle: submenuToolAngle(index, panel.tools.count, center: direction), origin: submenuCenter, opacity: submenuOpacity, scale: submenuScale)
                    }
                }
                Path { path in path.move(to: CGPoint(x: 260, y: 260)); path.addLine(to: CGPoint(x: 260 + vector.dx * 0.65, y: 260 + vector.dy * 0.65)) }.stroke(MirTheme.Colors.selection.opacity(0.62), style: StrokeStyle(lineWidth: 2.2, lineCap: .round, dash: [4, 8])).opacity(isCommitting ? 0 : 1)
            }.frame(width: 520, height: 520).position(center).scaleEffect(depthScale).transition(.opacity.combined(with: .scale(scale: 0.9))).animation(.interactiveSpring(response: 0.22, dampingFraction: 0.78), value: state)
            if distance >= store.settings.activationRadius, let tool = committedTool ?? selectedTool, !isCommitting { Text(isCommitting ? "" : "Отпустите — \(tool.title)").font(.system(size: 12, weight: .semibold)).foregroundStyle(.white).padding(.horizontal, 14).padding(.vertical, 8).background(.ultraThinMaterial, in: Capsule()).position(x: center.x, y: center.y + 190).transition(.opacity) }
            Button(action: onSettings) { Image(systemName: "gearshape.fill") }.buttonStyle(.plain).foregroundStyle(.white.opacity(0.7)).padding(10).background(.black.opacity(0.45), in: Circle()).position(x: center.x + 42, y: center.y + 42)
        }
        .allowsHitTesting(false)
        .onChange(of: state) { _, newState in
            previousState = newState
            publishSelectionIntent(for: newState)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRadialMenuEnded)) { note in
            guard (note.userInfo?["commit"] as? Bool) == true, !isCommitting else { return }
            commitSelection()
        }
        .onAppear { previousState = .init(); committedTool = nil; isCommitting = false }
        .onDisappear { previousState = .init(); committedTool = nil; isCommitting = false }
    }

    private func publishSelectionIntent(for newState: MIR4DRadialSelectionState) {
        guard let panelIndex = newState.panelIndex, panels.indices.contains(panelIndex) else { return }
        let panel = panels[panelIndex]
        if let toolIndex = newState.toolIndex, panel.tools.indices.contains(toolIndex) {
            let tool = panel.tools[toolIndex]
            MIRIntentRouter.shared.publish(MIRIntent(source: .trackpad, phase: .selection, action: tool.command, directionRadians: newState.angle, confidence: 0.92))
        } else {
            MIRIntentRouter.shared.publish(MIRIntent(source: .trackpad, phase: .preview, action: panel.title, directionRadians: newState.angle, confidence: 0.80))
        }
    }

    private func commitSelection() {
        guard let tool = selectedTool else { return }
        committedTool = tool
        isCommitting = true
        MIRIntentRouter.shared.publish(MIRIntent(source: .keyboard, phase: .confirmation, action: tool.command, directionRadians: state.angle, confidence: 1.0))
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.16) {
            MIRIntentRouter.shared.publish(MIRIntent(source: .keyboard, phase: .execution, action: tool.command, directionRadians: state.angle, confidence: 1.0))
            onToolActivated(tool)
            committedTool = nil
            isCommitting = false
        }
    }

    private func radialNode(title: String, icon: String, selected: Bool, radius: Double, angle: Double, origin: CGPoint = .zero, opacity: Double = 1, scale: Double = 1) -> some View {
        let x = 260 + origin.x + CGFloat(cos(angle) * radius)
        let y = 260 + origin.y + CGFloat(sin(angle) * radius)
        return VStack(spacing: 3) { Image(systemName: icon).font(.system(size: selected ? 16 : 13, weight: .semibold)); if !title.isEmpty { Text(title).font(.system(size: 8, weight: .semibold)).lineLimit(1) } }.foregroundStyle(selected ? .white : .white.opacity(0.72)).frame(width: selected ? 82 : 68, height: selected ? 50 : 44).background(selected ? MirTheme.Colors.selection.opacity(0.86) : Color.black.opacity(0.48), in: RoundedRectangle(cornerRadius: 13)).overlay(RoundedRectangle(cornerRadius: 13).stroke(selected ? MirTheme.Colors.accentBright : .white.opacity(0.12), lineWidth: selected ? 1.5 : 0.8)).scaleEffect((selected ? 1.08 : 1) * scale).opacity(opacity).position(x: x, y: y)
    }
    private func sectorAngle(_ index: Int, _ count: Int) -> Double { -Double.pi / 2 + Double.pi * 2 / Double(max(count, 1)) * Double(index) }
    private func submenuToolAngle(_ index: Int, _ count: Int, center: Double) -> Double { guard count > 0 else { return center }; let spread = min(Double.pi * 0.9, max(Double.pi * 0.42, Double(count - 1) * Double.pi / 9)); if count == 1 { return center }; return center - spread / 2 + spread * Double(index) / Double(count - 1) }
}
