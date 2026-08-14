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
    @State private var presentedPanel: StartPanel?

    private enum StartPanel: String, Identifiable {
        case openProject
        case newProject
        var id: String { rawValue }
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
        .sheet(item: $presentedPanel) { panel in
            switch panel {
            case .openProject:
                MIR4DProjectOpenView().environmentObject(appState)
            case .newProject:
                MIR4DNewProjectView().environmentObject(appState)
            }
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRequestNewProject)) { _ in
            present(.newProject)
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DOpenProject)) { _ in
            present(.openProject)
        }
    }

    private var header: some View {
        HStack {
            VStack(alignment: .leading, spacing: 4) {
                Text("МИР 4D").font(.system(size: 28, weight: .bold, design: .rounded))
                Text("Инженерная среда моделирования").font(.system(size: 12)).foregroundStyle(.secondary)
            }
            Spacer()
            HStack(spacing: 8) {
                Circle().fill(.green).frame(width: 8, height: 8)
                Text("Система готова").font(.system(size: 12)).foregroundStyle(.secondary)
            }
        }
        .foregroundStyle(.white)
        .padding(.horizontal, 32)
        .padding(.vertical, 22)
    }

    private var content: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 22) {
                Text("Начало работы").font(.system(size: 24, weight: .semibold)).foregroundStyle(.white)
                Text("Выберите направление работы MIR 4D").font(.system(size: 13)).foregroundStyle(.secondary)
                LazyVGrid(columns: [GridItem(.flexible()), GridItem(.flexible()), GridItem(.flexible())], spacing: 16) {
                    startCard(mode: .openProject)
                    startCard(mode: .newProject)
                    startCard(mode: .laboratory4D)
                    startCard(mode: .mathematicalUniverse)
                    startCard(mode: .programmingWorld)
                    startCard(mode: .knowledgeWorld)
                }
            }
            .padding(32)
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
                    Text("Начать")
                    Spacer()
                    Image(systemName: "arrow.right")
                }
                .font(.system(size: 12)).foregroundStyle(.white.opacity(0.7))
            }
            .padding(22)
            .frame(maxWidth: .infinity, minHeight: 190, alignment: .leading)
            .contentShape(RoundedRectangle(cornerRadius: 16))
            .background(RoundedRectangle(cornerRadius: 16).fill(Color.white.opacity(0.035)))
            .overlay(RoundedRectangle(cornerRadius: 16).stroke(Color.white.opacity(0.08), lineWidth: 1))
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
        .help(mode.description)
    }

    private var footer: some View {
        HStack {
            Text("Самодиагностика: \(Int(diagnostic.progress * 100))%").font(.system(size: 11, design: .monospaced)).foregroundStyle(.secondary)
            Spacer()
            Text("MIR 4D • Engineering Platform").font(.system(size: 11)).foregroundStyle(.secondary)
        }
        .padding(.horizontal, 32)
        .padding(.vertical, 14)
    }

    private func present(_ panel: StartPanel) {
        selectedMode = panel == .newProject ? .newProject : .openProject
        DispatchQueue.main.async { presentedPanel = panel }
    }

    private func activate(_ mode: MIR4DStartMode) {
        selectedMode = mode
        switch mode {
        case .openProject: present(.openProject)
        case .newProject: present(.newProject)
        case .laboratory4D:
            appState.selectWorkbench(.fourD)
            appState.showNotification("4D лаборатория активирована", type: .success)
        case .mathematicalUniverse:
            NotificationCenter.default.post(name: .mir4DStartMathUniverse, object: nil)
        case .programmingWorld:
            NotificationCenter.default.post(name: .mir4DStartProgrammingWorld, object: nil)
        case .knowledgeWorld:
            NotificationCenter.default.post(name: .mir4DStartKnowledgeWorld, object: nil)
        }
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
    static let mir4DStart4DLaboratory = Notification.Name("MIR4D.Start4DLaboratory")
    static let mir4DStartMathUniverse = Notification.Name("MIR4D.StartMathUniverse")
    static let mir4DStartProgrammingWorld = Notification.Name("MIR4D.StartProgrammingWorld")
    static let mir4DStartKnowledgeWorld = Notification.Name("MIR4D.StartKnowledgeWorld")
}
