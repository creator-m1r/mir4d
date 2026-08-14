//
//  MIR4DStartMenuView.swift
//  MIR4D
//
//  Project Hub — центральная точка навигации по проектам MIR 4D.
//  Вся работа с проектами проходит через MIR4DProjectCommands.
//

import SwiftUI
import AppKit

struct MIR4DStartMenuView: View {
    @EnvironmentObject private var appState: CADAppState
    @ObservedObject var diagnostic: MIR4DBootCoordinator

    @State private var selectedMode: MIR4DStartMode?
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

    private var continueAvailability: MIR4DRecentAvailability? {
        guard let project = continueProject else { return nil }
        return MIR4DProjectSession.shared.availability(of: project)
    }

    private var canContinue: Bool { continueAvailability == .available }

    private var continueStatusText: String {
        if continueProject == nil { return "Нет проекта для продолжения" }
        return canContinue ? "Последний проект" : "Проект недоступен"
    }

    private var continueStatusColor: Color {
        canContinue ? .secondary : .orange
    }

    var body: some View {
        VStack(spacing: 0) {
            header
            Divider().opacity(0.2)
            content
            Divider().opacity(0.2)
            footer
        }
        .background(Color(red: 0.025, green: 0.035, blue: 0.055))
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

    private var header: some View {
        HStack {
            VStack(alignment: .leading, spacing: 4) {
                Text("МИР 4D")
                    .font(.system(size: 28, weight: .bold, design: .rounded))
                Text("Инженерная среда моделирования")
                    .font(.system(size: 12))
                    .foregroundStyle(.secondary)
            }
            Spacer()
            HStack(spacing: 8) {
                Circle().fill(.green).frame(width: 8, height: 8)
                Text("Система готова")
                    .font(.system(size: 12))
                    .foregroundStyle(.secondary)
            }
        }
        .foregroundStyle(.white)
        .padding(.horizontal, 32)
        .padding(.vertical, 22)
    }

    private var content: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 22) {
                VStack(alignment: .leading, spacing: 5) {
                    Text("Начало работы")
                        .font(.system(size: 24, weight: .semibold))
                        .foregroundStyle(.white)
                    Text("Создайте проект, откройте существующий или продолжите работу.")
                        .font(.system(size: 13))
                        .foregroundStyle(.secondary)
                }

                primaryActions
                openButton
                recentSection
                scenariosSection
            }
            .padding(32)
        }
    }

    private var primaryActions: some View {
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
            continueCardContent
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
        .disabled(!canContinue)
        .help(canContinue ? "Открыть последний проект" : "Нет доступного проекта для продолжения")
    }

    private var continueCardContent: some View {
        VStack(alignment: .leading, spacing: 11) {
            HStack {
                Image(systemName: "arrow.forward.circle.fill")
                    .font(.system(size: 23))
                Spacer()
                Image(systemName: "arrow.right")
                    .font(.system(size: 11, weight: .bold))
            }

            Text("Продолжить")
                .font(.system(size: 18, weight: .semibold))

            if let project = continueProject {
                Text(project.name)
                    .font(.system(size: 13, weight: .medium))
                    .foregroundStyle(.white.opacity(0.9))
                    .lineLimit(1)
            }

            Text(continueStatusText)
                .font(.system(size: 11))
                .foregroundStyle(continueStatusColor)

            Spacer(minLength: 4)
        }
        .foregroundStyle(.white)
        .padding(20)
        .frame(maxWidth: .infinity, minHeight: 142, alignment: .leading)
        .background(
            RoundedRectangle(cornerRadius: 16)
                .fill(Color.white.opacity(canContinue ? 0.07 : 0.035))
        )
        .overlay(
            RoundedRectangle(cornerRadius: 16)
                .stroke(Color.white.opacity(canContinue ? 0.13 : 0.06), lineWidth: 1)
        )
    }

    private var createCard: some View {
        Button { presentNewProject() } label: {
            VStack(alignment: .leading, spacing: 11) {
                HStack {
                    Image(systemName: "plus.square.fill").font(.system(size: 23))
                    Spacer()
                    Image(systemName: "arrow.right").font(.system(size: 11, weight: .bold))
                }
                Text("Создать проект").font(.system(size: 18, weight: .semibold))
                Text("Новый проект MIR 4D")
                    .font(.system(size: 13, weight: .medium))
                    .foregroundStyle(.white.opacity(0.9))
                Text("CAD • BIM • 4D")
                    .font(.system(size: 11))
                    .foregroundStyle(.secondary)
                Spacer(minLength: 4)
            }
            .foregroundStyle(.white)
            .padding(20)
            .frame(maxWidth: .infinity, minHeight: 142, alignment: .leading)
            .background(RoundedRectangle(cornerRadius: 16).fill(Color.white.opacity(0.07)))
            .overlay(RoundedRectangle(cornerRadius: 16).stroke(Color.white.opacity(0.13), lineWidth: 1))
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
    }

    private var openButton: some View {
        Button { presentOpenProject() } label: {
            HStack(spacing: 12) {
                Image(systemName: "folder.fill").font(.system(size: 18))
                VStack(alignment: .leading, spacing: 2) {
                    Text("Открыть проект .mir4d").font(.system(size: 14, weight: .semibold))
                    Text("Выбрать существующий проект MIR 4D")
                        .font(.system(size: 11))
                        .foregroundStyle(.secondary)
                }
                Spacer()
                Image(systemName: "arrow.right").font(.system(size: 11, weight: .bold))
            }
            .foregroundStyle(.white)
            .padding(.horizontal, 18)
            .frame(maxWidth: .infinity, minHeight: 52)
            .background(RoundedRectangle(cornerRadius: 12).fill(Color.white.opacity(0.045)))
            .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.white.opacity(0.09), lineWidth: 1))
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
    }

    private var recentSection: some View {
        VStack(alignment: .leading, spacing: 11) {
            HStack {
                VStack(alignment: .leading, spacing: 3) {
                    Text("Недавние проекты")
                        .font(.system(size: 16, weight: .semibold))
                        .foregroundStyle(.white)
                    Text("Последние открытые проекты MIR 4D")
                        .font(.system(size: 11))
                        .foregroundStyle(.secondary)
                }
                Spacer()
                Text("до 10")
                    .font(.system(size: 10, weight: .medium, design: .monospaced))
                    .foregroundStyle(.secondary)
            }

            if recentProjects.isEmpty {
                emptyRecentState
            } else {
                VStack(spacing: 1) {
                    ForEach(recentProjects) { project in
                        recentRow(project)
                    }
                }
                .background(RoundedRectangle(cornerRadius: 12).fill(Color.white.opacity(0.035)))
                .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.white.opacity(0.07), lineWidth: 1))
            }

            Toggle(
                "Открывать последний проект при запуске",
                isOn: Binding(
                    get: { autoOpenLastProject },
                    set: { value in
                        autoOpenLastProject = value
                        MIR4DProjectSession.shared.isAutoOpenLastProjectEnabled = value
                    }
                )
            )
            .toggleStyle(.switch)
            .font(.system(size: 11))
            .foregroundStyle(.secondary)
        }
    }

    private var emptyRecentState: some View {
        HStack(spacing: 12) {
            Image(systemName: "clock.arrow.circlepath")
                .font(.system(size: 20))
                .foregroundStyle(.secondary)
            VStack(alignment: .leading, spacing: 3) {
                Text("Нет недавних проектов")
                    .font(.system(size: 12, weight: .medium))
                    .foregroundStyle(.white)
                Text("Создайте проект или откройте существующий .mir4d.")
                    .font(.system(size: 11))
                    .foregroundStyle(.secondary)
            }
            Spacer()
        }
        .padding(16)
        .background(RoundedRectangle(cornerRadius: 12).fill(Color.white.opacity(0.025)))
        .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.white.opacity(0.06), lineWidth: 1))
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
                    .font(.system(size: 18))
                    .foregroundStyle(available ? .white : .orange)
                    .frame(width: 28)

                VStack(alignment: .leading, spacing: 3) {
                    Text(project.name)
                        .font(.system(size: 13, weight: .medium))
                        .foregroundStyle(.white)
                        .lineLimit(1)

                    HStack(spacing: 7) {
                        Text(project.path)
                            .font(.system(size: 10, design: .monospaced))
                            .foregroundStyle(.secondary)
                            .lineLimit(1)
                        Text("•").foregroundStyle(.secondary)
                        Text(project.lastOpened.formatted(date: .abbreviated, time: .shortened))
                            .font(.system(size: 10))
                            .foregroundStyle(.secondary)
                    }

                    if availability == .missing {
                        Text("Проект недоступен: папка не найдена")
                            .font(.system(size: 10))
                            .foregroundStyle(.orange)
                    } else if availability == .invalid {
                        Text("Проект недоступен: пакет MIR 4D повреждён или неполный")
                            .font(.system(size: 10))
                            .foregroundStyle(.orange)
                    }
                }

                Spacer()

                if available {
                    Image(systemName: "arrow.up.right")
                        .font(.system(size: 10))
                        .foregroundStyle(.secondary)
                } else {
                    Text("недоступен")
                        .font(.system(size: 10, weight: .medium))
                        .foregroundStyle(.orange)
                }
            }
            .padding(.horizontal, 14)
            .padding(.vertical, 11)
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

    private var scenariosSection: some View {
        VStack(alignment: .leading, spacing: 12) {
            HStack {
                VStack(alignment: .leading, spacing: 3) {
                    Text("Сценарии работы")
                        .font(.system(size: 16, weight: .semibold))
                        .foregroundStyle(.white)
                    Text("Рабочие области MIR 4D")
                        .font(.system(size: 11))
                        .foregroundStyle(.secondary)
                }
                Spacer()
            }

            LazyVGrid(columns: [GridItem(.flexible()), GridItem(.flexible())], spacing: 14) {
                startCard(mode: .laboratory4D)
                startCard(mode: .mathematicalUniverse)
                startCard(mode: .programmingWorld)
                startCard(mode: .knowledgeWorld)
            }
        }
    }

    private func startCard(mode: MIR4DStartMode) -> some View {
        Button { activate(mode) } label: {
            VStack(alignment: .leading, spacing: 12) {
                Image(systemName: mode.icon).font(.system(size: 27)).foregroundStyle(.white)
                Text(mode.title).font(.system(size: 16, weight: .semibold)).foregroundStyle(.white)
                Text(mode.description).font(.system(size: 11)).foregroundStyle(.secondary).lineLimit(3)
                Spacer(minLength: 4)
                HStack {
                    Text("Открыть рабочую область")
                    Spacer()
                    Image(systemName: "arrow.right")
                }
                .font(.system(size: 11))
                .foregroundStyle(.white.opacity(0.7))
            }
            .padding(19)
            .frame(maxWidth: .infinity, minHeight: 155, alignment: .leading)
            .contentShape(RoundedRectangle(cornerRadius: 15))
            .background(RoundedRectangle(cornerRadius: 15).fill(Color.white.opacity(0.035)))
            .overlay(RoundedRectangle(cornerRadius: 15).stroke(Color.white.opacity(0.08), lineWidth: 1))
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
        .help(mode.description)
    }

    private var footer: some View {
        HStack {
            Text("Самодиагностика: \(Int(diagnostic.progress * 100))%")
                .font(.system(size: 11, design: .monospaced))
                .foregroundStyle(.secondary)
            Spacer()
            Text("MIR 4D • Engineering Platform")
                .font(.system(size: 11))
                .foregroundStyle(.secondary)
        }
        .padding(.horizontal, 32)
        .padding(.vertical, 14)
    }

    private func presentOpenProject() {
        showNewProject = false
        DispatchQueue.main.async { showOpenProject = true }
    }

    private func presentNewProject() {
        selectedMode = .newProject
        showOpenProject = false
        DispatchQueue.main.async { showNewProject = true }
    }

    private func activate(_ mode: MIR4DStartMode) {
        selectedMode = mode
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

struct MIR4DStartCardButtonStyle: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .scaleEffect(configuration.isPressed ? 0.98 : 1.0)
            .opacity(configuration.isPressed ? 0.85 : 1.0)
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
        case .knowledgeWorld: return "graduationcap"
        }
    }
}

extension Notification.Name {
    static let mir4DOpenProject = Notification.Name("MIR4D.OpenProject")
    static let mir4DStartWorkspace = Notification.Name("MIR4D.StartWorkspace")
    static let mir4DStart4DLaboratory = Notification.Name("MIR4D.Start4DLaboratory")
    static let mir4DStartMathUniverse = Notification.Name("MIR4D.StartMathUniverse")
    static let mir4DStartProgrammingWorld = Notification.Name("MIR4D.StartProgrammingWorld")
    static let mir4DStartKnowledgeWorld = Notification.Name("MIR4D.StartKnowledgeWorld")
}
