import Foundation

/// UI state for automatic sketch constraints.
/// Constraint solving itself remains in MirEngine/Sketch.
@MainActor
final class SketchConstraintState: ObservableObject {
    @Published var autoConstraintsEnabled = true
    @Published private(set) var detected: [DetectedConstraint] = []

    struct DetectedConstraint: Identifiable, Equatable {
        let id = UUID()
        let title: String
        let symbol: String
    }

    func clear() {
        detected.removeAll()
    }

    func suggestHorizontal() {
        guard autoConstraintsEnabled else { return }
        detected.append(DetectedConstraint(title: "Горизонтальность", symbol: "—"))
    }

    func suggestVertical() {
        guard autoConstraintsEnabled else { return }
        detected.append(DetectedConstraint(title: "Вертикальность", symbol: "│"))
    }

    func suggestCoincident() {
        guard autoConstraintsEnabled else { return }
        detected.append(DetectedConstraint(title: "Совпадение", symbol: "●"))
    }
}
