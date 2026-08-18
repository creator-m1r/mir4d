import SwiftUI

/// Draws the spatial fan: intention ring, directional submenu rings, the
/// gesture ray, the centre core and the voice hint.
///
/// Segments are arcs, not buttons. There is nothing to click: selection is
/// direction and release. Highlight states follow
/// `idle → hover → selected → active → disabled`.
struct MIRSpatialMenuRenderer: View {
    let tree: [MIRSpatialMenuItem]
    let state: MIRSpatialMenuState
    let settings: MIRSpatialMenuSettings
    let context: MIRSpatialMenuSceneContext
    let center: CGPoint
    let voiceHint: String?
    let language: String

    private var visibleIntents: [MIRSpatialMenuItem] {
        Array(tree.prefix(min(tree.count, settings.maxLevel1Segments)))
    }

    private var intent: MIRSpatialMenuItem? {
        guard let index = state.intentIndex, visibleIntents.indices.contains(index) else { return nil }
        return visibleIntents[index]
    }

    private var category: MIRSpatialMenuItem? {
        guard let intent, let index = state.categoryIndex, intent.children.indices.contains(index) else { return nil }
        return intent.children[index]
    }

    private var containerSize: CGFloat {
        CGFloat(settings.toolOffset * 2 + 190)
    }

    private var containerCenter: CGPoint {
        CGPoint(x: containerSize / 2, y: containerSize / 2)
    }

    var body: some View {
        ZStack {
            gestureRay
            centerCore
            intentionRing
            if let intent { categoryFan(intent: intent) }
            if let category { toolFan(category: category) }
            if let voiceHint { hintChip(voiceHint) }
        }
        .frame(width: containerSize, height: containerSize)
        .position(center)
        .allowsHitTesting(false)
        .animation(MIRSpatialMenuAnimation.ringReveal, value: state.level)
    }

    // MARK: - Centre core

    private var centerCore: some View {
        let hasTool = state.toolIndex != nil
        return ZStack {
            Circle()
                .fill(MirTheme.Colors.surface.opacity(0.9))
                .frame(width: 76, height: 76)
                .overlay(Circle().stroke(MirTheme.Colors.accentBright.opacity(0.8), lineWidth: 1.2))
                .shadow(color: MirTheme.Colors.accentBright.opacity(0.35), radius: 10)

            VStack(spacing: 3) {
                Image(systemName: hasTool ? "scope" : "cursorarrow.motionlines")
                    .font(.system(size: 18, weight: .semibold))
                Text(currentTitle)
                    .font(.system(size: 9, weight: .bold))
                    .lineLimit(1)
            }
            .foregroundStyle(.white)
        }
    }

    private var currentTitle: String {
        if let category, let toolIndex = state.toolIndex, category.children.indices.contains(toolIndex) {
            return category.children[toolIndex].localizedTitle(language)
        }
        if let intent, let categoryIndex = state.categoryIndex, intent.children.indices.contains(categoryIndex) {
            return intent.children[categoryIndex].localizedTitle(language)
        }
        if let intent {
            return intent.localizedTitle(language)
        }
        return "МИР"
    }

    // MARK: - Gesture ray

    private var gestureRay: some View {
        let c = containerCenter
        let length = max(state.distance, 1)
        let ux = state.dx / length
        let uy = state.dy / length
        let reach = min(state.distance, settings.toolOffset + 30)
        return Path { path in
            path.move(to: c)
            path.addLine(to: CGPoint(x: c.x + CGFloat(ux * reach), y: c.y + CGFloat(uy * reach)))
        }
        .stroke(MirTheme.Colors.selection.opacity(0.30), style: StrokeStyle(lineWidth: 1.4, lineCap: .round, dash: [3, 8]))
        .overlay {
            Circle()
                .fill(MirTheme.Colors.accentBright.opacity(0.9))
                .frame(width: 7, height: 7)
                .position(x: c.x + CGFloat(ux * reach), y: c.y + CGFloat(uy * reach))
        }
    }

    // MARK: - Level 1: intention ring

    private var intentionRing: some View {
        let c = containerCenter
        let count = visibleIntents.count
        return ZStack {
            ForEach(Array(visibleIntents.enumerated()), id: \.element.id) { index, item in
                let highlight = highlightForIntent(index: index)
                MIRSpatialMenuArcSegment(
                    center: c,
                    radius: settings.intentRadius,
                    thickness: 34,
                    startAngle: sectorStart(index: index, count: count),
                    endAngle: sectorEnd(index: index, count: count),
                    title: settings.showLabels ? item.localizedTitle(language) : "",
                    icon: item.icon,
                    highlight: highlight,
                    labelAtRadius: settings.intentRadius - 14
                )
            }
            // Quiet legibility ring.
            Circle()
                .stroke(.white.opacity(0.08), lineWidth: 1)
                .frame(width: CGFloat(settings.intentRadius * 2 + 34), height: CGFloat(settings.intentRadius * 2 + 34))
        }
    }

