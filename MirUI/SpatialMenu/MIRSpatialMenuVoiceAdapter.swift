import Foundation

/// Maps spoken commands into spatial intents.
///
/// Speech recognition stays in MIR4DVoiceAssistant; this adapter is only the
/// voice → Intent channel. It deliberately contains no speech engine, no chat
/// window and no CAD algorithms — it produces the same MIRIntent the fan does:
///
/// ```text
/// «Создай эскиз» → CREATE → SKETCH
/// «Вытяни на сорок» → target: selectedSurface, action: extrude, distance: 40
/// ```
@MainActor
final class MIRSpatialMenuVoiceAdapter: ObservableObject {
    static let shared = MIRSpatialMenuVoiceAdapter()

    /// Last recognized intention, shown as a quiet hint above the centre.
    @Published private(set) var lastVoiceHint: String?
    @Published private(set) var lastParsedIntent: MIRIntent?

    private var observer: NSObjectProtocol?
    private weak var appState: CADAppState?

    private init() {}

    func connect(appState: CADAppState) {
        self.appState = appState
        guard observer == nil else { return }

        observer = NotificationCenter.default.addObserver(
            forName: .mir4DVoiceTranscript,
            object: nil,
            queue: .main
        ) { [weak self] notification in
            guard let transcript = notification.object as? String else { return }
            Task { @MainActor [weak self] in
                self?.ingest(transcript)
            }
        }
    }

    func disconnect() {
        if let observer {
            NotificationCenter.default.removeObserver(observer)
            self.observer = nil
        }
        lastVoiceHint = nil
        lastParsedIntent = nil
    }

    func ingest(_ raw: String) {
        let text = normalized(raw)
        guard !text.isEmpty else { return }

        let targetID = appState?.selection.ids.first

        if let distance = extractDistance(from: text),
           text.contains("вытян") || text.contains("выдав") || text.contains("выдави") {
            publish(
                action: "model.extrude",
                value: distance,
                targetID: targetID,
                hint: "CREATE → SOLID → EXTRUDE · \(distance)"
            )
            return
        }

        if text.contains("вращен") {
            publish(action: "model.revolve", hint: "CREATE → SOLID → REVOLVE")
            return
        }

        if text.contains("эскиз") {
            if text.contains("вытян") || text.contains("выдав") {
                publish(action: "model.extrude", hint: "SKETCH → EXIT → EXTRUDE")
            } else {
                publish(action: "create.sketch", hint: "CREATE → SKETCH")
            }
            return
        }

        if text.contains("создай тело") || text.contains("создать тело") || text.contains("куб") {
            publish(action: "create.body", hint: "CREATE → SOLID")
            return
        }

        if text.contains("измер") {
            publish(action: "measure.distance", hint: "MEASURE → DISTANCE")
            return
        }

        if text.contains("отмени") {
            publish(action: "history.undo", hint: "EDIT → HISTORY → UNDO")
            return
        }

        if text.contains("повтори") {
            publish(action: "history.redo", hint: "EDIT → HISTORY → REDO")
            return
        }

        if text.contains("покажи всё") || text.contains("показать всё") || text.contains("покажи все") {
            publish(action: "viewport.fit", hint: "VIEW → CAMERA → FIT")
            return
        }

        if text.contains("сетку") {
            publish(action: "viewport.grid", hint: "VIEW → DISPLAY → GRID")
            return
        }
    }

    // MARK: - Parsing

    /// «на сорок» / «40 мм» → 40.0
    private func extractDistance(from text: String) -> Double? {
        let pattern = #"на\s+([0-9]+(?:\.[0-9]+)?)"#
        guard let range = text.range(of: pattern, options: .regularExpression) else { return nil }
        let digits = text[range].replacingOccurrences(of: "на", with: "")
            .trimmingCharacters(in: .whitespaces)
        return Double(digits)
    }

    private func normalized(_ value: String) -> String {
        value.lowercased()
            .replacingOccurrences(of: "ё", with: "е")
            .trimmingCharacters(in: .whitespacesAndNewlines)
    }

    private func publish(action: String, value: Double? = nil, targetID: String? = nil, hint: String) {
        let intent = MIRIntent(
            source: .voice,
            phase: .selection,
            action: action,
            targetID: targetID,
            value: value,
            confidence: 0.94
        )
        lastVoiceHint = hint
        lastParsedIntent = intent
        MIRIntentRouter.shared.publish(intent)
    }
}

extension Notification.Name {
    static let mir4DVoiceTranscript = Notification.Name("MIR4D.VoiceTranscript")
}