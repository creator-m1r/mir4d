import Foundation
import CoreGraphics

/// UI-side parametric dimension state.
/// Values are proposals until committed through MirEngine commands.
@MainActor
final class SketchDimensionState: ObservableObject {
    enum Kind: String, CaseIterable {
        case length = "Длина"
        case radius = "Радиус"
        case angle = "Угол"
        case horizontal = "X"
        case vertical = "Y"
    }

    struct Dimension: Identifiable, Equatable {
        let id = UUID()
        var kind: Kind
        var value: Double
        var unit: String = "мм"
        var driven: Bool = true
    }

    @Published private(set) var dimensions: [Dimension] = []

    func addLength(_ value: CGFloat) {
        dimensions.append(Dimension(kind: .length, value: Double(value)))
    }

    func addRadius(_ value: CGFloat) {
        dimensions.append(Dimension(kind: .radius, value: Double(value)))
    }

    func addAngle(_ value: CGFloat) {
        dimensions.append(Dimension(kind: .angle, value: Double(value), unit: "°"))
    }

    func clear() {
        dimensions.removeAll()
    }
}
