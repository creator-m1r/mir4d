import Foundation
import AVFoundation
import Speech

/// Потокобезопасный (по контракту использования) держатель распознавателя.
/// `SFSpeechAudioBufferRecognitionRequest` не является Sendable, поэтому
/// помечаем холдер как `@unchecked Sendable` и обращаемся к request только
/// из одного потока записи (real-time audio tap) и из главного потока
/// (создание/отмена). Это устраняет MainActor-executor assertion в tap-callback
/// (ТЗ §5: real-time audio callback нельзя делать actor-isolated).
private final class VoiceRecognitionHolder {
    var request: SFSpeechAudioBufferRecognitionRequest?
}
extension VoiceRecognitionHolder: @unchecked Sendable {}

/// Local voice interface for the engineer.
/// No chat window and no microphone control are exposed in the creative workspace.
/// Recognition is explicitly requested on-device and commands are executed through CADAppState/EventBus.
@MainActor
final class MIR4DVoiceAssistant: NSObject, ObservableObject {
    enum State: Equatable {
        case idle
        case listening
        case processing
        case unavailable(String)
    }

    @Published private(set) var state: State = .idle
    @Published private(set) var lastTranscript = ""

    private let recognizer = SFSpeechRecognizer(locale: Locale(identifier: "ru-RU"))
    private let audioEngine = AVAudioEngine()
    private let synthesizer = AVSpeechSynthesizer()
    private var recognitionTask: SFSpeechRecognitionTask?
    /// nonisolated: к request обращается real-time audio tap (см. beginRecognition).
    nonisolated private let holder = VoiceRecognitionHolder()
    private weak var appState: CADAppState?
    private var restarting = false

    func start(appState: CADAppState) {
        self.appState = appState
        Task { await requestPermissionsAndListen() }
    }

    func stop() {
        recognitionTask?.cancel()
        recognitionTask = nil
        holder.request = nil
        audioEngine.stop()
        audioEngine.inputNode.removeTap(onBus: 0)
        state = .idle
    }

    private func requestPermissionsAndListen() async {
        let speechStatus = await requestSpeechAuthorization()
        guard speechStatus == .authorized else {
            state = .unavailable("Для голосового управления требуется разрешение на распознавание речи.")
            return
        }

        let micGranted = await requestMicrophonePermission()
        guard micGranted else {
            state = .unavailable("Для голосового управления требуется доступ к микрофону.")
            return
        }

        guard let recognizer, recognizer.supportsOnDeviceRecognition else {
            state = .unavailable("Локальное распознавание речи недоступно для выбранного языка на этом Mac.")
            return
        }

        beginRecognition()
    }

    // nonisolated: completion-блоки SFSpeechRecognizer.requestAuthorization /
    // AVCaptureDevice.requestAccess(for:) в этом SDK не @Sendable и наследуют
    // @MainActor-изоляцию. Если TCC-ответ приходит в фоновом потоке, рантайм
    // Swift вставляет swift_task_isCurrentExecutorWithFlagsImpl ->
    // dispatch_assert_queue_fail (EXC_BREAKPOINT, ТЗ §5). Делаем метод
    // nonisolated, тогда completion тоже nonisolated, а сам запрос TCC
    // выполняется на главном потоке вызова (ViewBridge-безопасно).
    private nonisolated func requestSpeechAuthorization() async -> SFSpeechRecognizerAuthorizationStatus {
        await withCheckedContinuation { continuation in
            SFSpeechRecognizer.requestAuthorization { status in
                continuation.resume(returning: status)
            }
        }
    }

    private nonisolated func requestMicrophonePermission() async -> Bool {
        await withCheckedContinuation { continuation in
            AVCaptureDevice.requestAccess(for: .audio) { granted in
                continuation.resume(returning: granted)
            }
        }
    }

    private func beginRecognition() {
        guard !audioEngine.isRunning, let recognizer, recognizer.isAvailable else { return }
        recognitionTask?.cancel()
        recognitionTask = nil

        let request = SFSpeechAudioBufferRecognitionRequest()
        request.shouldReportPartialResults = true
        request.requiresOnDeviceRecognition = true
        holder.request = request

        let inputNode = audioEngine.inputNode
        inputNode.removeTap(onBus: 0)
        let format = inputNode.outputFormat(forBus: 0)

        // Захватываем ТОЛЬКО holder (nonisolated + @unchecked Sendable).
        // Внутри tap-callback НЕТ self → Swift не вставляет проверку executor
        // (ТЗ §5: real-time audio callback запрещено делать actor-isolated).
        let holder = self.holder
        inputNode.installTap(onBus: 0, bufferSize: 4096, format: format) { @Sendable buffer, _ in
            guard let copied = Self.copyBuffer(buffer) else { return }
            holder.request?.append(copied)
        }

        recognitionTask = recognizer.recognitionTask(with: request) { [weak self] result, error in
            guard let self else { return }
            Task { @MainActor in
                if let result {
                    self.lastTranscript = result.bestTranscription.formattedString
                    NotificationCenter.default.post(name: .mir4DVoiceTranscript, object: self.lastTranscript)
                    if result.isFinal { self.handleCommand(self.lastTranscript) }
                }
                if error != nil || result?.isFinal == true {
                    self.restartRecognitionIfNeeded()
                }
            }
        }

        do {
            audioEngine.prepare()
            try audioEngine.start()
            state = .listening
        } catch {
            state = .unavailable("Не удалось запустить локальное распознавание речи.")
        }
    }

