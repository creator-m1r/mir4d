import Foundation
import AppKit

/// Presentation-side bridge between SwiftUI selection controls and the native
/// viewport. It deliberately does not own CAD selection state; the viewport
/// remains the source of hit-testing and CADAppState remains the UI model.
final class ViewportSelectionBridge: NSObject {
    static let shared = ViewportSelectionBridge()

    private(set) var filter: String = "auto"
    private(set) var snapEnabled = true
    private(set) var additiveSelection = false

    private var observer: NSObjectProtocol?

    private override init() {
        super.init()
        observer = NotificationCenter.default.addObserver(
            forName: .mir4DSelectionFilterChanged,
            object: nil,
            queue: .main
        ) { [weak self] note in
            guard let value = note.object as? String else { return }
            self?.filter = value
            self?.applyToAttachedViews()
        }
    }

    deinit {
        if let observer { NotificationCenter.default.removeObserver(observer) }
    }

    func setSnapEnabled(_ enabled: Bool) {
        snapEnabled = enabled
        applyToAttachedViews()
    }

    func setAdditiveSelection(_ enabled: Bool) {
        additiveSelection = enabled
        applyToAttachedViews()
    }

    func attach(_ view: MirGLCustomView) {
        view.mirSelectionFilter = filter
        view.mirSnapEnabled = snapEnabled
        view.mirAdditiveSelection = additiveSelection
    }

    private func applyToAttachedViews() {
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
