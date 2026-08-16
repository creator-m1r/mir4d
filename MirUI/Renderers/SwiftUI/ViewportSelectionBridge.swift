import Foundation

/// Presentation-side bridge between SwiftUI selection controls and the native
/// viewport. It deliberately does not own CAD selection state.
///
/// The current MirGLCustomView exposes the rendering/picking surface through
/// its existing callback and C ABI. We therefore publish a small, stable
/// notification payload here instead of inventing Swift properties on the
/// native view. The native viewport can subscribe when its picking API is
/// ready, without coupling SwiftUI to MirEngine internals.
final class ViewportSelectionBridge: NSObject {
    static let shared = ViewportSelectionBridge()

    private(set) var filter: String = "auto"
    private(set) var snapEnabled = true
    private(set) var additiveSelection = false

    private var filterObserver: NSObjectProtocol?

    private override init() {
        super.init()
        filterObserver = NotificationCenter.default.addObserver(
            forName: .mir4DSelectionFilterChanged,
            object: nil,
            queue: .main
        ) { [weak self] note in
            guard let value = note.object as? String else { return }
            self?.filter = value
            self?.publish()
        }
    }

    deinit {
        if let filterObserver {
            NotificationCenter.default.removeObserver(filterObserver)
        }
    }

    func setSnapEnabled(_ enabled: Bool) {
        snapEnabled = enabled
        publish()
    }

    func setAdditiveSelection(_ enabled: Bool) {
        additiveSelection = enabled
        publish()
    }

    func publish() {
        NotificationCenter.default.post(
            name: .mir4DViewportSelectionStateChanged,
            object: self,
            userInfo: [
                "filter": filter,
                "snapEnabled": snapEnabled,
                "additiveSelection": additiveSelection
            ]
        )
    }
}

extension Notification.Name {
    static let mir4DViewportSelectionStateChanged = Notification.Name("MIR4D.ViewportSelectionStateChanged")
}