    private func highlightForIntent(index: Int) -> MIRSpatialMenuHighlightState {
        let item = visibleIntents[index]
        guard item.enabled else { return .disabled }
        guard state.intentIndex == index else { return .idle }
        guard state.level == .intent else { return .selected }
        return .hover
    }

    // MARK: - Level 2: category fan

    private func categoryFan(intent: MIRSpatialMenuItem) -> some View {
        let c = containerCenter
        let direction = MIRSpatialMenuLayout.angle(
            forIndex: state.intentIndex ?? 0,
            count: max(1, visibleIntents.count)
        )
        let reveal = MIRSpatialMenuLayout.smoothstep(MIRSpatialMenuLayout.revealProgress(
            distance: state.distance,
            start: settings.intentRadius,
            end: settings.categoryOffset
        ))
        let fanCenter = MIRSpatialMenuLayout.submenuCenter(direction: direction, offset: settings.categoryOffset * reveal, center: c)
        let count = intent.children.count
        let spread = MIRSpatialMenuLayout.toolSpread(count: count)

        return ZStack {
            ForEach(Array(intent.children.enumerated()), id: \.element.id) { index, item in
                let highlight = highlightForCategory(index: index)
                MIRSpatialMenuArcSegment(
                    center: fanCenter,
                    radius: 56,
                    thickness: 26,
                    startAngle: fanStart(centerAngle: direction, index: index, count: count, spread: spread),
                    endAngle: fanEnd(centerAngle: direction, index: index, count: count, spread: spread),
                    title: settings.showLabels ? item.localizedTitle(language) : "",
                    icon: item.icon,
                    highlight: highlight,
                    labelAtRadius: 42
                )
                .opacity(0.15 + reveal * 0.85)
                .scaleEffect(0.72 + reveal * 0.28)
            }
        }
        .opacity(reveal > 0.02 ? 1 : 0)
    }

    private func highlightForCategory(index: Int) -> MIRSpatialMenuHighlightState {
        let item = (intent?.children ?? [])[safe: index]
        guard let item, item.enabled else { return .disabled }
        guard state.categoryIndex == index else { return .idle }
        guard state.level == .category else { return .selected }
        return .hover
    }

    // MARK: - Level 3: tool fan

    private func toolFan(category: MIRSpatialMenuItem) -> some View {
        let c = containerCenter
        let direction = MIRSpatialMenuLayout.angle(
            forIndex: state.intentIndex ?? 0,
            count: max(1, visibleIntents.count)
        )
        let categoryIndex = state.categoryIndex ?? 0
        let categoryCount = intent?.children.count ?? 1
        let categoryAngle = MIRSpatialMenuLayout.toolAngle(index: categoryIndex, count: categoryCount, centerAngle: direction)
        let reveal = MIRSpatialMenuLayout.smoothstep(MIRSpatialMenuLayout.revealProgress(
            distance: state.distance,
            start: settings.categoryOffset,
            end: settings.toolOffset
        ))
        let fanCenter = MIRSpatialMenuLayout.submenuCenter(direction: categoryAngle, offset: settings.toolOffset * reveal, center: c)
        let count = category.children.count
        let spread = MIRSpatialMenuLayout.toolSpread(count: count)

        return ZStack {
            ForEach(Array(category.children.enumerated()), id: \.element.id) { index, item in
                let highlight = highlightForTool(index: index)
                MIRSpatialMenuArcSegment(
                    center: fanCenter,
                    radius: 58,
                    thickness: 28,
                    startAngle: fanStart(centerAngle: categoryAngle, index: index, count: count, spread: spread),
                    endAngle: fanEnd(centerAngle: categoryAngle, index: index, count: count, spread: spread),
                    title: settings.showLabels ? item.localizedTitle(language) : "",
                    icon: item.icon,
                    highlight: highlight,
                    labelAtRadius: 44
                )
                .opacity(0.15 + reveal * 0.85)
                .scaleEffect(0.72 + reveal * 0.28)
            }
        }
        .opacity(reveal > 0.02 ? 1 : 0)
    }

