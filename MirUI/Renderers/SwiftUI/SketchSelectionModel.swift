import SwiftUI
import Combine

@MainActor
final class SketchSelectionModel: ObservableObject {
    @Published private(set) var selectedGeometryIDs: Set<UInt32> = []

    var allowsMultipleSelection = false

    func select(_ id: UInt32, additive: Bool = false) {
        if additive || allowsMultipleSelection {
            if selectedGeometryIDs.contains(id) {
                selectedGeometryIDs.remove(id)
            } else {
                selectedGeometryIDs.insert(id)
            }
        } else {
            selectedGeometryIDs = [id]
        }
    }

    func clear() {
        selectedGeometryIDs.removeAll()
    }

    func contains(_ id: UInt32) -> Bool {
        selectedGeometryIDs.contains(id)
    }
}
