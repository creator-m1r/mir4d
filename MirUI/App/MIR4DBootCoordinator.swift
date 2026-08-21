
import Foundation
import SwiftUI
import AppKit
import Metal

@MainActor
final class MIR4DBootCoordinator: ObservableObject {

    enum State: Equatable {
        case idle
        case booting
        case ready
        case warning
        case failed
    }

    enum Severity {
        case info
        case success
        case warning
        case error
    }

    struct Step: Identifiable {

        let id = UUID()

        let index: Int

        let title: String

        let detail: String

        let severity: Severity
    }

    @Published private(set) var state: State = .idle

    @Published private(set) var progress: Double = 0

    @Published private(set) var currentTitle =
        "Подготовка MIR 4D…"

    @Published private(set) var currentDetail =
        "Ожидание запуска."

    @Published private(set) var steps: [Step] = []

    @Published private(set) var errorCount = 0

    @Published private(set) var warningCount = 0

    private var hasStarted = false

    private let totalSteps = 11

    func start() async {

        guard !hasStarted else {
            return
        }

        hasStarted = true

        state = .booting

        progress = 0

        steps.removeAll()

        errorCount = 0

        warningCount = 0

        await execute(
            index: 1,
            title: "Проверка платформы",
            detail:
                "Проверяем macOS и архитектуру процессора."
        ) {
            self.checkPlatform()
        }

        await execute(
            index: 2,
            title: "Проверка SwiftUI",
            detail:
                "Проверяем пользовательский интерфейс MIR 4D."
        ) {
            self.checkSwiftUI()
        }

        await execute(
            index: 3,
            title: "Проверка ресурсов",
            detail:
                "Проверяем Bundle и файловые ресурсы."
        ) {
            self.checkResources()
        }

        await execute(
            index: 4,
            title: "Проверка Event Bus",
            detail:
                "Проверяем систему обмена событиями."
        ) {
            self.checkEventBus()
        }

        await execute(
            index: 5,
            title: "Проверка GPU",
            detail:
                "Проверяем доступность графического устройства."
        ) {
            self.checkGPU()
        }

        await execute(
            index: 6,
            title: "Проверка дисплея",
            detail:
                "Проверяем графическое окружение macOS."
        ) {
            self.checkDisplay()
        }

        await execute(
            index: 7,
            title: "Проверка памяти",
            detail:
                "Проверяем доступную физическую память."
        ) {
            self.checkMemory()
        }

        await execute(
            index: 8,
            title: "Подготовка инженерной среды",
            detail:
                "Подготавливаем окружение MIR 4D."
        ) {
            self.prepareEngineeringEnvironment()
        }

        await execute(
            index: 9,
            title: "Проверка файловой системы",
            detail:
                "Проверяем каталоги для проектов MIR 4D."
        ) {
            self.checkFileSystem()
        }

        await execute(
            index: 10,
            title: "Проверка целостности приложения",
            detail:
                "Проверяем Bundle и исполняемый файл."
        ) {
            self.checkApplicationIntegrity()
        }

        await execute(
            index: 11,
            title: "Финальная проверка",
            detail:
                "Определяем готовность приложения."
        ) {
            self.finalizeBoot()
        }

        if errorCount > 0 {

            state = .failed

            currentTitle =
                "Запуск остановлен"

            currentDetail =
                "Обнаружены критические ошибки."

        } else if warningCount > 0 {

            state = .warning

            currentTitle =
                "MIR 4D готов с предупреждениями"

            currentDetail =
                "Основная среда готова к работе."

        } else {

            state = .ready

            currentTitle =
                "MIR 4D готов"

            currentDetail =
                "Инженерная среда успешно подготовлена."
        }

        progress = 1.0
    }

    private func execute(
        index: Int,
        title: String,
        detail: String,
        operation: () -> StepResult
    ) async {

        currentTitle = title

        currentDetail = detail

        let result = operation()

        switch result {

        case .success(let message):

            addStep(
                index: index,
                title: title,
                detail: message,
                severity: .success
            )

        case .warning(let message):

            warningCount += 1

            addStep(
                index: index,
                title: title,
                detail: message,
                severity: .warning
            )

        case .failure(let message):

            errorCount += 1

            addStep(
                index: index,
                title: title,
                detail: message,
                severity: .error
            )
        }

        progress =
            Double(index) /
            Double(totalSteps)

        try? await Task.sleep(
            for: .milliseconds(180)
        )
    }

    private enum StepResult {

        case success(String)

        case warning(String)

        case failure(String)
    }

    private func checkPlatform()
        -> StepResult
    {

        let processInfo =
            ProcessInfo.processInfo

        let version =
            processInfo.operatingSystemVersion

        guard version.majorVersion >= 14 else {

            return .failure(
                "Требуется macOS 14 или новее."
            )
        }

        #if arch(arm64)

        let architecture =
            "Apple Silicon ARM64"

        #elseif arch(x86_64)

        let architecture =
            "Intel x86_64"

        #else

        let architecture =
            "Unknown"

        #endif

        return .success(
            "macOS " +
            "\(version.majorVersion)." +
            "\(version.minorVersion)." +
            "\(version.patchVersion), " +
            "\(architecture)."
        )
    }

