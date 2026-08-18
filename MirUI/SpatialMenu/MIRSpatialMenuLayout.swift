import Foundation
import CoreGraphics

/// Pure radial layout geometry for the spatial fan.
/// No rendering, no state: positions are derived from settings only.
enum MIRSpatialMenuLayout {

    /// Angle of the i-th segment of a ring with `count` segments.
    /// The fan opens upward (-π/2), like a compass.
    static func angle(forIndex index: Int, count: Int) -> Double {
        guard count > 0 else { return -Double.pi / 2 }
        return -Double.pi / 2 + (Double.pi * 2 / Double(count)) * Double(index)
    }

    /// Cartesian position of a segment on a ring.
    static func position(radius: Double, angle: Double, center: CGPoint) -> CGPoint {
        CGPoint(
            x: center.x + CGFloat(cos(angle) * radius),
            y: center.y + CGFloat(sin(angle) * radius)
        )
    }

    /// Centre of a submenu ring, shifted along the current movement direction.
    static func submenuCenter(direction angle: Double, offset: Double, center: CGPoint) -> CGPoint {
        CGPoint(
            x: center.x + CGFloat(cos(angle) * offset),
            y: center.y + CGFloat(sin(angle) * offset)
        )
    }

    /// Angular spread of a submenu with `count` tools around the movement direction.
    /// A single tool sits exactly on the direction; more tools open like a fan.
    static func toolSpread(count: Int) -> Double {
        guard count > 1 else { return 0 }
        return min(Double.pi * 0.9, max(Double.pi * 0.42, Double(count - 1) * Double.pi / 9))
    }

    /// Angle of the i-th tool of a submenu centred on `centerAngle`.
    static func toolAngle(index: Int, count: Int, centerAngle: Double) -> Double {
        guard count > 0 else { return centerAngle }
        let spread = toolSpread(count: count)
        guard count > 1 else { return centerAngle }
        return centerAngle - spread / 2 + spread * Double(index) / Double(count - 1)
    }

    /// Reveal progress of the second/third ring as the finger travels outward.
    static func revealProgress(distance: Double, start: Double, end: Double) -> Double {
        guard end > start else { return 0 }
        return min(max((distance - start) / (end - start), 0), 1)
    }

    /// Smoothstep easing for ring reveal.
    static func smoothstep(_ value: Double) -> Double {
        let x = min(max(value, 0), 1)
        return x * x * (3 - 2 * x)
    }

    /// Normalized angle in [0, 2π).
    static func normalizedAngle(_ value: Double) -> Double {
        let full = Double.pi * 2
        var result = value.truncatingRemainder(dividingBy: full)
        if result < 0 { result += full }
        return result
    }

    /// Shortest signed angle from `from` to `to` in (-π, π].
    static func shortestSignedAngle(from: Double, to: Double) -> Double {
        atan2(sin(to - from), cos(to - from))
    }

    /// Index of the segment nearest to `angle` in a ring of `count` segments.
    static func nearestSegmentIndex(angle: Double, count: Int) -> Int? {
        guard count > 0 else { return nil }
        let sector = Double.pi * 2 / Double(count)
        return Int(floor((normalizedAngle(angle) + sector / 2) / sector)) % count
    }

    /// Magnetic blend of a raw angle toward the nearest segment centre.
    static func magneticAngle(raw: Double, count: Int, strength: Double) -> Double {
        guard count > 0, let target = nearestSegmentIndex(angle: raw, count: count) else { return raw }
        let sector = Double.pi * 2 / Double(count)
        let centre = (Double(target) + 0.5) * sector
        let delta = atan2(sin(centre - raw), cos(centre - raw))
        return normalizedAngle(raw + delta * min(max(strength, 0), 1))
    }
}