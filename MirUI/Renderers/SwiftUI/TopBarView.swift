import SwiftUI

struct TopBarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var registry: CADCommandRegistry
    @Binding var commandPalettePresented: Bool
    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        HStack(spacing: 10) {
            brand
            divider
            workspaceMenu
            Spacer(minLength: 8)
            context
            aiInspector
            commandPalette
            moreMenu
        }
        .padding(.horizontal, 14)
        .frame(height: 50)
        .background(MirTheme.Colors.topBar)
        .overlay(alignment: .bottom) { Rectangle().fill(MirTheme.Colors.borderStrong).frame(height: 1) }
        .onAppear {
            registry.registerDefaults(appState: appState)
            registry.registerExtendedScenarioCommands(appState: appState)
        }
    }

    private var divider: some View { Rectangle().fill(MirTheme.Colors.border).frame(width: 1, height: 24) }

    private var brand: some View {
        HStack(spacing: 8) {
            Image(systemName: "sun.max.fill").font(.system(size: 15, weight: .semibold)).foregroundStyle(MirTheme.Colors.keyframe)
            VStack(alignment: .leading, spacing: 1) {
                Text("МИР 4D").font(.system(size: 12, weight: .bold)).foregroundStyle(MirTheme.Colors.textPrimary)
                HStack(spacing: 5) {
                    Circle().fill(appState.documentDirty ? MirTheme.Colors.warning : MirTheme.Colors.success).frame(width: 5, height: 5)
                    Text(appState.documentName).font(.system(size: 9)).foregroundStyle(MirTheme.Colors.textTertiary).lineLimit(1)
                }
            }
        }.frame(minWidth: 145, alignment: .leading)
    }

    private var workspaceMenu: some View {
        Menu {
            Section(russian ? "Моделирование" : "Modeling") {
                workbenchButton(.model); workbenchButton(.sketch); workbenchButton(.assembly); workbenchButton(.drawing)
            }
            Section(russian ? "Инженерия" : "Engineering") { workbenchButton(.simulation); workbenchButton(.fourD) }
            Section(russian ? "Совместная работа" : "Collaboration") { workbenchButton(.collaboration) }
        } label: {
            HStack(spacing: 7) {
                Image(systemName: appState.workbench.icon).font(.system(size: 12, weight: .semibold))
                VStack(alignment: .leading, spacing: 0) {
                    Text(russian ? appState.workbench.titleRU : appState.workbench.titleEN).font(.system(size: 11, weight: .semibold))
                    Text(russian ? "Рабочая область" : "Workspace").font(.system(size: 8)).foregroundStyle(MirTheme.Colors.textTertiary)
                }
                Image(systemName: "chevron.down").font(.system(size: 8, weight: .bold)).foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .foregroundStyle(MirTheme.Colors.textPrimary).padding(.horizontal, 9).frame(height: 34)
            .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: 8))
            .overlay(RoundedRectangle(cornerRadius: 8).stroke(MirTheme.Colors.border, lineWidth: 1))
        }.menuStyle(.borderlessButton)
    }

    private func workbenchButton(_ workbench: CADWorkbench) -> some View {
        Button { withAnimation(.easeOut(duration: 0.18)) { appState.selectWorkbench(workbench) } } label: {
            Label(russian ? workbench.titleRU : workbench.titleEN, systemImage: workbench.icon)
        }
    }

    private var context: some View {
        HStack(spacing: 6) {
            Circle().fill(appState.documentDirty ? MirTheme.Colors.warning : MirTheme.Colors.success).frame(width: 6, height: 6)
            Text(appState.documentDirty ? (russian ? "Есть изменения" : "Unsaved") : (appState.selectionCount > 0 ? (russian ? "Выбрано: \(appState.selectionCount)" : "Selected: \(appState.selectionCount)") : (russian ? "Сохранено" : "Saved")))
                .font(.system(size: 9, weight: .medium)).foregroundStyle(MirTheme.Colors.textTertiary).lineLimit(1)
        }.padding(.horizontal, 9).frame(height: 30).background(MirTheme.Colors.surface, in: Capsule()).overlay(Capsule().stroke(MirTheme.Colors.border, lineWidth: 1))
    }

    private var aiInspector: some View {
        Button {
            appState.togglePanel(.aiInspector)
        } label: {
            HStack(spacing: 6) {
                Image(systemName: "sparkles")
                Text(russian ? "AI" : "AI")
            }
            .font(.system(size: 10, weight: .semibold))
            .foregroundStyle(MirTheme.Colors.accentBright)
            .padding(.horizontal, 10).frame(height: 30)
            .background(MirTheme.Colors.accentSoft, in: RoundedRectangle(cornerRadius: 8))
            .overlay(RoundedRectangle(cornerRadius: 8).stroke(MirTheme.Colors.accentBright.opacity(0.35), lineWidth: 1))
        }
        .buttonStyle(.plain)
        .help(russian ? "Открыть AI Inspector" : "Open AI Inspector")
    }

    private var commandPalette: some View {
        Button { commandPalettePresented = true } label: {
            HStack(spacing: 6) {
                Image(systemName: "magnifyingglass")
                Text(russian ? "Поиск действий" : "Search actions")
                Text("⌘K").font(.system(size: 8, weight: .bold, design: .monospaced)).foregroundStyle(MirTheme.Colors.textTertiary)
            }.font(.system(size: 10, weight: .medium)).foregroundStyle(MirTheme.Colors.textSecondary).padding(.horizontal, 10).frame(height: 30)
                .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: 8)).overlay(RoundedRectangle(cornerRadius: 8).stroke(MirTheme.Colors.border, lineWidth: 1))
        }.buttonStyle(.plain).keyboardShortcut("k", modifiers: [.command])
    }

    private var moreMenu: some View {
        Menu {
            Section(russian ? "Проект" : "Project") {
                commandMenuItem("document.new", russian ? "Новый проект" : "New project", "doc.badge.plus")
                commandMenuItem("document.open", russian ? "Открыть проект" : "Open project", "folder")
                commandMenuItem("document.save", russian ? "Сохранить" : "Save", "square.and.arrow.down")
            }
            Section(russian ? "Вид" : "View") {
                commandMenuItem("viewport.grid", russian ? "Сетка" : "Grid", "grid")
                commandMenuItem("viewport.axes", russian ? "Оси" : "Axes", "axis.3d")
                commandMenuItem("viewport.section", russian ? "Сечение" : "Section", "scissors")
            }
            Divider()
            Button { appState.toggleExperience() } label: {
                Label(russian ? "Режим: \(appState.ui.experience == .expert ? "Эксперт" : "Базовый")" : "Mode: \(appState.ui.experience == .expert ? "Expert" : "Basic")", systemImage: "slider.horizontal.3")
            }
        } label: { Image(systemName: "ellipsis.circle").font(.system(size: 16, weight: .medium)).foregroundStyle(MirTheme.Colors.textSecondary).frame(width: 32, height: 32) }
            .menuStyle(.borderlessButton)
    }

    private func commandMenuItem(_ id: String, _ title: String, _ fallbackIcon: String) -> some View {
        Button { execute(id) } label: { Label(title, systemImage: registry.commands.first(where: { $0.id == id })?.icon ?? fallbackIcon) }
    }

    private func execute(_ id: String) {
        guard let command = registry.commands.first(where: { $0.id == id }) else { MirEventBus.shared.publish(.commandRequested(id)); return }
        guard command.workbenches.contains(appState.workbench), command.isAvailable(appState.activeContext) else { return }
        MirEventBus.shared.publish(.commandRequested(id)); MirEventBus.shared.publish(.commandStarted(id)); command.execute(); MirEventBus.shared.publish(.commandFinished(id))
    }
}
