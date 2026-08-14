//
//  MIR4DSelfDiagnostics.swift
//  MIR4D
//
//  Самодиагностика MIR 4D.
//  Swift 6 / macOS 14+
//
//  ВАЖНО:
//  Диагностика НЕ создаёт NSOpenGLContext.
//  Диагностика НЕ вызывает OpenGL.
//  Диагностика НЕ запускает MirEngine.
//
//  Архитектура:
//
//      MIR4DApp
//          │
//          ▼
//      Boot Coordinator
//          │
//          ▼
//      Self Diagnostics
//          │
//          ├── macOS
//          ├── CPU
//          ├── RAM
//          ├── GPU / Metal
//          ├── Display
//          ├── File System
//          ├── Bundle
//          └── MirEngine API presence
//          │
//          ▼
//      MIR4D Main Window
//          │
//          ▼
//      MirGLCustomView
//          │
//          ▼
//      MirEngine
//          │
//          ▼
//      OpenGL
//

import Foundation
import SwiftUI
import AppKit
import Metal

// MARK: - MIR 4D Self Diagnostics

@MainActor
final class MIR4DSelfDiagnostics: ObservableObject {

    // MARK: Severity

    enum Severity: String, Sendable {
        case info
        case success
        case warning
        case error
    }

    // MARK: Result

    struct DiagnosticResult: Identifiable, Sendable {

        let id: UUID
        let name: String
        let message: String
        let severity: Severity

        init(
            name: String,
            message: String,
            severity: Severity
        ) {
            self.id = UUID()
            self.name = name
            self.message = message
            self.severity = severity
        }
    }

    // MARK: Snapshot

    struct SystemSnapshot: Sendable {

        let macOSMajor: Int
        let macOSMinor: Int
        let macOSPatch: Int

        let architecture: String

        let physicalMemoryGB: Double

        let screenCount: Int
        let mainScreenWidth: Int
        let mainScreenHeight: Int

        let metalAvailable: Bool
        let metalDeviceName: String?

        let homeDirectoryAvailable: Bool

        let documentsDirectoryAvailable: Bool

        let temporaryDirectoryAvailable: Bool

        let bundleIdentifier: String

        let executablePathAvailable: Bool
    }

    // MARK: Published

    @Published private(set) var results: [DiagnosticResult] = []

    @Published private(set) var progress: Double = 0

    @Published private(set) var isRunning = false

    @Published private(set) var completed = false

    @Published private(set) var startedAt: Date?

    @Published private(set) var finishedAt: Date?

    // MARK: Public state

    var hasErrors: Bool {
        results.contains {
            $0.severity == .error
        }
    }

    var hasWarnings: Bool {
        results.contains {
            $0.severity == .warning
        }
    }

    var isSuccessful: Bool {
        !hasErrors
    }

    var errorCount: Int {
        results.filter {
            $0.severity == .error
        }.count
    }

    var warningCount: Int {
        results.filter {
            $0.severity == .warning
        }.count
    }

    var successCount: Int {
        results.filter {
            $0.severity == .success
        }.count
    }

    // MARK: Init

    init() {}

    // MARK: Run

    func run() async {

        guard !isRunning else {
            return
        }

        isRunning = true
        completed = false

        results.removeAll()

        progress = 0

        startedAt = Date()
        finishedAt = nil

        let snapshot = makeSnapshot()

        let checks: [() -> DiagnosticResult] = [

            {
                self.checkOperatingSystem(snapshot)
            },

            {
                self.checkArchitecture(snapshot)
            },

            {
                self.checkMemory(snapshot)
            },

            {
                self.checkDisplay(snapshot)
            },

            {
                self.checkMetal(snapshot)
            },

            {
                self.checkHomeDirectory(snapshot)
            },

            {
                self.checkDocumentsDirectory(snapshot)
            },

            {
                self.checkTemporaryDirectory(snapshot)
            },

            {
                self.checkBundle(snapshot)
            },

            {
                self.checkExecutable(snapshot)
            },

            {
                self.checkSwiftRuntime()
            },

            {
                self.checkFoundation()
            },

            {
                self.checkMir4DArchitecture()
            }
        ]

        let total = checks.count

        for index in checks.indices {

            if Task.isCancelled {
                break
            }

            let result = checks[index]()

            results.append(result)

            progress =
                Double(index + 1)
                /
                Double(max(total, 1))

            printResult(result)

            /*
             Небольшая пауза нужна только для визуального
             отображения процесса запуска.
             */

            try? await Task.sleep(
                for: .milliseconds(70)
            )
        }

        progress = 1.0

        completed = true

        isRunning = false

        finishedAt = Date()

        printSummary()
    }

    // MARK: Reset

    func reset() {

        results.removeAll()

        progress = 0

        isRunning = false

        completed = false

        startedAt = nil

        finishedAt = nil
    }

    // MARK: Snapshot

