import Foundation

/// CAD model commands are the single entry point from UI actions into the
/// parameter/document model. Views do not mutate MIR4DModelDocument directly.
@MainActor
final class MIR4DModelCommands {
    static let shared = MIR4DModelCommands()

    private let runtime = MIR4DModelRuntime.shared
    private let session = MIR4DProjectSession.shared

    private init() {}

    @discardableResult
    func createBox(
        appState: CADAppState,
        width: Double,
        depth: Double,
        height: Double
    ) -> UUID? {
        guard width > 0, depth > 0, height > 0 else {
            appState.showNotification("Размеры должны быть больше нуля", type: .error)
            return nil
        }

        guard session.projectURL != nil else {
            appState.showNotification("Сначала создайте или откройте проект MIR 4D", type: .warning)
            return nil
        }

        let bodyID = runtime.addBox(width: width, depth: depth, height: height)
        appState.documentDirty = true
        session.scheduleAutoSave()

        NotificationCenter.default.post(
            name: .mir4DCreateBox,
            object: [
                "width": width,
                "depth": depth,
                "height": height,
                "bodyID": bodyID.uuidString
            ]
        )

        appState.setSelection(ids: [bodyID.uuidString], kind: .body)
        appState.showNotification("Создано тело: \(bodyID.uuidString.prefix(8))", type: .success)
        return bodyID
    }
}