    private func restartRecognitionIfNeeded() {
        guard !restarting else { return }
        restarting = true
        audioEngine.stop()
        audioEngine.inputNode.removeTap(onBus: 0)
        holder.request = nil
        recognitionTask = nil
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.15) { [weak self] in
            guard let self else { return }
            self.restarting = false
            self.beginRecognition()
        }
    }

    private func handleCommand(_ raw: String) {
        let command = normalized(raw)
        guard !command.isEmpty else { return }
        state = .processing

        guard let appState else {
            speak("Я не вижу рабочую сцену.")
            return
        }

        if command.contains("новый проект") || command.contains("новый документ") {
            appState.newDocument()
            speak("Создаю новый проект.")
            return
        }
        if command.contains("создай тело") || command.contains("создать тело") || command.contains("создай куб") || command.contains("создать куб") {
            _ = MIR4DModelCommands.shared.createBox(appState: appState, width: 100, depth: 60, height: 40)
            speak("Создаю тело.")
            return
        }
        if command.contains("отмени") || command.contains("отменить") {
            MirEventBus.shared.publish(.undoRequested)
            speak("Отменяю последнее действие.")
            return
        }
        if command.contains("повтори") || command.contains("повторить") {
            MirEventBus.shared.publish(.redoRequested)
            speak("Повторяю действие.")
            return
        }
        if command.contains("покажи всё") || command.contains("показать всё") || command.contains("покажи все") {
            MirEventBus.shared.publish(.commandRequested("viewport.fit"))
            speak("Показываю всю сцену.")
            return
        }
        if command.contains("включи сетку") {
            appState.toggleGrid()
            speak("Сетка включена.")
            return
        }
        if command.contains("выключи сетку") {
            appState.toggleGrid()
            speak("Готово.")
            return
        }
        if command.contains("включи оси") {
            appState.toggleAxes()
            speak("Оси включены.")
            return
        }
        if command.contains("выключи оси") {
            appState.toggleAxes()
            speak("Готово.")
            return
        }
        if command.contains("эскиз") {
            appState.selectWorkbench(.sketch)
            if command.contains("лини") { appState.selectedTool = "line"; speak("Открываю эскиз и инструмент линии.") }
            else if command.contains("прямоуголь") { appState.selectedTool = "rectangle"; speak("Открываю эскиз и прямоугольник.") }
            else if command.contains("окруж") || command.contains("круг") { appState.selectedTool = "circle"; speak("Открываю эскиз и окружность.") }
            else { speak("Открываю эскиз.") }
            return
        }
        if command.contains("измер") {
            appState.selectedTool = "measure"
            speak("Инструмент измерения готов.")
            return
        }
        if command.contains("выдавлив") {
            MirEventBus.shared.publish(.commandRequested("model.extrude"))
            speak("Запускаю выдавливание.")
            return
        }
        if command.contains("вращен") {
            MirEventBus.shared.publish(.commandRequested("model.revolve"))
            speak("Запускаю вращение.")
            return
        }

        speak("Я услышал: \(raw). Команда пока не распознана.")
    }

    /// Надёжное копирование AVAudioPCMBuffer (buffer.copy() не всегда
    /// корректен для PCM). Вызывается из real-time audio callback, поэтому
    /// static + nonisolated — не захватывает self и не требует actor-executor.
    private nonisolated static func copyBuffer(_ buffer: AVAudioPCMBuffer) -> AVAudioPCMBuffer? {
        let format = buffer.format
        guard let copy = AVAudioPCMBuffer(pcmFormat: format, frameCapacity: buffer.frameCapacity) else {
            return nil
        }
        copy.frameLength = buffer.frameLength

        if let src = buffer.floatChannelData, let dst = copy.floatChannelData {
            for channel in 0..<Int(format.channelCount) {
                memcpy(dst[channel], src[channel], Int(buffer.frameLength) * MemoryLayout<Float>.size)
            }
        } else if let src = buffer.int16ChannelData, let dst = copy.int16ChannelData {
            for channel in 0..<Int(format.channelCount) {
                memcpy(dst[channel], src[channel], Int(buffer.frameLength) * MemoryLayout<Int16>.size)
            }
        } else if let src = buffer.int32ChannelData, let dst = copy.int32ChannelData {
            for channel in 0..<Int(format.channelCount) {
                memcpy(dst[channel], src[channel], Int(buffer.frameLength) * MemoryLayout<Int32>.size)
            }
        }
        return copy
    }

    private func normalized(_ value: String) -> String {
        value.lowercased()
            .replacingOccurrences(of: "ё", with: "е")
            .trimmingCharacters(in: .whitespacesAndNewlines)
    }

    private func speak(_ text: String) {
        state = .processing
        recognitionTask?.cancel()
        recognitionTask = nil
        holder.request = nil
        audioEngine.stop()
        audioEngine.inputNode.removeTap(onBus: 0)

        let utterance = AVSpeechUtterance(string: text)
        utterance.voice = AVSpeechSynthesisVoice(language: "ru-RU")
        utterance.rate = 0.48
        synthesizer.speak(utterance)

        DispatchQueue.main.asyncAfter(deadline: .now() + max(0.8, Double(text.count) * 0.045)) { [weak self] in
            guard let self, !self.synthesizer.isSpeaking else { return }
            self.beginRecognition()
        }
    }
}
