import Foundation
import CoreGraphics

struct MirViewportHoverState: Equatable, Sendable {
    var point: CGPoint
    var filter: MirSelectionFilter
    var active: Bool

    static let inactive = MirViewportHoverState(
        point: .zero,
        filter: .auto,
        active: false
    )
}

extension Notification.Name {
    static let mir4DViewportHoverChanged = Notification.Name("MIR4D.ViewportHoverChanged")
}

@MainActor
final class ViewportHoverStateStore: ObservableObject {
    static let shared = ViewportHoverStateStore()

    @Published private(set) var state = MirViewportHoverState.inactive

    private init() {}

    func update(point: CGPoint, filter: MirSelectionFilter) {
        let next = MirViewportHoverState(point: point, filter: filter, active: true)
        guard next != state else { return }
        state = next
        NotificationCenter.default.post(name: .mir4DViewportHoverChanged, object: next)
    }

    func clear() {
        guard state.active else { return }
        state = .inactive
        NotificationCenter.default.post(name: .mir4DViewportHoverChanged, object: state)
    }
}
