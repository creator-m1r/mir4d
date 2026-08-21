
import SwiftUI
import AppKit

struct MIR4DProjectOpenView: View {
    @Environment(\.dismiss) private var dismiss
    @EnvironmentObject private var appState: CADAppState

    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: "folder")
                .font(.system(size: 44))
            Text("Открыть проект")
                .font(.title2)
                .bold()
            Text("Выберите каталог проекта MIR 4D (.mir4d).")
                .foregroundStyle(.secondary)
            HStack {
                Button("Отмена") { dismiss() }
                Button("Выбрать проект") { openProject() }
                    .buttonStyle(.borderedProminent)
            }
        }
        .padding(40)
        .frame(width: 500, height: 260)
    }

    private func openProject() {
        let panel = NSOpenPanel()
        panel.title = "Открыть проект MIR 4D"
        panel.message = "Выберите каталог с расширением .mir4d."
        panel.prompt = "Открыть"
        panel.canChooseFiles = false
        panel.canChooseDirectories = true
        panel.allowsMultipleSelection = false
        panel.canCreateDirectories = false
        panel.treatsFilePackagesAsDirectories = true

        guard panel.runModal() == .OK,
              let url = panel.url else {
            return
        }

        guard url.pathExtension.lowercased() == "mir4d" else {
            appState.showNotification(
                "Выберите каталог проекта с расширением .mir4d",
                type: .warning
            )
            return
        }

        appState.openMIR4DProject(url: url)
        dismiss()
    }
}
