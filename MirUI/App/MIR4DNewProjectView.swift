import SwiftUI
import AppKit

struct MIR4DNewProjectView: View {
    @Environment(\.dismiss) private var dismiss
    @EnvironmentObject private var appState: CADAppState

    var presetWorkbench: CADWorkbench? = nil
    var defaultName: String? = nil

    @State private var projectName: String
    @State private var parentURL: URL?
    @State private var errorMessage: String?

    init(presetWorkbench: CADWorkbench? = nil, defaultName: String? = nil) {
        self.presetWorkbench = presetWorkbench
        self.defaultName = defaultName
        _projectName = State(initialValue: defaultName ?? "Мой проект")
        _parentURL = State(initialValue: nil)
        _errorMessage = State(initialValue: nil)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            HStack(spacing: 12) {
                Image(systemName: "plus.square.fill").font(.system(size: 34))
                VStack(alignment: .leading, spacing: 3) {
                    Text("Создать новый проект").font(.title2.bold())
                    Text("Название и место хранения проекта MIR 4D").foregroundStyle(.secondary)
                }
            }

            VStack(alignment: .leading, spacing: 8) {
                Text("Название проекта").font(.headline)
                TextField("Например: Корпус насоса", text: $projectName)
                    .textFieldStyle(.roundedBorder)
                    .onSubmit { createProject() }
            }

            VStack(alignment: .leading, spacing: 8) {
                Text("Место хранения").font(.headline)
                HStack(spacing: 10) {
                    Image(systemName: "folder").foregroundStyle(.secondary)
                    Text(parentURL?.path ?? "Место не выбрано — нажмите «Выбрать…»")
                        .foregroundStyle(parentURL == nil ? .secondary : .primary)
                        .lineLimit(1)
                    Spacer()
                    Button("Выбрать…") { chooseFolder() }
                }
                .padding(10)
                .background(.quaternary.opacity(0.45), in: RoundedRectangle(cornerRadius: 8))
            }

            Text("Будет создан каталог «\(projectName.trimmingCharacters(in: .whitespacesAndNewlines)).mir4d» с папками Models, Scenes, Results и Documents.")
                .font(.caption).foregroundStyle(.secondary)

            if let errorMessage {
                Label(errorMessage, systemImage: "exclamationmark.triangle.fill")
                    .font(.caption).foregroundStyle(.red)
            }

            HStack {
                Spacer()
                Button("Отмена") { dismiss() }
                Button("Создать проект") { createProject() }
                    .buttonStyle(.borderedProminent)
                    .keyboardShortcut(.defaultAction)
            }
        }
        .padding(28)
        .frame(width: 620)
        .onAppear {
            if parentURL == nil {
                parentURL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first
            }
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in
            dismiss()
        }
    }

    private func chooseFolder() {
        let panel = NSOpenPanel()
        panel.title = "Выберите место хранения проекта MIR 4D"
        panel.message = "Выберите каталог, внутри которого будет создан проект."
        panel.prompt = "Выбрать"
        panel.canChooseFiles = false
        panel.canChooseDirectories = true
        panel.allowsMultipleSelection = false
        panel.canCreateDirectories = true
        panel.directoryURL = parentURL
        if panel.runModal() == .OK {
            parentURL = panel.url
            errorMessage = nil
        }
    }

    private func createProject() {
        let trimmedName = projectName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmedName.isEmpty else {
            errorMessage = "Введите название проекта."
            return
        }
        if trimmedName.contains("/") || trimmedName.contains(":") {
            errorMessage = "Название проекта содержит недопустимый символ."
            return
        }
        if parentURL == nil {
            chooseFolder()
            guard parentURL != nil else { return }
        }
        guard let parentURL else { return }
        let projectURL = parentURL.appendingPathComponent("\(trimmedName).mir4d", isDirectory: true)
        if FileManager.default.fileExists(atPath: projectURL.path) {
            errorMessage = "Проект с таким названием уже существует в выбранном месте."
            return
        }

        errorMessage = nil
        appState.createMIR4DProject(name: trimmedName, parentURL: parentURL, workbench: presetWorkbench)
    }
}