    private func highlightForTool(index: Int) -> MIRSpatialMenuHighlightState {
        let item = (category?.children ?? [])[safe: index]
        guard let item, item.enabled else { return .disabled }
        guard state.toolIndex == index else { return .idle }
        return .active
    }

    // MARK: - Voice hint

    private func hintChip(_ hint: String) -> some View {
        Text(hint)
            .font(.system(size: 12, weight: .semibold, design: .rounded))
            .foregroundStyle(MirTheme.Colors.textPrimary)
            .padding(.horizontal, 14)
            .padding(.vertical, 8)
            .background(.ultraThinMaterial, in: Capsule())
            .overlay(Capsule().stroke(MirTheme.Colors.accentBright.opacity(0.35), lineWidth: 1))
            .position(x: containerCenter.x, y: containerCenter.y + 214)
            .transition(.opacity.combined(with: .scale(scale: 0.9)))
    }

    // MARK: - Angles

    private func sectorStart(index: Int, count: Int) -> Double {
        MIRSpatialMenuLayout.angle(forIndex: index, count: count) - Double.pi / Double(max(count, 1)) + 0.035
    }

    private func sectorEnd(index: Int, count: Int) -> Double {
        MIRSpatialMenuLayout.angle(forIndex: index, count: count) + Double.pi / Double(max(count, 1)) - 0.035
    }

    private func fanStart(centerAngle: Double, index: Int, count: Int, spread: Double) -> Double {
        MIRSpatialMenuLayout.toolAngle(index: index, count: count, centerAngle: centerAngle) - 0.28
    }

    private func fanEnd(centerAngle: Double, index: Int, count: Int, spread: Double) -> Double {
        MIRSpatialMenuLayout.toolAngle(index: index, count: count, centerAngle: centerAngle) + 0.28
    }
}

// MARK: - Fan arc segment

/// One arc segment of the fan with glow, label and highlight animation.
struct MIRSpatialMenuArcSegment: View {
    let center: CGPoint
    let radius: Double
    let thickness: Double
    let startAngle: Double
    let endAngle: Double
    let title: String
    let icon: String
    let highlight: MIRSpatialMenuHighlightState
    let labelAtRadius: Double

    private var color: Color {
        switch highlight {
        case .idle: return .white.opacity(0.30)
        case .hover: return MirTheme.Colors.accent.opacity(0.85)
        case .selected: return MirTheme.Colors.accentBright
        case .active: return MirTheme.Colors.success
        case .disabled: return .white.opacity(0.12)
        }
    }

    var body: some View {
        ZStack {
            arcPath
                .stroke(color, style: StrokeStyle(lineWidth: MIRSpatialMenuAnimation.strokeWidth(for: highlight) + 4, lineCap: .round))
                .blur(radius: MIRSpatialMenuAnimation.glow(for: highlight))
                .opacity(MIRSpatialMenuAnimation.glow(for: highlight) > 0 ? 1 : 0)

            arcPath
                .stroke(color, style: StrokeStyle(lineWidth: MIRSpatialMenuAnimation.strokeWidth(for: highlight), lineCap: .round))
                .opacity(MIRSpatialMenuAnimation.opacity(for: highlight))

            if !title.isEmpty || !icon.isEmpty {
                VStack(spacing: 2) {
                    if !icon.isEmpty {
                        Image(systemName: icon)
                            .font(.system(size: highlight == .idle || highlight == .disabled ? 11 : 14, weight: .semibold))
                    }
                    if !title.isEmpty {
                        Text(title)
                            .font(.system(size: 8, weight: .semibold))
                            .lineLimit(1)
                    }
                }
                .foregroundStyle(highlight == .idle ? .white.opacity(0.6) : .white)
                .position(labelPosition)
            }
        }
        .scaleEffect(MIRSpatialMenuAnimation.scale(for: highlight))
        .animation(MIRSpatialMenuAnimation.highlight, value: highlight)
    }

    private var arcPath: Path {
        Path { path in
            path.addArc(
                center: center,
                radius: radius,
                startAngle: .radians(startAngle),
                endAngle: .radians(endAngle),
                clockwise: false
            )
        }
    }

    private var labelPosition: CGPoint {
        let mid = (startAngle + endAngle) / 2
        return CGPoint(
            x: center.x + CGFloat(cos(mid) * labelAtRadius),
            y: center.y + CGFloat(sin(mid) * labelAtRadius)
        )
    }
}

private extension Array {
    subscript(safe index: Int) -> Element? {
        indices.contains(index) ? self[index] : nil
    }
}