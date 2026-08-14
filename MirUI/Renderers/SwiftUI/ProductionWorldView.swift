import SwiftUI

@MainActor
final class ProductionWorldStore: ObservableObject {
    enum Stage: String, CaseIterable, Identifiable {
        case idea
        case model
        case assembly
        case test
        case scenario
        case drawing
        case manufacture

        var id: String { rawValue }

        var titleRU: String {
            switch self {
            case .idea: return "Идея"
            case .model: return "Модель"
            case .assembly: return "Сборка"
            case .test: return "Испытание"
            case .scenario: return "Цифровой мир"
            case .drawing: return "Чертёж"
            case .manufacture: return "Производство"
            }
        }

        var titleEN: String {
            switch self {
            case .idea: return "Idea"
            case .model: return "Model"
            case .assembly: return "Assembly"
            case .test: return "Test"
            case .scenario: return "Digital World"
            case .drawing: return "Drawing"
            case .manufacture: return "Manufacture"
            }
        }

        var icon: String {
            switch self {
            case .idea: return "lightbulb"
            case .model: return "cube"
            case .assembly: return "square.stack.3d.up"
            case .test: return "waveform.path.ecg"
            case .scenario: return "gamecontroller"
            case .drawing: return "doc.text"
            case .manufacture: return "hammer"
            }
        }

        var color: Color {
            switch self {
            case .idea: return MirTheme.Colors.warning
            case .model: return MirTheme.Colors.accent
            case .assembly: return MirTheme.Colors.selection
            case .test: return MirTheme.Colors.simulation
            case .scenario: return MirTheme.Colors.time
            case .drawing: return MirTheme.Colors.success
            case .manufacture: return MirTheme.Colors.success
            }
        }
    }

    @Published var activeStage: Stage = .model
    @Published var completion: [Stage: Double] = [
        .idea: 1.0,
        .model: 0.72,
        .assembly: 0.42,
        .test: 0.0,
        .scenario: 0.0,
        .drawing: 0.0,
        .manufacture: 0.0
    ]
    @Published var worldRunning = false
    @Published var testPassed = false
    @Published var productionReady = false
    @Published var currentMachine = "Станок не выбран"
    @Published var testResult = "Испытание не запускалось"
    @Published var drawingStatus = "Чертёж не выпущен"
    @Published var manufacturingStatus = "Производственное задание не создано"

    var overallProgress: Double {
        let values = Stage.allCases.map { completion[$0] ?? 0 }
        return values.reduce(0, +) / Double(max(values.count, 1))
    }

    func activate(_ stage: Stage) {
        activeStage = stage
    }

    func advance(to stage: Stage) {
        activeStage = stage
        switch stage {
        case .idea:
            completion[.idea] = 1
        case .model:
            completion[.idea] = 1
            completion[.model] = max(completion[.model] ?? 0, 0.75)
        case .assembly:
            completion[.model] = 1
            completion[.assembly] = max(completion[.assembly] ?? 0, 0.6)
        case .test:
            completion[.assembly] = 1
            completion[.test] = max(completion[.test] ?? 0, 0.15)
        case .scenario:
            completion[.test] = max(completion[.test] ?? 0, 0.8)
            completion[.scenario] = max(completion[.scenario] ?? 0, 0.2)
        case .drawing:
            completion[.scenario] = max(completion[.scenario] ?? 0, 1)
            completion[.drawing] = max(completion[.drawing] ?? 0, 0.3)
        case .manufacture:
            completion[.drawing] = 1
            completion[.manufacture] = max(completion[.manufacture] ?? 0, 0.2)
        }
    }

    func runDigitalTest() {
        worldRunning = true
        activeStage = .test
        completion[.test] = 0.55
        testResult = "Испытание запущено · виртуальный стенд активен"
    }

    func finishDigitalTest() {
        worldRunning = false
        testPassed = true
        completion[.test] = 1
        testResult = "Испытание пройдено · ограничений не обнаружено"
        activeStage = .scenario
        completion[.scenario] = 0.55
    }

    func releaseDrawing() {
        drawingStatus = "КД выпущена · ревизия A"
        completion[.drawing] = 1
        activeStage = .drawing
    }

    func createManufacturingOrder() {
        productionReady = true
        manufacturingStatus = "Заказ МО-2026-001 создан · очередь производства"
        completion[.manufacture] = 1
        activeStage = .manufacture
    }
}

