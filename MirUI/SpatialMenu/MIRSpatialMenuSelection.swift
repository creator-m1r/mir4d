import Foundation
import CoreGraphics

/// Three-level directional selection for the spatial fan.
///
/// The gesture is one continuous motion:
/// ```text
/// press → activate → move → highlight → enter submenu → move → highlight → release → execute
/// ```
/// There is no clicking on small buttons: selection follows the movement direction.
enum MIRSpatialMenuSelection {

    /// Computes the full selection state for a gesture vector.
    /// `previous` enables hysteresis so the highlight never flickers at ring boundaries.
    static func state(
        vector: CGVector,
        tree: [MIRSpatialMenuItem],
        settings: MIRSpatialMenuSettings,
        previous: MIRSpatialMenuState?
    ) -> MIRSpatialMenuState {
        let dx = Double(vector.dx)
        let dy = Double(vector.dy)
        let distance = hypot(dx, dy)
        guard !tree.isEmpty else { return .initial }
        guard distance >= settings.deadZone else {
            return MIRSpatialMenuState(level: .intent, dx: dx, dy: dy)
        }

        let rawAngle = atan2(dy, dx)
        let visible = Array(tree.prefix(min(tree.count, settings.maxLevel1Segments)))

        // Level 1: intention ring, magnetically stabilised.
        let intentAngle = MIRSpatialMenuLayout.magneticAngle(
            raw: rawAngle,
            count: visible.count,
            strength: settings.magneticStrength
        )
        let intentIndex = hystereticSegment(
            angle: intentAngle,
            count: visible.count,
            previous: previous?.intentIndex,
            hysteresis: settings.magneticHysteresis
        )

        var state = MIRSpatialMenuState(
            level: .intent,
            intentIndex: intentIndex,
            dx: dx,
            dy: dy
        )

        guard let intentIndex, visible.indices.contains(intentIndex) else { return state }
        let intent = visible[intentIndex]
        guard !intent.children.isEmpty else { return state }

        // Level 2: category ring, revealed along the movement direction.
        state.level = .category
        let categoryReveal = MIRSpatialMenuLayout.revealProgress(
            distance: distance,
            start: settings.intentRadius,
            end: settings.categoryOffset
        )
        guard categoryReveal > 0.18, distance >= settings.categoryOffset else { return state }

        let categoryCount = intent.children.count
        let spread = MIRSpatialMenuLayout.toolSpread(count: categoryCount)
        let categoryCenter = intentAngle
        let relativeCategory = MIRSpatialMenuLayout.shortestSignedAngle(from: categoryCenter, to: rawAngle)
        let categoryIndex = fanSegment(
            relativeAngle: relativeCategory,
            centerAngle: categoryCenter,
            count: categoryCount,
            spread: spread,
            previous: previous?.categoryIndex,
            hysteresis: settings.magneticHysteresis
        )
        guard let categoryIndex, intent.children.indices.contains(categoryIndex) else { return state }
        state.categoryIndex = categoryIndex
        let category = intent.children[categoryIndex]
        guard !category.children.isEmpty else { return state }

        // Level 3: tool ring, revealed along the category direction.
        state.level = .tool
        let toolReveal = MIRSpatialMenuLayout.revealProgress(
            distance: distance,
            start: settings.categoryOffset,
            end: settings.toolOffset
        )
        guard toolReveal > 0.18, distance >= settings.toolOffset else { return state }

        let toolCount = category.children.count
        let toolSpread = MIRSpatialMenuLayout.toolSpread(count: toolCount)
        let toolCenter = MIRSpatialMenuLayout.toolAngle(
            index: categoryIndex,
            count: categoryCount,
            centerAngle: categoryCenter
        )
        let relativeTool = MIRSpatialMenuLayout.shortestSignedAngle(from: toolCenter, to: rawAngle)
        let toolIndex = fanSegment(
            relativeAngle: relativeTool,
            centerAngle: toolCenter,
            count: toolCount,
            spread: toolSpread,
            previous: previous?.toolIndex,
            hysteresis: settings.magneticHysteresis
        )
        guard let toolIndex, category.children.indices.contains(toolIndex) else { return state }
        state.toolIndex = toolIndex
        return state
    }

    /// Resolved tool command of a state, if the gesture reached a tool.
    static func resolvedTool(
        _ state: MIRSpatialMenuState,
        tree: [MIRSpatialMenuItem]
    ) -> MIRSpatialMenuItem? {
        guard state.level == .tool,
              let intentIndex = state.intentIndex,
              tree.indices.contains(intentIndex) else { return nil }
        let intent = tree[intentIndex]
        guard let categoryIndex = state.categoryIndex,
              intent.children.indices.contains(categoryIndex) else { return nil }
        let category = intent.children[categoryIndex]
        guard let toolIndex = state.toolIndex,
              category.children.indices.contains(toolIndex) else { return nil }
        return category.children[toolIndex]
    }

    // MARK: - Segment helpers

    /// Hysteretic segment index for a full ring: a segment can only change after
    /// the angle crosses its centre plus an angular buffer.
    private static func hystereticSegment(
        angle: Double,
        count: Int,
        previous: Int?,
        hysteresis: Double
    ) -> Int? {
        guard count > 0 else { return nil }
        let sector = Double.pi * 2 / Double(count)
        let candidate = MIRSpatialMenuLayout.nearestSegmentIndex(angle: angle, count: count) ?? 0
        guard let previous, previous >= 0, previous < count else { return candidate }
        if candidate == previous { return previous }
        let previousCentre = (Double(previous) + 0.5) * sector
        let distanceFromCentre = abs(MIRSpatialMenuLayout.shortestSignedAngle(from: previousCentre, to: angle))
        let threshold = sector * (0.5 + min(max(hysteresis, 0), 0.45))
        return distanceFromCentre > threshold ? candidate : previous
    }

    /// Segment index inside a fan submenu expressed by its relative angle.
    /// The fan is centred on `centerAngle` and spreads `spread` radians wide.
    private static func fanSegment(
        relativeAngle: Double,
        centerAngle: Double,
        count: Int,
        spread: Double,
        previous: Int?,
        hysteresis: Double
    ) -> Int? {
        guard count > 0 else { return nil }
        guard count > 1 else { return 0 }

        let candidateRaw = (relativeAngle + spread / 2) / spread
        let candidate = min(max(Int((candidateRaw * Double(count - 1)).rounded()), 0), count - 1)

        guard let previous, previous >= 0, previous < count else { return candidate }
        if candidate == previous { return previous }

        let previousCentre = MIRSpatialMenuLayout.toolAngle(index: previous, count: count, centerAngle: centerAngle)
        let distanceFromCentre = abs(MIRSpatialMenuLayout.shortestSignedAngle(from: previousCentre, to: centerAngle + relativeAngle))
        let sector = spread / Double(max(count - 1, 1))
        let threshold = sector * (0.5 + min(max(hysteresis, 0), 0.45))
        return distanceFromCentre > threshold ? candidate : previous
    }
}