import Foundation
import MirServer

/// Адаптер совместной работы: связывает `MIR4DModelRuntime` с подсистемой
/// `MirServer.Collaboration`, реализуя протокол `MirCollaborativeDocument`.
///
/// Позволяет применять входящие операции коллег к реальной модели проекта без
/// прямой зависимости подсистемы MirServer от CAD-ядра.
extension MIR4DModelRuntime: MirCollaborativeDocument {

    func applyCollaborationOperation(_ operation: MirCollaborationOperation) throws {
        guard let entityID = UUID(uuidString: operation.entityID) else { return }

        switch operation.kind {
        case .create:
            let params = (try? JSONDecoder().decode(MirCollaborationCreateParameters.self, from: operation.parameters))
                ?? MirCollaborationCreateParameters(width: 1, depth: 1, height: 1)
            _ = addBox(width: params.width, depth: params.depth, height: params.height, bodyID: entityID)

        case .transform:
            let transform = (try? JSONDecoder().decode(MirCollaborationTransform.self, from: operation.parameters))
                ?? MirCollaborationTransform()
            applyCollaborationTransform(bodyID: entityID, matrix: transform.matrix)

        case .delete:
            deleteBodyCollaboratively(entityID)

        case .rename:
            let name = (try? JSONDecoder().decode(MirCollaborationRenameParameters.self, from: operation.parameters).name)
                ?? "Тело"
            renameBody(entityID, to: name)

        case .select:
            break
        case .updateMeta:
            break
        }
    }

    /// Удаление тела по идентификатору через существующий механизм viewport.
    private func deleteBodyCollaboratively(_ bodyID: UUID) {
        guard let operation = document.operations.first(where: { $0.bodyID == bodyID }),
              let geometry = document.geometry.first(where: { $0.id == operation.featureIDs.first }),
              let engineObjectID = geometry.parameters["engineObjectID"] else {
            return
        }
        removeBody(forViewportObjectID: UInt64(engineObjectID))
    }

    func currentCollaborationSnapshot() throws -> MirProjectSnapshot {
        let entities = document.bodies.map { body in
            MirCollaborationEntityState(
                entityID: body.id.uuidString,
                type: "body",
                transform: [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1],
                name: body.name
            )
        }
        return MirProjectSnapshot(
            projectID: MIR4DProjectSession.shared.projectUUID?.uuidString ?? "",
            entities: entities,
            baseClock: MirOperationClock(counter: 0, authorID: "snapshot")
        )
    }
}
