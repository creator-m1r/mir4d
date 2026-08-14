//
//  MIR4DApp.swift
//  MIR4D
//
//  Главная точка входа MIR 4D.
//

import SwiftUI
import AppKit

@main
struct MIR4DApp: App {

    @StateObject private var appState = CADAppState()

    var body: some Scene {
        WindowGroup {
            MIR4DStartupView()
                .environmentObject(appState)
                .frame(minWidth: 1280, minHeight: 800)
        }
        .windowStyle(.hiddenTitleBar)
        .commands {
            CommandGroup(replacing: .newItem) {
                Button("Новый проект") {
                    appState.newDocument()
                }
                .keyboardShortcut("n", modifiers: [.command])

                Button("Открыть проект…") {
                    NotificationCenter.default.post(
                        name: .mir4DOpenProject,
                        object: nil
                    )
                }
                .keyboardShortcut("o", modifiers: [.command])
            }
        }
    }
}