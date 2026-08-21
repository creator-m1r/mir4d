import Foundation
import CoreGraphics

@MainActor
final class SketchParameterEditController: ObservableObject {
    @Published private(set) var pendingValue: Double?
    @Published private(set) var pendingKind: SketchDimensionState.Kind?
    @Published private(set) var isEditing = false

    func begin(kind: SketchDimensionState.Kind, value: Double) {
        pendingKind = kind
        pendingValue = value
        isEditing = true
    }

    func update(text: String) {
        let normalized = text.replacingOccurrences(of: ",", with: ".")
        pendingValue = Double(normalized)
    }

    func cancel() {
        pendingValue = nil
        pendingKind = nil
        isEditing = false
    }

    func commit() -> (SketchDimensionState.Kind, Double)? {
        guard let kind = pendingKind, let value = pendingValue else {
            cancel()
            return nil
        }
        cancel()
        return (kind, value)
    }
}
