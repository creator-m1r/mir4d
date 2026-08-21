import Foundation

struct MirViewportSelectionState: Equatable, Sendable {
    var filter: MirSelectionFilter
    var snapEnabled: Bool
    var additive: Bool

    static let `default` = MirViewportSelectionState(
        filter: .auto,
        snapEnabled: true,
        additive: false
    )
}

extension Notification.Name {
    static let mir4DViewportSelectionStateChanged = Notification.Name("MIR4D.ViewportSelectionStateChanged")
}

final class ViewportSelectionStateStore: ObservableObject {
    static let shared = ViewportSelectionStateStore()

    @Published private(set) var state = MirViewportSelectionState.default

    private var observers: [NSObjectProtocol] = []

    private init() {
        observers.append(NotificationCenter.default.addObserver(
            forName: .mir4DSelectionFilterChanged,
            object: nil,
            queue: .main
        ) { [weak self] note in
            guard let raw = note.object as? String,
                  let filter = MirSelectionFilter(rawValue: raw) else { return }
            self?.update(filter: filter)
        })
    }

    deinit {
        observers.forEach(NotificationCenter.default.removeObserver)
    }

    func update(filter: MirSelectionFilter? = nil, snapEnabled: Bool? = nil, additive: Bool? = nil) {
        var next = state
        if let filter { next.filter = filter }
        if let snapEnabled { next.snapEnabled = snapEnabled }
        if let additive { next.additive = additive }
        guard next != state else { return }
        state = next
        NotificationCenter.default.post(
            name: .mir4DViewportSelectionStateChanged,
            object: next
        )
    }
}
