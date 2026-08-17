import Foundation

#if canImport(MirServer)
import MirServer
#endif

/// CAD model commands are the single entry point from UI actions into the
/// parameter/document model. Views do not mutate MIR4DModelDocument directly.
@MainActor
final class MIR4DModelCommands {
    static let shared = MIR4DModelCommands()

    private let runtime = MIR4DModelRuntime.shared
    private let session = MIR4DProjectSession.shared

    private var observers: [NSObjectProtocol] = []

    private init() {
        let center = NotificationCenter.default
        observers.append(center.addObserver(forName: .mir4DRunCAECampaign, object: nil, queue: .main) { [weak self] note in
            let payload = note.userInfo?["definition"] as? String
            MainActor.assumeIsolated {
                guard let self else { return }
                let definition = payload ?? MIR4DModelCommands.builtInCAEDefinition
                self.handleCAE(definition: definition)
            }
        })
    }

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

        #if canImport(MirServer)
        // Транслировать создание примитива команде при совместной работе.
        let params = (try? JSONEncoder().encode(["width": width, "depth": depth, "height": height])) ?? Data()
        MirCollaborationController.shared.applyLocal(kind: .create, entityID: bodyID.uuidString, parameters: params)
        #endif

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

    /// ТЗ Этап 1: создать пользовательскую рабочую плоскость параллельно базовой.
    func createWorkPlane(
        basePlane: UInt32,
        offset: Double = 10.0,
        angleDeg: Double = 0.0
    ) {
        NotificationCenter.default.post(
            name: .mir4DCreateWorkPlane,
            object: nil,
            userInfo: [
                "basePlane": basePlane,
                "offset": offset,
                "angleDeg": angleDeg
            ]
        )
    }

    /// ТЗ Этап 2: создать эскиз-прямоугольник через универсальный решатель ограничений.
    func createSketchRectangle(
        appState: CADAppState,
        width: Double = 40.0,
        height: Double = 25.0
    ) {
        guard session.projectURL != nil else {
            appState.showNotification("Сначала создайте или откройте проект MIR 4D", type: .warning)
            return
        }
        let corners = SketchController.shared.createSolvedRectangle(width: Float(width), height: Float(height))
        guard let corners else {
            appState.showNotification("Не удалось решить эскиз: ограничения противоречивы", type: .error)
            return
        }
        NotificationCenter.default.post(
            name: .mir4DSketchSolved,
            object: nil,
            userInfo: ["corners": corners, "width": width, "height": height]
        )
        appState.showNotification("Эскиз построен решателем ограничений: \(corners.count) углов", type: .success)
    }

    /// ТЗ CAE: запустить встроенную комплексную мультифизическую кампанию
    /// испытаний через Event Bus. Результат приходит в `.mir4DCAEResult`.
    func runBuiltInCAECampaign() {
        NotificationCenter.default.post(
            name: .mir4DRunCAECampaign,
            object: nil,
            userInfo: ["definition": MIR4DModelCommands.builtInCAEDefinition]
        )
    }

    // MARK: - CAE Event Bus handler

    private func handleCAE(definition: String) {

        guard let report = MIR4DRunCAECampaign(definition: definition) else {
            NotificationCenter.default.post(
                name: .mir4DCAEResult,
                object: nil,
                userInfo: ["error": "CAE-движок недоступен"]
            )
            return
        }

        let url = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent("cae_campaign_report.json")
        try? report.write(to: url, atomically: true, encoding: .utf8)

        NotificationCenter.default.post(
            name: .mir4DCAEResult,
            object: nil,
            userInfo: ["report": report, "path": url.path]
        )
    }

    /// Встроенное определение кампании испытаний (текстовый грамматикой CAECampaign).
    nonisolated static let builtInCAEDefinition = """
    case hot
      material temperature 350
      initial flowRate 5
      criterion temperature 0 400
      criterion stress 0 1e9
    case soft
      material youngModulus 1e11
      initial flowRate 5
      criterion stress 0 1e11
    """

}
