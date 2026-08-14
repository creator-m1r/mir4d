//
//  MIR4DStartMenuView.swift
//  MIR4D
//
//  Главное стартовое меню MIR 4D.
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

    private var recentProjects: [MIR4DRecentProject] {
        MIR4DProjectSession.shared.recentProjects
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
                Text("Начало работы")
                    .font(.system(size: 24, weight: .semibold))
                    .foregroundStyle(.white)
                Text("Выберите проект или направление работы MIR 4D")
                    .font(.system(size: 13))
                    .foregroundStyle(.secondary)

                quickActions

                if !recentProjects.isEmpty {
                    recentSection
                }

                scenariosSection
            }
            .padding(32)
        }
    }

    private var quickActions: some View {
        HStack(spacing: 12) {
            quickAction(title: "Создать проект", icon: "plus.square.fill") {
                presentNewProject()
            }
            quickAction(title: "Открыть проект", icon: "folder.fill") {
                presentOpenProject()
            }
        }
    }

    private func quickAction(title: String, icon: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            HStack(spacing: 10) {
                Image(systemName: icon).font(.system(size: 18))
                Text(title).font(.system(size: 14, weight: .semibold))
                Spacer()
                Image(systemName: "arrow.right")
                    .font(.system(size: 11, weight: .bold))
            }
            .foregroundStyle(.white)
            .padding(.horizontal, 18)
            .frame(maxWidth: .infinity, minHeight: 48)
            .background(RoundedRectangle(cornerRadius: 12).fill(Color.white.opacity(0.06)))
            .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.white.opacity(0.10), lineWidth: 1))
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
    }

    private var recentSection: some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack {
                Text("Недавние проекты")
                    .font(.system(size: 16, weight: .semibold))
                    .foregroundStyle(.white)
                Spacer()
                Text("до 10 проектов")
                    .font(.system(size: 10))
                    .foregroundStyle(.secondary)
            }

            VStack(spacing: 1) {
                ForEach(recentProjects) { project in
                    Button {
                        MIR4DProjectSession.shared.openProject(appState: appState, url: project.url)
                    } label: {
                        HStack(spacing: 12) {
                            Image(systemName: "cube.transparent")
                                .font(.system(size: 18))
                                .frame(width: 28)
                            VStack(alignment: .leading, spacing: 2) {
                                Text(project.name)
                                    .font(.system(size: 13, weight: .medium))
                                    .foregroundStyle(.white)
                                Text(project.path)
                                    .font(.system(size: 10, design: .monospaced))
                                    .foregroundStyle(.secondary)
                                    .lineLimit(1)
                            }
                            Spacer()
                            Image(systemName: "arrow.up.right")
                                .font(.system(size: 10))
                                .foregroundStyle(.secondary)
                        }
                        .padding(.horizontal, 14)
                        .padding(.vertical, 10)
                        .contentShape(Rectangle())
                    }
                    .buttonStyle(.plain)
                }
            }
            .background(RoundedRectangle(cornerRadius: 12).fill(Color.white.opacity(0.035)))
            .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.white.opacity(0.07), lineWidth: 1))

            Toggle("Открывать последний проект при запуске", isOn: Binding(
                get: { autoOpenLastProject },
                set: {
                    autoOpenLastProject = $0
                    MIR4DProjectSession.shared.isAutoOpenLastProjectEnabled = $0
                }
            ))
            .toggleStyle(.switch)
            .font(.system(size: 11))
            .foregroundStyle(.secondary)
        }
    }

    private var scenariosSection: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Сценарии работы")
                .font(.system(size: 16, weight: .semibold))
                .foregroundStyle(.white)

            LazyVGrid(columns: [GridItem(.flexible()), GridItem(.flexible()), GridItem(.flexible())], spacing: 16) {
                startCard(mode: .laboratory4D)
                startCard(mode: .mathematicalUniverse)
                startCard(mode: .programmingWorld)
                startCard(mode: .knowledgeWorld)
            }
        }
    }

    private func startCard(mode: MIR4DStartMode) -> some View {
        Button { activate(mode) } label: {
            VStack(alignment: .leading, spacing: 14) {
                Image(systemName: mode.icon).font(.system(size: 30)).foregroundStyle(.white)
                Text(mode.title).font(.system(size: 17, weight: .semibold)).foregroundStyle(.white)
                Text(mode.description).font(.system(size: 12)).foregroundStyle(.secondary).lineLimit(3)
                Spacer()
                HStack {
                    Text("Открыть рабочую область")
                    Spacer()
                    Image(systemName: "arrow.right")
                }
                .font(.system(size: 12)).foregroundStyle(.white.opacity(0.7))
            }
            .padding(22)
            .frame(maxWidth: .infinity, minHeight: 170, alignment: .leading)
            .contentShape(RoundedRectangle(cornerRadius: 16))
            .background(RoundedRectangle(cornerRadius: 16).fill(Color.white.opacity(0.035)))
            .overlay(RoundedRectangle(cornerRadius: 16).stroke(Color.white.opacity(0.08), lineWidth: 1))
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
        NotificationCenter.default.post(name: .mir4DStartWorkspace, object: nil, userInfo: ["workbench": workbench.rawValue])
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
