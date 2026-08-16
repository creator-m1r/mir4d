//
//  MIR4DStartMenuView.swift
//  MIR 4D
//
//  Project Hub — simple command centre for MIR 4D.
//  The start screen deliberately exposes only the actions a user needs first:
//  continue, create, open. Advanced workspaces remain available below.
//

import SwiftUI
import AppKit

struct MIR4DStartMenuView: View {
    @EnvironmentObject private var appState: CADAppState
    @ObservedObject var diagnostic: MIR4DBootCoordinator

    @State private var showOpenProject = false
    @State private var showNewProject = false
    @State private var autoOpenLastProject = MIR4DProjectSession.shared.isAutoOpenLastProjectEnabled
    @State private var recentRefreshToken = UUID()

    private var commands: MIR4DProjectCommands { .shared }

    private var recentProjects: [MIR4DRecentProject] {
        _ = recentRefreshToken
        return MIR4DProjectSession.shared.recentProjectsList()
    }

    private var continueProject: MIR4DRecentProject? { recentProjects.first }

    private var canContinue: Bool {
        guard let project = continueProject else { return false }
        return MIR4DProjectSession.shared.availability(of: project) == .available
    }

    var body: some View {
        VStack(spacing: 0) {
            topBar
            Divider().opacity(0.16)
            content
        }
        .background(Color(red: 0.018, green: 0.026, blue: 0.042))
        .sheet(isPresented: $showOpenProject) {
            MIR4DProjectOpenView().environmentObject(appState)
        }
        .sheet(isPresented: $showNewProject) {
            MIR4DNewProjectView().environmentObject(appState)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRequestNewProject)) { _ in
            presentNewProject()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DOpenProject)) { _ in
            presentOpenProject()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in
            showNewProject = false
            showOpenProject = false
            refreshRecents()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectSaved)) { _ in
            refreshRecents()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectClosed)) { _ in
            refreshRecents()
        }
    }

    // MARK: - Top navigation

    private var topBar: some View {
        HStack(spacing: 18) {
            HStack(spacing: 11) {
                ZStack {
                    RoundedRectangle(cornerRadius: 9)
                        .fill(Color.white.opacity(0.08))
                    Text("M4D")
                        .font(.system(size: 12, weight: .bold, design: .monospaced))
                        .foregroundStyle(.white)
                }
                .frame(width: 40, height: 40)

                VStack(alignment: .leading, spacing: 2) {
                    Text("МИР 4D")
                        .font(.system(size: 16, weight: .bold, design: .rounded))
                    Text("Инженерная среда")
                        .font(.system(size: 10))
                        .foregroundStyle(.secondary)
                }
            }

            Spacer()

            HStack(spacing: 8) {
                systemStatus
                Divider().frame(height: 20).opacity(0.25)
                Button("Открыть") { presentOpenProject() }
                    .buttonStyle(MIR4DTopBarButtonStyle(prominent: false))
                Button("Новый проект") { presentNewProject() }
                    .buttonStyle(MIR4DTopBarButtonStyle(prominent: true))
            }
        }
        .padding(.horizontal, 24)
        .padding(.vertical, 14)
        .foregroundStyle(.white)
    }

    private var systemStatus: some View {
        HStack(spacing: 7) {
            Circle()
                .fill(.green)
                .frame(width: 6, height: 6)
            Text("Готово")
                .font(.system(size: 10, weight: .medium))
                .foregroundStyle(.secondary)
        }
        .help("Самодиагностика завершена: \(Int(diagnostic.progress * 100))%")
    }

    // MARK: - Main content

    private var content: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 26) {
                welcome
                primaryArea
                recentSection
                workspaceSection
            }
            .frame(maxWidth: 1120)
            .frame(maxWidth: .infinity)
            .padding(.horizontal, 34)
            .padding(.vertical, 30)
        }
    }

    private var welcome: some View {
        VStack(alignment: .leading, spacing: 7) {
            Text("Что будем делать?")
                .font(.system(size: 30, weight: .semibold, design: .rounded))
                .foregroundStyle(.white)
            Text("Начните с проекта. Остальные инструменты появятся внутри рабочей среды, когда они понадобятся.")
                .font(.system(size: 13))
                .foregroundStyle(.secondary)
        }
    }

    private var primaryArea: some View {
        HStack(spacing: 14) {
            continueCard
            createCard
        }
    }

    private var continueCard: some View {
        Button {
            guard canContinue else { return }
            _ = commands.restoreLastProject(appState: appState)
        } label: {
            HStack(spacing: 18) {
                Image(systemName: "arrow.forward.circle.fill")
                    .font(.system(size: 30))
                    .foregroundStyle(canContinue ? .white : .secondary)

                VStack(alignment: .leading, spacing: 5) {
                    Text("Продолжить")
                        .font(.system(size: 17, weight: .semibold))
                    if let project = continueProject {
                        Text(project.name)
                            .font(.system(size: 12, weight: .medium))
                            .foregroundStyle(.white.opacity(0.82))
                            .lineLimit(1)
                    } else {
                        Text("Последний проект не найден")
                            .font(.system(size: 12))
                            .foregroundStyle(.secondary)
                    }
                    Text(canContinue ? "Вернуться к последней работе" : "Создайте или откройте проект")
                        .font(.system(size: 10))
                        .foregroundStyle(.secondary)
                }

                Spacer()
                Image(systemName: "arrow.right")
                    .font(.system(size: 11, weight: .bold))
                    .foregroundStyle(.secondary)
            }
            .foregroundStyle(.white)
            .padding(.horizontal, 20)
            .frame(maxWidth: .infinity, minHeight: 112)
            .background(
                RoundedRectangle(cornerRadius: 15)
                    .fill(Color.white.opacity(canContinue ? 0.075 : 0.035))
            )
            .overlay(
                RoundedRectangle(cornerRadius: 15)
                    .stroke(Color.white.opacity(canContinue ? 0.14 : 0.06), lineWidth: 1)
            )
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
        .disabled(!canContinue)
    }

    private var createCard: some View {
        Button { presentNewProject() } label: {
            HStack(spacing: 18) {
                Image(systemName: "plus.circle.fill")
                    .font(.system(size: 30))
                    .foregroundStyle(.white)

                VStack(alignment: .leading, spacing: 5) {
                    Text("Новый проект")
                        .font(.system(size: 17, weight: .semibold))
                    Text("Создать инженерный проект MIR 4D")
                        .font(.system(size: 12))
                        .foregroundStyle(.white.opacity(0.82))
                    Text("CAD • BIM • 4D")
                        .font(.system(size: 10, weight: .medium, design: .monospaced))
                        .foregroundStyle(.secondary)
                }

                Spacer()
                Image(systemName: "arrow.right")
                    .font(.system(size: 11, weight: .bold))
                    .foregroundStyle(.secondary)
            }
            .foregroundStyle(.white)
            .padding(.horizontal, 20)
            .frame(maxWidth: .infinity, minHeight: 112)
            .background(
                RoundedRectangle(cornerRadius: 15)
                    .fill(Color.white.opacity(0.075))
            )
            .overlay(
                RoundedRectangle(cornerRadius: 15)
                    .stroke(Color.white.opacity(0.14), lineWidth: 1)
            )
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
    }

    // MARK: - Recent projects

    private var recentSection: some View {
        VStack(alignment: .leading, spacing: 12) {
            HStack(alignment: .firstTextBaseline) {
                VStack(alignment: .leading, spacing: 3) {
                    Text("Ваши проекты")
                        .font(.system(size: 17, weight: .semibold))
                        .foregroundStyle(.white)
                    Text("Откройте проект одним нажатием")
                        .font(.system(size: 11))
                        .foregroundStyle(.secondary)
                }
                Spacer()
                Button("Все проекты") { presentOpenProject() }
                    .buttonStyle(MIR4DQuietButtonStyle())
            }

            if recentProjects.isEmpty {
                emptyRecentState
            } else {
                VStack(spacing: 1) {
                    ForEach(Array(recentProjects.prefix(5))) { project in
                        recentRow(project)
                    }
                }
                .background(RoundedRectangle(cornerRadius: 13).fill(Color.white.opacity(0.032)))
                .overlay(RoundedRectangle(cornerRadius: 13).stroke(Color.white.opacity(0.065), lineWidth: 1))
            }

            Toggle(
                "Автоматически открывать последний проект",
                isOn: Binding(
                    get: { autoOpenLastProject },
                    set: { value in
                        autoOpenLastProject = value
                        MIR4DProjectSession.shared.isAutoOpenLastProjectEnabled = value
                    }
                )
            )
            .toggleStyle(.switch)
            .font(.system(size: 10))
            .foregroundStyle(.secondary)
        }
    }

    private var emptyRecentState: some View {
        HStack(spacing: 13) {
            Image(systemName: "folder.badge.plus")
                .font(.system(size: 21))
                .foregroundStyle(.secondary)
            VStack(alignment: .leading, spacing: 3) {
                Text("Здесь появятся ваши проекты")
                    .font(.system(size: 12, weight: .medium))
                    .foregroundStyle(.white)
                Text("Создайте новый проект или откройте существующий .mir4d.")
                    .font(.system(size: 11))
                    .foregroundStyle(.secondary)
            }
            Spacer()
        }
        .padding(17)
    }

    private func recentRow(_ project: MIR4DRecentProject) -> some View {
        let availability = MIR4DProjectSession.shared.availability(of: project)
        let available = availability == .available

        return Button {
            guard available else { return }
            commands.open(appState: appState, url: project.url)
        } label: {
            HStack(spacing: 12) {
                Image(systemName: available ? "cube.transparent" : "exclamationmark.triangle")
                    .font(.system(size: 17))
                    .foregroundStyle(available ? .white : .orange)
                    .frame(width: 27)

                VStack(alignment: .leading, spacing: 3) {
                    Text(project.name)
                        .font(.system(size: 12, weight: .medium))
                        .foregroundStyle(.white)
                        .lineLimit(1)
                    HStack(spacing: 6) {
                        Text(project.path)
                            .font(.system(size: 9, design: .monospaced))
                            .foregroundStyle(.secondary)
                            .lineLimit(1)
                        Text("•").foregroundStyle(.secondary)
                        Text(project.lastOpened.formatted(date: .abbreviated, time: .shortened))
                            .font(.system(size: 9))
                            .foregroundStyle(.secondary)
                    }
                }

                Spacer()
                Image(systemName: available ? "arrow.up.right" : "exclamationmark.triangle")
                    .font(.system(size: 10))
                    .foregroundStyle(available ? .secondary : .orange)
            }
            .padding(.horizontal, 14)
            .padding(.vertical, 10)
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
        .disabled(!available)
        .contextMenu {
            Button("Удалить из недавних") {
                MIR4DProjectSession.shared.removeFromRecents(id: project.id)
                refreshRecents()
            }
        }
    }

    // MARK: - Workspaces

    private var workspaceSection: some View {
        VStack(alignment: .leading, spacing: 12) {
            VStack(alignment: .leading, spacing: 3) {
                Text("Рабочие области")
                    .font(.system(size: 17, weight: .semibold))
                    .foregroundStyle(.white)
                Text("Дополнительные режимы MIR 4D — без перегрузки главного экрана")
                    .font(.system(size: 11))
                    .foregroundStyle(.secondary)
            }

            LazyVGrid(columns: [GridItem(.flexible()), GridItem(.flexible())], spacing: 10) {
                workspaceCard(.laboratory4D)
                workspaceCard(.mathematicalUniverse)
                workspaceCard(.programmingWorld)
                workspaceCard(.knowledgeWorld)
            }
        }
    }

    private func workspaceCard(_ mode: MIR4DStartMode) -> some View {
        Button { activate(mode) } label: {
            HStack(spacing: 12) {
                Image(systemName: mode.icon)
                    .font(.system(size: 17))
                    .foregroundStyle(.white)
                    .frame(width: 30)
                VStack(alignment: .leading, spacing: 2) {
                    Text(mode.title)
                        .font(.system(size: 12, weight: .medium))
                        .foregroundStyle(.white)
                    Text(mode.description)
                        .font(.system(size: 9))
                        .foregroundStyle(.secondary)
                        .lineLimit(2)
                }
                Spacer()
                Image(systemName: "chevron.right")
                    .font(.system(size: 9, weight: .bold))
                    .foregroundStyle(.secondary)
            }
            .padding(13)
            .frame(maxWidth: .infinity, minHeight: 68, alignment: .leading)
            .background(RoundedRectangle(cornerRadius: 11).fill(Color.white.opacity(0.028)))
            .overlay(RoundedRectangle(cornerRadius: 11).stroke(Color.white.opacity(0.06), lineWidth: 1))
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
        .help(mode.description)
    }

    // MARK: - Actions

    private func presentOpenProject() {
        showNewProject = false
        DispatchQueue.main.async { showOpenProject = true }
    }

    private func presentNewProject() {
        showOpenProject = false
        DispatchQueue.main.async { showNewProject = true }
    }

    private func activate(_ mode: MIR4DStartMode) {
        switch mode {
        case .openProject: presentOpenProject()
        case .newProject: presentNewProject()
        case .laboratory4D: enterWorkspace(workbench: .fourD, message: "4D лаборатория активирована")
        case .mathematicalUniverse: enterWorkspace(workbench: .simulation, message: "Математическая вселенная активирована")
        case .programmingWorld: enterWorkspace(workbench: .model, message: "Мир программирования открыт в рабочем пространстве")
        case .knowledgeWorld: enterWorkspace(workbench: .model, message: "МИР Знаний открыт в рабочем пространстве")
        }
    }

    private func enterWorkspace(workbench: CADWorkbench, message: String) {
        appState.selectWorkbench(workbench)
        appState.documentName = "Новый проект"
        appState.documentDirty = false
        appState.showNotification(message, type: .success)
        NotificationCenter.default.post(
            name: .mir4DStartWorkspace,
            object: nil,
            userInfo: ["workbench": workbench.rawValue]
        )
    }

    private func refreshRecents() {
        recentRefreshToken = UUID()
    }
}

struct MIR4DTopBarButtonStyle: ButtonStyle {
    let prominent: Bool

    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .font(.system(size: 11, weight: .semibold))
            .padding(.horizontal, 14)
            .frame(height: 32)
            .background(
                RoundedRectangle(cornerRadius: 8)
                    .fill(Color.white.opacity(prominent ? 0.12 : 0.055))
            )
            .overlay(
                RoundedRectangle(cornerRadius: 8)
                    .stroke(Color.white.opacity(prominent ? 0.16 : 0.08), lineWidth: 1)
            )
            .opacity(configuration.isPressed ? 0.72 : 1)
    }
}

struct MIR4DQuietButtonStyle: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .font(.system(size: 10, weight: .medium))
            .foregroundStyle(.secondary)
            .opacity(configuration.isPressed ? 0.55 : 1)
    }
}