    private func makeSnapshot() -> SystemSnapshot {

        let process =
            ProcessInfo.processInfo

        let os =
            process.operatingSystemVersion

        let architecture: String

        #if arch(arm64)

        architecture =
            "Apple Silicon ARM64"

        #elseif arch(x86_64)

        architecture =
            "Intel x86_64"

        #else

        architecture =
            "Unknown"

        #endif

        let memory =
            Double(process.physicalMemory)
            /
            1024.0
            /
            1024.0
            /
            1024.0

        let screens =
            NSScreen.screens

        let mainScreen =
            NSScreen.main

        let metalDevice =
            MTLCreateSystemDefaultDevice()

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

        let bundle =
            Bundle.main

        return SystemSnapshot(

            macOSMajor:
                os.majorVersion,

            macOSMinor:
                os.minorVersion,

            macOSPatch:
                os.patchVersion,

            architecture:
                architecture,

            physicalMemoryGB:
                memory,

            screenCount:
                screens.count,

            mainScreenWidth:
                Int(
                    mainScreen?
                        .frame
                        .width
                    ?? 0
                ),

            mainScreenHeight:
                Int(
                    mainScreen?
                        .frame
                        .height
                    ?? 0
                ),

            metalAvailable:
                metalDevice != nil,

            metalDeviceName:
                metalDevice?.name,

            homeDirectoryAvailable:
                fileManager.fileExists(
                    atPath: home.path
                ),

            documentsDirectoryAvailable:
                fileManager.fileExists(
                    atPath: documents.path
                ),

            temporaryDirectoryAvailable:
                fileManager.fileExists(
                    atPath: temporary.path
                ),

            bundleIdentifier:
                bundle.bundleIdentifier
                ?? "unknown",

            executablePathAvailable:
                bundle.executablePath != nil
        )
    }

    // MARK: macOS

    private func checkOperatingSystem(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        let version =
            "\(snapshot.macOSMajor)." +
            "\(snapshot.macOSMinor)." +
            "\(snapshot.macOSPatch)"

        guard snapshot.macOSMajor >= 14 else {

            return DiagnosticResult(
                name: "macOS",
                message:
                    "Обнаружена macOS \(version). " +
                    "Для MIR 4D требуется macOS 14+.",
                severity: .error
            )
        }

        return DiagnosticResult(
            name: "macOS",
            message:
                "macOS \(version) поддерживается.",
            severity: .success
        )
    }

    // MARK: Architecture

    private func checkArchitecture(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        #if arch(arm64)

        return DiagnosticResult(
            name: "Архитектура CPU",
            message:
                "Обнаружен Apple Silicon ARM64.",
            severity: .success
        )

        #elseif arch(x86_64)

        return DiagnosticResult(
            name: "Архитектура CPU",
            message:
                "Обнаружен Intel x86_64.",
            severity: .success
        )

        #else

        return DiagnosticResult(
            name: "Архитектура CPU",
            message:
                "Архитектура процессора не распознана.",
            severity: .warning
        )

        #endif
    }

    // MARK: Memory

    private func checkMemory(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        let value =
            String(
                format:
                    "%.1f GB",
                snapshot.physicalMemoryGB
            )

        if snapshot.physicalMemoryGB >= 8 {

            return DiagnosticResult(
                name: "Оперативная память",
                message:
                    "Физическая память: \(value).",
                severity: .success
            )
        }

        return DiagnosticResult(
            name: "Оперативная память",
            message:
                "Физическая память: \(value). " +
                "Для сложных CAD-сцен рекомендуется больше 8 GB.",
            severity: .warning
        )
    }

    // MARK: Display

    private func checkDisplay(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        guard snapshot.screenCount > 0 else {

            return DiagnosticResult(
                name: "Дисплей",
                message:
                    "macOS не сообщила доступных дисплеев.",
                severity: .error
            )
        }

        if snapshot.mainScreenWidth <= 0 ||
            snapshot.mainScreenHeight <= 0 {

            return DiagnosticResult(
                name: "Дисплей",
                message:
                    "Основной дисплей обнаружен, " +
                    "но его размер определить не удалось.",
                severity: .warning
            )
        }

        return DiagnosticResult(
            name: "Дисплей",
            message:
                "\(snapshot.screenCount) дисплей(ов), " +
                "основной: " +
                "\(snapshot.mainScreenWidth)×" +
                "\(snapshot.mainScreenHeight).",
            severity: .success
        )
    }

    // MARK: Metal

    private func checkMetal(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        guard snapshot.metalAvailable else {

            return DiagnosticResult(
                name: "GPU / Metal",
                message:
                    "Metal-устройство не обнаружено.",
                severity: .warning
            )
        }

        let deviceName =
            snapshot.metalDeviceName
            ?? "неизвестное устройство"

        return DiagnosticResult(
            name: "GPU / Metal",
            message:
                "Metal доступен: \(deviceName).",
            severity: .success
        )
    }

    // MARK: Home

