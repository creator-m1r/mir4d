import SwiftUI
import Combine

@MainActor
final class MIR4DBrushIndicator: ObservableObject {
    static let shared = MIR4DBrushIndicator()

    @Published var active: Bool = false

    @Published var position: CGPoint = .zero

    @Published var radius: CGFloat = 0.2

    @Published var mode: MIR4DSculptIntent.Mode? = nil

    func show(x: Double, y: Double, radius: CGFloat, mode: MIR4DSculptIntent.Mode) {
        active = true
        position = CGPoint(x: x, y: y)
        self.radius = radius
        self.mode = mode
    }

    func hide() {
        active = false
        mode = nil
    }
}
