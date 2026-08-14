//
//  MIR4DProjectOpenView.swift
//

import SwiftUI
import AppKit

struct MIR4DProjectOpenView: View {

    @Environment(\.dismiss)
    private var dismiss

    @EnvironmentObject
    private var appState: CADAppState

    var body: some View {

        VStack(spacing: 20) {

            Image(systemName: "folder")
                .font(.system(size: 44))

            Text("Открыть проект")
                .font(.title2)
                .bold()

            Text(
                "Выберите файл проекта MIR 4D."
            )
            .foregroundStyle(.secondary)

            HStack {

                Button("Отмена") {
                    dismiss()
                }

                Button("Выбрать проект") {

                    openProject()
                }
                .buttonStyle(.borderedProminent)
            }
        }
        .padding(40)
        .frame(
            width: 500,
            height: 260
        )
    }

    private func openProject() {

        let panel =
            NSOpenPanel()

        panel.title =
            "Открыть проект MIR 4D"

        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false

        panel.allowedContentTypes = []

        if panel.runModal() == .OK,
           let url = panel.url {

            appState.documentName =
                url.deletingPathExtension()
                    .lastPathComponent

            appState.documentDirty = false

            appState.showNotification(
                "Проект открыт: \(url.lastPathComponent)",
                type: .success
            )

            dismiss()
        }
    }
}