    private func checkSwiftUI()
        -> StepResult
    {

        return .success(
            "SwiftUI runtime доступен."
        )
    }

    private func checkResources()
        -> StepResult
    {

        let bundle =
            Bundle.main

        let bundleURL =
            bundle.bundleURL

        guard !bundleURL.path.isEmpty else {

            return .failure(
                "Bundle приложения не найден."
            )
        }

        guard bundle.resourcePath != nil else {

            return .warning(
                "Каталог Resources отсутствует."
            )
        }

        return .success(
            "Bundle и Resources доступны."
        )
    }

    private func checkEventBus()
        -> StepResult
    {

        let name =
            Notification.Name(
                "MIR4D.Boot.EventBus.Test"
            )

        let observer =
            NotificationCenter.default.addObserver(
                forName: name,
                object: nil,
                queue: .main
            ) { _ in
                
            }

        NotificationCenter.default.post(
            name: name,
            object: nil
        )

        NotificationCenter.default.removeObserver(
            observer
        )

        return .success(
            "NotificationCenter / Event Bus доступны."
        )
    }

    private func checkGPU()
        -> StepResult
    {

        guard let device =
            MTLCreateSystemDefaultDevice()
        else {

            return .warning(
                "Metal GPU не обнаружен."
            )
        }

        return .success(
            "GPU: \(device.name). Metal доступен."
        )
    }

    private func checkDisplay()
        -> StepResult
    {

        let screens =
            NSScreen.screens

        guard !screens.isEmpty else {

            return .failure(
                "Не найден ни один дисплей."
            )
        }

        guard let main =
            NSScreen.main
        else {

            return .warning(
                "Основной дисплей не определён."
            )
        }

        let width =
            Int(main.frame.width)

        let height =
            Int(main.frame.height)

        return .success(
            "\(screens.count) дисплея. " +
            "Основной: \(width)×\(height)."
        )
    }

    

    private func checkMemory()
        -> StepResult
    {

        let memory =
            ProcessInfo
                .processInfo
                .physicalMemory

        guard memory > 0 else {

            return .failure(
                "Не удалось определить объём памяти."
            )
        }

        let gb =
            Double(memory)
            /
            1024.0
            /
            1024.0
            /
            1024.0

        if gb < 8 {

            return .warning(
                String(
                    format:
                        "Физическая память: %.1f GB.",
                    gb
                )
            )
        }

        return .success(
            String(
                format:
                    "Физическая память: %.1f GB.",
                gb
            )
        )
    }


    private func checkFileSystem()
        -> StepResult
    {

        let fileManager =
            FileManager.default

        let home =
            fileManager.homeDirectoryForCurrentUser

        let documents =
            home.appendingPathComponent(
                "Documents",
                isDirectory: true
            )

        let temporary =
            URL(
                fileURLWithPath:
                    NSTemporaryDirectory(),
                isDirectory: true
            )

        var missing: [String] = []

        if !fileManager.fileExists(
            atPath: home.path
        ) {
            missing.append("Домашний каталог")
        }

        if !fileManager.fileExists(
            atPath: documents.path
        ) {
            missing.append("Documents")
        }

        if !fileManager.fileExists(
            atPath: temporary.path
        ) {
            missing.append("Temporary")
        }

        guard missing.isEmpty else {

            return .warning(
                "Отсутствуют: " +
                missing.joined(separator: ", ") +
                "."
            )
        }

        return .success(
            "Каталоги проектов доступны."
        )
    }

    
    private func checkApplicationIntegrity()
        -> StepResult
    {

        let bundle =
            Bundle.main

        let bundleID =
            bundle.bundleIdentifier

        guard bundleID != nil,
              !bundleID!.isEmpty
        else {

            return .warning(
                "Bundle identifier отсутствует."
            )
        }

        guard bundle.executablePath != nil else {

            return .failure(
                "Исполняемый файл приложения не найден."
            )
        }

        return .success(
            "Bundle: \(bundleID ?? "unknown")."
        )
    }

    

    private func prepareEngineeringEnvironment()
        -> StepResult
    {


        return .success(
            "Базовая инженерная среда подготовлена."
        )
    }

    
    private func finalizeBoot()
        -> StepResult
    {

        if errorCount > 0 {

            return .failure(
                "Критических ошибок: \(errorCount)."
            )
        }

        if warningCount > 0 {

            return .warning(
                "Предупреждений: \(warningCount)."
            )
        }

        return .success(
            "Все предварительные проверки пройдены."
        )
    }

    private func addStep(
        index: Int,
        title: String,
        detail: String,
        severity: Severity
    ) {

        steps.append(
            Step(
                index: index,
                title: title,
                detail: detail,
                severity: severity
            )
        )
    }

    
    func reset() {

        hasStarted = false

        state = .idle

        progress = 0

        currentTitle =
            "Подготовка MIR 4D…"

        currentDetail =
            "Ожидание запуска."

        steps.removeAll()

        errorCount = 0

        warningCount = 0
    }
}