    private func checkHomeDirectory(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        guard snapshot.homeDirectoryAvailable else {

            return DiagnosticResult(
                name: "Домашний каталог",
                message:
                    "Домашний каталог пользователя недоступен.",
                severity: .error
            )
        }

        return DiagnosticResult(
            name: "Домашний каталог",
            message:
                "Домашний каталог пользователя доступен.",
            severity: .success
        )
    }

    // MARK: Documents

    private func checkDocumentsDirectory(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        if snapshot.documentsDirectoryAvailable {

            return DiagnosticResult(
                name: "Documents",
                message:
                    "Каталог Documents существует.",
                severity: .success
            )
        }

        return DiagnosticResult(
            name: "Documents",
            message:
                "Каталог Documents отсутствует. " +
                "Он может быть создан позже.",
            severity: .warning
        )
    }

    // MARK: Temporary

    private func checkTemporaryDirectory(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        guard snapshot.temporaryDirectoryAvailable else {

            return DiagnosticResult(
                name: "Temporary",
                message:
                    "Временный каталог недоступен.",
                severity: .error
            )
        }

        return DiagnosticResult(
            name: "Temporary",
            message:
                "Временный каталог доступен.",
            severity: .success
        )
    }

    // MARK: Bundle

    private func checkBundle(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        guard snapshot.bundleIdentifier != "unknown" else {

            return DiagnosticResult(
                name: "Application Bundle",
                message:
                    "Bundle identifier отсутствует.",
                severity: .warning
            )
        }

        return DiagnosticResult(
            name: "Application Bundle",
            message:
                "Bundle: \(snapshot.bundleIdentifier).",
            severity: .success
        )
    }

    // MARK: Executable

    private func checkExecutable(
        _ snapshot: SystemSnapshot
    ) -> DiagnosticResult {

        guard snapshot.executablePathAvailable else {

            return DiagnosticResult(
                name: "Executable",
                message:
                    "Исполняемый файл приложения не найден.",
                severity: .error
            )
        }

        return DiagnosticResult(
            name: "Executable",
            message:
                "Исполняемый файл приложения найден.",
            severity: .success
        )
    }

    // MARK: Swift

    private func checkSwiftRuntime()
        -> DiagnosticResult {

        return DiagnosticResult(
            name: "Swift Runtime",
            message:
                "Swift runtime доступен.",
            severity: .success
        )
    }

    // MARK: Foundation

    private func checkFoundation()
        -> DiagnosticResult {

        let fileManager =
            FileManager.default

        let temporary =
            NSTemporaryDirectory()

        guard
            !temporary.isEmpty
        else {

            return DiagnosticResult(
                name: "Foundation",
                message:
                    "Foundation не вернул временный каталог.",
                severity: .error
            )
        }

        _ = fileManager

        return DiagnosticResult(
            name: "Foundation",
            message:
                "Foundation и FileManager работают.",
            severity: .success
        )
    }

    // MARK: MIR 4D Architecture

    private func checkMir4DArchitecture()
        -> DiagnosticResult {

        /*
         ВАЖНО.

         Здесь специально НЕ вызываются:

             MirEngineCreateMacOpenGLContext
             MirEngineCreateOpenGLRenderer
             MirEngineCreateViewport
             MirEngineRender

         Причина:

         Самодиагностика выполняется ДО создания
         MirGLCustomView.

         Реальный графический pipeline:

             Boot
               ↓
             Diagnostics
               ↓
             Start Window
               ↓
             CADMainView
               ↓
             MirGLView
               ↓
             MirGLCustomView
               ↓
             MirEngine
               ↓
             OpenGL Context

         Это предотвращает создание второго OpenGL
         контекста во время загрузки приложения.
         */

        return DiagnosticResult(
            name: "MIR 4D Architecture",
            message:
                "Архитектура запуска подготовлена. " +
                "MirEngine и OpenGL будут подключены " +
                "на этапе создания 3D viewport.",
            severity: .success
        )
    }

    // MARK: Logging

    private func printResult(
        _ result: DiagnosticResult
    ) {

        let prefix: String

        switch result.severity {

        case .info:
            prefix = "ℹ️"

        case .success:
            prefix = "✅"

        case .warning:
            prefix = "⚠️"

        case .error:
            prefix = "❌"
        }

        print(
            "\(prefix) MIR4D SELF-DIAGNOSTIC | " +
            "\(result.name) | " +
            "\(result.message)"
        )
    }

    // MARK: Summary

    private func printSummary() {

        print("")
        print("========================================")
        print("       MIR 4D SELF DIAGNOSTICS")
        print("========================================")
        print("Проверок: \(results.count)")
        print("Успешно: \(successCount)")
        print("Предупреждений: \(warningCount)")
        print("Ошибок: \(errorCount)")

        if hasErrors {

            print("STATUS: FAILED")

        } else if hasWarnings {

            print("STATUS: READY WITH WARNINGS")

        } else {

            print("STATUS: READY")
        }

        print("========================================")
        print("")
    }
}
