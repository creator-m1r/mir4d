import SwiftUI

/// Central presentation router for MIR 4D workbenches.
///
/// The router owns no engineering state. It only translates the current
/// ActiveContext into a small presentation layer: title, mode description,
/// contextual hints and optional workbench-specific controls.
struct WorkbenchContentRouter: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry

    var body: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.sm) {
            header
            contextualContent
        }
        .padding(MirTheme.Spacing.md)
        .frame(maxWidth: 430, alignment: .leading)
        .mirFloating()
        .padding(MirTheme.Spacing.md)
    }

    private var header: some View {
        HStack(spacing: MirTheme.Spacing.sm) {
            Image(systemName: appState.workbench.icon)
                .foregroundStyle(accent)

            VStack(alignment: .leading, spacing: 2) {
                Text(localizedWorkbench)
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)

                Text(localizedSubMode)
                    .font(MirTheme.Typography.caption)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }

            Spacer()

            Text(contextSummary)
                .font(MirTheme.Typography.numeric)
                .foregroundStyle(accent)
        }
    }

    @ViewBuilder
    private var contextualContent: some View {
        switch appState.workbench {
        case .model:
            modelContent
        case .sketch:
            sketchContent
        case .assembly:
            assemblyContent
        case .simulation:
            simulationContent
        case .fourD:
            fourDContent
        case .drawing:
            drawingContent
        case .collaboration:
            collaborationContent
        case .visualization:
            visualizationContent
        }
    }

    private var modelContent: some View {
        hint(
            ru: "Модель: создавайте и изменяйте BRep через команды MirEngine.",
            en: "Model: create and edit BRep through MirEngine commands."
        )
    }

    private var sketchContent: some View {
        HStack(spacing: MirTheme.Spacing.sm) {
            contextBadge("SNAP", enabled: appState.interaction.snapEnabled)
            contextBadge("INF", enabled: appState.interaction.inferenceEnabled)
            contextBadge("GEO", enabled: appState.selection.hasSelection)
        }
    }

    private var assemblyContent: some View {
        hint(
            ru: "Сборка: структура, связи и кинематические сценарии.",
            en: "Assembly: structure, mates and kinematic scenarios."
        )
    }

    private var simulationContent: some View {
        HStack(spacing: MirTheme.Spacing.sm) {
            Text(localizedPhysics)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.simulation)

            Text("·")
                .foregroundStyle(MirTheme.Colors.textTertiary)

            Text(localizedPhase)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)

            Spacer()

            Text(String(format: "%.0f%%", appState.simulation.progress * 100))
                .font(MirTheme.Typography.numeric)
                .foregroundStyle(MirTheme.Colors.simulation)
        }
    }

    private var fourDContent: some View {
        HStack(spacing: MirTheme.Spacing.sm) {
            contextBadge("T", enabled: appState.isPlaying)
            Text(appState.timeState.scenarioID)
                .font(.system(size: 9, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .lineLimit(1)

            Spacer()

            Button {
                appState.createTimeScenario()
            } label: {
                Image(systemName: "plus.circle")
            }
            .buttonStyle(.plain)
            .help(appState.ui.language == .russian ? "Новый сценарий" : "New scenario")

            Button {
                appState.createTimeBranch()
            } label: {
                Image(systemName: "arrow.triangle.branch")
            }
            .buttonStyle(.plain)
            .help(appState.ui.language == .russian ? "Новая ветка" : "New branch")
        }
        .foregroundStyle(MirTheme.Colors.time)
    }

    private var drawingContent: some View {
        hint(
            ru: "Чертёж: виды, размеры, сечения и спецификация.",
            en: "Drawing: views, dimensions, sections and BOM."
        )
    }

    private var collaborationContent: some View {
        hint(
            ru: "Сотрудничество: ревью, комментарии и версии документа.",
            en: "Collaboration: review, comments and document versions."
        )
    }

    private var visualizationContent: some View {
        hint(
            ru: "Визуализация: материалы, камера, освещение и рендер.",
            en: "Visualization: materials, camera, lighting and rendering."
        )
    }

    private func hint(ru: String, en: String) -> some View {
        Text(appState.ui.language == .russian ? ru : en)
            .font(MirTheme.Typography.caption)
            .foregroundStyle(MirTheme.Colors.textTertiary)
            .lineLimit(2)
    }

    private func contextBadge(_ text: String, enabled: Bool) -> some View {
        Text(text)
            .font(.system(size: 8, weight: .bold, design: .monospaced))
            .foregroundStyle(enabled ? MirTheme.Colors.success : MirTheme.Colors.textDisabled)
            .padding(.horizontal, 6)
            .padding(.vertical, 3)
            .background((enabled ? MirTheme.Colors.success : MirTheme.Colors.textDisabled).opacity(0.10))
            .clipShape(Capsule())
    }

    private var localizedWorkbench: String {
        appState.ui.language == .russian ? appState.workbench.titleRU : appState.workbench.titleEN
    }

    private var localizedSubMode: String {
        appState.ui.language == .russian ? appState.subMode.titleRU : appState.subMode.titleEN
    }

    private var localizedPhysics: String {
        appState.ui.language == .russian ? appState.simulation.physics.titleRU : appState.simulation.physics.titleEN
    }

    private var localizedPhase: String {
        switch appState.simulation.phase {
        case .setup: return appState.ui.language == .russian ? "Настройка" : "Setup"
        case .solve: return appState.ui.language == .russian ? "Расчёт" : "Solve"
        case .results: return appState.ui.language == .russian ? "Результаты" : "Results"
        case .compare: return appState.ui.language == .russian ? "Сравнение" : "Compare"
        }
    }

    private var contextSummary: String {
        if appState.selection.count > 0 {
            return "S:\(appState.selection.count)"
        }
        return appState.workbench == .fourD
            ? String(format: "%.2fs", appState.currentTime)
            : "—"
    }

    private var accent: Color {
        switch appState.workbench {
        case .sketch: return MirTheme.Colors.sketch
        case .simulation: return MirTheme.Colors.simulation
        case .fourD: return MirTheme.Colors.time
        default: return MirTheme.Colors.accentBright
        }
    }
}
