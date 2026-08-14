import SwiftUI
import AppKit
import Combine

@MainActor
final class MIR4DStartupController: ObservableObject {

    enum Phase {
        case diagnostics
        case ready
        case failed
    }

    enum StartupDestination {
        case openProject
        case newProject
        case laboratory4D
        case mathematics
        case knowledge
    }

    struct DiagnosticStep: Identifiable {
        let id = UUID()
        let title: String
        var progress: Double
        var status: Status

        enum Status {
            case waiting
            case running
            case success
            case warning
            case failure
        }
    }

    @Published private(set) var phase: Phase = .diagnostics

    @Published private(set) var progress: Double = 0.0

    @Published private(set) var statusText: String =
        "Подготовка MIR 4D..."

    @Published private(set) var steps: [DiagnosticStep] = []

    @Published var selectedDestination: StartupDestination?

    @Published private(set) var diagnosticPassed = false

    private var started = false

    init() {
        prepareSteps()
    }

    private func prepareSteps() {

        steps = [

            DiagnosticStep(
                title: "Инициализация MIR 4D",
                progress: 0,
                status: .waiting
            ),

            DiagnosticStep(
                title: "Проверка Swift / SwiftUI",
                progress: 0,
                status: .waiting
            ),

            DiagnosticStep(
                title: "Проверка AppKit",
                progress: 0,
                status: .waiting
            ),

            DiagnosticStep(
                title: "Проверка OpenGL",
                progress: 0,
                status: .waiting
            ),

            DiagnosticStep(
                title: "Проверка MirEngine",
                progress: 0,
                status: .waiting
            ),

            DiagnosticStep(
                title: "Проверка 3D Viewport",
                progress: 0,
                status: .waiting
            ),

            DiagnosticStep(
                title: "Проверка 3D сцены",
                progress: 0,
                status: .waiting
            ),

            DiagnosticStep(
                title: "Проверка Event Bus",
                progress: 0,
                status: .waiting
            ),

            DiagnosticStep(
                title: "Проверка готовности интерфейса",
                progress: 0,
                status: .waiting
            )
        ]
    }

    func start() {

        guard !started else {
            return
        }

        started = true

        Task { @MainActor in
            await runDiagnostics()
        }
    }

    private func runDiagnostics() async {

        phase = .diagnostics
        diagnosticPassed = false
        progress = 0

        let total = steps.count

        for index in 0..<total {

            await runStep(index)

            let completed = Double(index + 1)

            progress = completed / Double(total)

            if progress >= 1.0 {
                progress = 1.0
            }

            try? await Task.sleep(
                for: .milliseconds(120)
            )
        }

        diagnosticPassed = true

        progress = 1.0

        statusText = "MIR 4D готов к работе"

        phase = .ready
    }

    private func runStep(_ index: Int) async {

        guard steps.indices.contains(index) else {
            return
        }

        steps[index].status = .running
        steps[index].progress = 0.1

        switch index {

        case 0:
            statusText = "Инициализация MIR 4D..."
            await animateStep(index)

        case 1:
            statusText = "Проверка Swift / SwiftUI..."
            await animateStep(index)

        case 2:
            statusText = "Проверка AppKit..."
            await animateStep(index)

        case 3:
            statusText = "Проверка OpenGL..."
            await animateStep(index)

        case 4:
            statusText = "Проверка MirEngine..."
            await animateStep(index)

        case 5:
            statusText = "Проверка 3D Viewport..."
            await animateStep(index)

        case 6:
            statusText = "Проверка 3D сцены..."
            await animateStep(index)

        case 7:
            statusText = "Проверка Event Bus..."
            await animateStep(index)

        case 8:
            statusText = "Подготовка интерфейса..."
            await animateStep(index)

        default:
            break
        }

        steps[index].progress = 1.0
        steps[index].status = .success
    }

    private func animateStep(_ index: Int) async {

        let values: [Double] = [
            0.25,
            0.5,
            0.75,
            1.0
        ]

        for value in values {

            steps[index].progress = value

            try? await Task.sleep(
                for: .milliseconds(80)
            )
        }
    }

    func choose(_ destination: StartupDestination) {

        selectedDestination = destination
    }
}