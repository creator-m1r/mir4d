import Foundation
import CoreGraphics

enum MIRSpatialMenuLayout {

    static func angle(forIndex index: Int, count: Int) -> Double {
        guard count > 0 else { return -Double.pi / 2 }
        return -Double.pi / 2 + (Double.pi * 2 / Double(count)) * Double(index)
    }

    static func position(radius: Double, angle: Double, center: CGPoint) -> CGPoint {
        CGPoint(
            x: center.x + CGFloat(cos(angle) * radius),
            y: center.y + CGFloat(sin(angle) * radius)
        )
    }

    static func submenuCenter(direction angle: Double, offset: Double, center: CGPoint) -> CGPoint {
        CGPoint(
            x: center.x + CGFloat(cos(angle) * offset),
            y: center.y + CGFloat(sin(angle) * offset)
        )
    }

    static func toolSpread(count: Int) -> Double {
        guard count > 1 else { return 0 }
        return min(Double.pi * 0.9, max(Double.pi * 0.42, Double(count - 1) * Double.pi / 9))
    }

    static func toolAngle(index: Int, count: Int, centerAngle: Double) -> Double {
        guard count > 0 else { return centerAngle }
        let spread = toolSpread(count: count)
        guard count > 1 else { return centerAngle }
        return centerAngle - spread / 2 + spread * Double(index) / Double(count - 1)
    }

    static func revealProgress(distance: Double, start: Double, end: Double) -> Double {
        guard end > start else { return 0 }
        return min(max((distance - start) / (end - start), 0), 1)
    }

    static func smoothstep(_ value: Double) -> Double {
        let x = min(max(value, 0), 1)
        return x * x * (3 - 2 * x)
    }

    static func normalizedAngle(_ value: Double) -> Double {
        let full = Double.pi * 2
        var result = value.truncatingRemainder(dividingBy: full)
        if result < 0 { result += full }
        return result
    }

    static func shortestSignedAngle(from: Double, to: Double) -> Double {
        atan2(sin(to - from), cos(to - from))
    }

    static func nearestSegmentIndex(angle: Double, count: Int) -> Int? {
        guard count > 0 else { return nil }
        let sector = Double.pi * 2 / Double(count)
        return Int(floor((normalizedAngle(angle) + sector / 2) / sector)) % count
    }

    static func magneticAngle(raw: Double, count: Int, strength: Double) -> Double {
        guard count > 0, let target = nearestSegmentIndex(angle: raw, count: count) else { return raw }
        let sector = Double.pi * 2 / Double(count)
        let centre = (Double(target) + 0.5) * sector
        let delta = atan2(sin(centre - raw), cos(centre - raw))
        return normalizedAngle(raw + delta * min(max(strength, 0), 1))
    }
}