struct ProductionWorldView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var store: ProductionWorldStore

    var body: some View {
        VStack(spacing: 8) {
            ribbon
            controlCard
        }
        .padding(.horizontal, 12)
        .padding(.top, 10)
    }

    private var ribbon: some View {
        HStack(spacing: 4) {
            ForEach(ProductionWorldStore.Stage.allCases) { stage in
                Button {
                    select(stage)
                } label: {
                    VStack(spacing: 4) {
                        Image(systemName: stage.icon)
                            .font(.system(size: 13, weight: .semibold))
                        Text(appState.ui.language == .russian ? stage.titleRU : stage.titleEN)
                            .font(.system(size: 10, weight: .medium))
                            .lineLimit(1)
                        ProgressView(value: store.completion[stage] ?? 0)
                            .progressViewStyle(.linear)
                            .tint(stage.color)
                    }
                    .frame(maxWidth: .infinity, minHeight: 58)
                    .foregroundStyle(stage == store.activeStage ? stage.color : MirTheme.Colors.textSecondary)
                    .background(stage == store.activeStage ? stage.color.opacity(0.12) : Color.clear)
                    .clipShape(RoundedRectangle(cornerRadius: 8))
                }
                .buttonStyle(.plain)
            }
        }
        .padding(6)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 10))
        .overlay {
            RoundedRectangle(cornerRadius: 10)
                .stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 1)
        }
    }

    private var controlCard: some View {
        HStack(spacing: 12) {
            VStack(alignment: .leading, spacing: 3) {
                Text(appState.ui.language == .russian ? "Цифровой производственный мир" : "Digital Production World")
                    .font(.system(size: 13, weight: .semibold))
                Text(stageDescription)
                    .font(.system(size: 11))
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                    .lineLimit(2)
            }

            Spacer()

            ProgressView(value: store.overallProgress)
                .frame(width: 120)
                .tint(MirTheme.Colors.success)

            Text("\(Int(store.overallProgress * 100))%")
                .font(.system(size: 11, weight: .bold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.success)

            actionButton
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 9)
        .background(.thinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 10))
    }

    private var actionButton: some View {
        Button {
            executeStageAction()
        } label: {
            Label(actionTitle, systemImage: actionIcon)
        }
        .buttonStyle(.borderedProminent)
        .controlSize(.small)
        .tint(store.activeStage.color)
    }

    private var actionTitle: String {
        switch store.activeStage {
        case .idea: return appState.ui.language == .russian ? "Начать моделирование" : "Start Modeling"
        case .model: return appState.ui.language == .russian ? "Собрать" : "Assemble"
        case .assembly: return appState.ui.language == .russian ? "Запустить испытание" : "Run Test"
        case .test: return store.worldRunning ? (appState.ui.language == .russian ? "Завершить тест" : "Finish Test") : (appState.ui.language == .russian ? "Запустить стенд" : "Run Stand"
        )
        case .scenario: return appState.ui.language == .russian ? "Открыть цифровой мир" : "Open Digital World"
        case .drawing: return appState.ui.language == .russian ? "Выпустить КД" : "Release Drawing"
        case .manufacture: return appState.ui.language == .russian ? "Создать заказ" : "Create Order"
        }
    }

    private var actionIcon: String {
        switch store.activeStage {
        case .idea: return "arrow.right"
        case .model: return "square.stack.3d.up"
        case .assembly: return "play.fill"
        case .test: return store.worldRunning ? "stop.fill" : "play.fill"
        case .scenario: return "gamecontroller"
        case .drawing: return "checkmark.seal"
        case .manufacture: return "paperplane"
        }
    }

    private var stageDescription: String {
        switch store.activeStage {
        case .idea: return appState.ui.language == .russian ? "Фиксируем замысел и превращаем его в инженерный объект." : "Capture the idea and turn it into an engineering object."
        case .model: return appState.ui.language == .russian ? "Создаём геометрию, параметры и конструктивные особенности." : "Create geometry, parameters and engineering features."
        case .assembly: return appState.ui.language == .russian ? "Собираем оборудование и задаём связи между узлами." : "Assemble equipment and define relationships."
        case .test: return store.testResult
        case .scenario: return appState.ui.language == .russian ? "Запускаем оборудование в игровом цифровом мире и проверяем сценарии работы." : "Run the equipment in a game-like digital world and test operating scenarios."
        case .drawing: return store.drawingStatus
        case .manufacture: return store.manufacturingStatus
        }
    }

    private func select(_ stage: ProductionWorldStore.Stage) {
        store.activate(stage)
        switch stage {
        case .model:
            appState.selectWorkbench(.model)
        case .assembly:
            appState.selectWorkbench(.assembly)
        case .test:
            appState.selectWorkbench(.simulation)
        case .scenario:
            appState.selectWorkbench(.fourD)
        case .drawing:
            appState.selectWorkbench(.drawing)
        case .manufacture:
            appState.selectWorkbench(.collaboration)
        case .idea:
            appState.selectWorkbench(.model)
        }
    }

    private func executeStageAction() {
        switch store.activeStage {
        case .idea:
            store.advance(to: .model)
            appState.selectWorkbench(.model)
        case .model:
            store.advance(to: .assembly)
            appState.selectWorkbench(.assembly)
        case .assembly:
            store.runDigitalTest()
            appState.selectWorkbench(.simulation)
        case .test:
            if store.worldRunning {
                store.finishDigitalTest()
                appState.selectWorkbench(.fourD)
            } else {
                store.runDigitalTest()
                appState.selectWorkbench(.simulation)
            }
        case .scenario:
            store.completion[.scenario] = 1
            appState.selectWorkbench(.fourD)
            store.activate(.drawing)
        case .drawing:
            store.releaseDrawing()
            appState.selectWorkbench(.drawing)
            store.activate(.manufacture)
        case .manufacture:
            store.createManufacturingOrder()
            appState.selectWorkbench(.collaboration)
        }
    }
}