struct MIR4DStartCardButtonStyle: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .scaleEffect(configuration.isPressed ? 0.985 : 1.0)
            .opacity(configuration.isPressed ? 0.84 : 1.0)
            .animation(.easeOut(duration: 0.12), value: configuration.isPressed)
    }
}

enum MIR4DStartMode: String, CaseIterable, Identifiable {
    case openProject, newProject, laboratory4D, mathematicalUniverse, programmingWorld, knowledgeWorld

    var id: String { rawValue }

    var title: String {
        switch self {
        case .openProject: return "Открыть проект"
        case .newProject: return "Создать новый проект"
        case .laboratory4D: return "4D лаборатория"
        case .mathematicalUniverse: return "Математическая вселенная"
        case .programmingWorld: return "Мир программирования"
        case .knowledgeWorld: return "МИР Знаний"
        }
    }

    var description: String {
        switch self {
        case .openProject: return "Открыть существующий инженерный проект MIR 4D."
        case .newProject: return "Создать новый проект CAD, BIM или 4D."
        case .laboratory4D: return "Симуляции, физические модели, сценарии и исследование поведения объектов во времени."
        case .mathematicalUniverse: return "Инженерные расчёты, математические модели и вычислительная среда нового поколения."
        case .programmingWorld: return "Создание программ из блоков, элементов и карт зависимостей кода."
        case .knowledgeWorld: return "Интерактивное обучение инженерии, математике, программированию и проектированию."
        }
    }

    var icon: String {
        switch self {
        case .openProject: return "folder"
        case .newProject: return "plus.square"
        case .laboratory4D: return "cube.transparent"
        case .mathematicalUniverse: return "function"
        case .programmingWorld: return "chevron.left.forwardslash.chevron.right"
        case .knowledgeWorld: return "books.vertical"
        }
    }
}
