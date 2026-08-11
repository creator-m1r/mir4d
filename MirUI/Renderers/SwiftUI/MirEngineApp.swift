import SwiftUI

@main
struct MirEngineApp: App {
    var body: some Scene {
        WindowGroup {
            CADMainView()
                .frame(minWidth: 1400, minHeight: 900)
                .preferredColorScheme(.dark)
        }
        .windowStyle(.hiddenTitleBar)
        .commands {
            CommandGroup(replacing: .newItem) {}
        }
    }
}