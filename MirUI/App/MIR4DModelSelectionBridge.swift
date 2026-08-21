import Foundation

@MainActor
extension MIR4DModelRuntime {
    func persistedBodyID(forEngineObjectID objectID: UInt64) -> UUID? {
        if let viewportBodyID = persistedBodyID(forViewportEngineObjectID: objectID) {
            return viewportBodyID
        }
        return document.bodyID(forEngineObjectID: objectID)
    }

    func persistedSelectionID(forEngineObjectID objectID: UInt64) -> String? {
        persistedBodyID(forEngineObjectID: objectID)?.uuidString
    }
}
