import Foundation

/// Resolves viewport/engine identities back to the persisted CAD model.
///
/// The viewport may report a native MirEngine object ID, but the CAD document
/// identity exposed to SwiftUI remains the persisted body UUID. Keeping this
/// conversion in the model runtime prevents views from inventing a second ID.
@MainActor
extension MIR4DModelRuntime {
    func persistedBodyID(forEngineObjectID objectID: UInt64) -> UUID? {
        document.bodyID(forEngineObjectID: objectID)
    }

    func persistedSelectionID(forEngineObjectID objectID: UInt64) -> String? {
        persistedBodyID(forEngineObjectID: objectID)?.uuidString
    }
}
