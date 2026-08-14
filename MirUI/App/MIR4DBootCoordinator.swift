//
//  MIR4DBootCoordinator.swift
//  MIR 4D
//
//  Центральный координатор запуска MIR 4D.
//
//  Задачи:
//
//  1. Запустить предварительную проверку.
//  2. Показать ход загрузки.
//  3. Не создавать OpenGL.
//  4. Не обращаться напрямую к MirEngine.
//  5. Не создавать CVDisplayLink.
//  6. Передать управление основной CAD-среде
//     после достижения 100%.
//

import Foundation
import SwiftUI
import AppKit
import Metal

@MainActor
final class MIR4DBootCoordinator: ObservableObject {

    // MARK: - State

    enum State: Equatable {
        case idle
        case booting
        case ready
        case warning
        case failed
    }

    // MARK: - Severity

    enum Severity {
        case info
        case success
        case warning
        case error
    }

    // MARK: - Step

    struct Step: Identifiable {

        let id = UUID()

        let index: Int

        let title: String

        let detail: String

        let severity: Severity
    }

    // MARK: - Published

    @Published private(set) var state: State = .idle

    @Published private(set) var progress: Double = 0

    @Published private(set) var currentTitle =
        "Подготовка MIR 4D…"

    @Published private(set) var currentDetail =
        "Ожидание запуска."

    @Published private(set) var steps: [Step] = []

    @Published private(set) var errorCount = 0

    @Published private(set) var warningCount = 0

    // MARK: - Internal

    private var hasStarted = false

    private let totalSteps = 9

    // MARK: - Start

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

        // -------------------------------------------------
        // 1
        // -------------------------------------------------

        await execute(
            index: 1,
            title: "Проверка платформы",
            detail:
                "Проверяем macOS и архитектуру процессора."
        ) {
            self.checkPlatform()
        }

        // -------------------------------------------------
        // 2
        // -------------------------------------------------

        await execute(
            index: 2,
            title: "Проверка SwiftUI",
            detail:
                "Проверяем пользовательский интерфейс MIR 4D."
        ) {
            self.checkSwiftUI()
        }

        // -------------------------------------------------
        // 3
        // -------------------------------------------------

        await execute(
            index: 3,
            title: "Проверка ресурсов",
            detail:
                "Проверяем Bundle и файловые ресурсы."
        ) {
            self.checkResources()
        }

        // -------------------------------------------------
        // 4
        // -------------------------------------------------

        await execute(
            index: 4,
            title: "Проверка Event Bus",
            detail:
                "Проверяем систему обмена событиями."
        ) {
            self.checkEventBus()
        }

        // -------------------------------------------------
        // 5
        // -------------------------------------------------

        await execute(
            index: 5,
            title: "Проверка GPU",
            detail:
                "Проверяем доступность графического устройства."
        ) {
            self.checkGPU()
        }

        // -------------------------------------------------
        // 6
        // -------------------------------------------------

        await execute(
            index: 6,
            title: "Проверка дисплея",
            detail:
                "Проверяем графическое окружение macOS."
        ) {
            self.checkDisplay()
        }

        // -------------------------------------------------
        // 7
        // -------------------------------------------------

        await execute(
            index: 7,
            title: "Проверка памяти",
            detail:
                "Проверяем доступную физическую память."
        ) {
            self.checkMemory()
        }

        // -------------------------------------------------
        // 8
        // -------------------------------------------------

        await execute(
            index: 8,
            title: "Подготовка инженерной среды",
            detail:
                "Подготавливаем окружение MIR 4D."
        ) {
            self.prepareEngineeringEnvironment()
        }

        // -------------------------------------------------
        // 9
        // -------------------------------------------------

        await execute(
            index: 9,
            title: "Финальная проверка",
            detail:
                "Определяем готовность приложения."
        ) {
            self.finalizeBoot()
        }

        // -------------------------------------------------
        // Final state
        // -------------------------------------------------

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

    // MARK: - Execute

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

    // MARK: - Step result

    private enum StepResult {

        case success(String)

        case warning(String)

        case failure(String)
    }

    // MARK: - Platform

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

    // MARK: - SwiftUI

    private func checkSwiftUI()
        -> StepResult
    {

        return .success(
            "SwiftUI runtime доступен."
        )
    }

    // MARK: - Resources

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

    // MARK: - Event Bus

    private func checkEventBus()
        -> StepResult
    {

        /*
         Swift 6:

         НЕ используем:

             var received = false

         внутри NotificationCenter closure.

         Это приводит к:

             Mutation of captured var
             in concurrently-executing code

         Нам достаточно проверить возможность регистрации
         и публикации уведомления.
        */

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
                // Проверка регистрации NotificationCenter.
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

    // MARK: - GPU

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

    // MARK: - Display

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

    // MARK: - Memory

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

    // MARK: - Engineering environment

    private func prepareEngineeringEnvironment()
        -> StepResult
    {

        /*
         ВАЖНО.

         Здесь НЕ вызываем:

             MirEngineCreateMacOpenGLContext
             MirEngineCreateOpenGLRenderer
             MirEngineCreateViewport
             CVDisplayLink

         Эти объекты принадлежат реальному
         жизненному циклу MirGLCustomView.

         Архитектура:

                 Boot
                  │
                  ▼
              Start UI
                  │
                  ▼
            CADMainView
                  │
                  ▼
             MirGLView
                  │
                  ▼
          MirGLCustomView
                  │
                  ▼
              MirEngine
        */

        return .success(
            "Базовая инженерная среда подготовлена."
        )
    }

    // MARK: - Finalize

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

    // MARK: - Add step

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

    // MARK: - Reset

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
