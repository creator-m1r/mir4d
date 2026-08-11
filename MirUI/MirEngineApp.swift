// MirUI/MirEngineApp.swift
// =================================================================================
// Точка входа приложения MirEngine.
// Создаёт окно с MirGLView, внутри которого крутится весь рендеринг.
// =================================================================================

import SwiftUI

@main
struct MirEngineApp: App {
    var body: some Scene {
        WindowGroup {
            MirGLView()
                .frame(minWidth: 800, minHeight: 600)
        }
    }
}