import Foundation
import CoreGraphics

/// Unified inspector state for the active sketch entity.
@MainActor
final class SketchInspectorState: ObservableObject {
    struct GeometryInfo: Equatable {
        var type: String = "Нет выбора"
        var x1: Double?
        var y1: Double?
        var x2: Double?
        var y2: Double?
        var length: Double?
        var radius: Double?
        var angle: Double?
    }

    @Published private(set) var geometry = GeometryInfo()

    func clear() {
        geometry = GeometryInfo()
    }

    func inspectLine(from a: CGPoint, to b: CGPoint) {
        let dx = Double(b.x - a.x)
        let dy = Double(b.y - a.y)
        geometry = GeometryInfo(
            type: "Линия",
            x1: Double(a.x), y1: Double(a.y),
            x2: Double(b.x), y2: Double(b.y),
            length: hypot(dx, dy),
            radius: nil,
            angle: atan2(dy, dx) * 180.0 / .pi
        )
    }

    func inspectCircle(center: CGPoint, radius: CGFloat) {
        geometry = GeometryInfo(
            type: "Окружность",
            x1: Double(center.x), y1: Double(center.y),
            x2: nil, y2: nil,
            length: nil,
            radius: Double(radius),
            angle: nil
        )
    }
}
