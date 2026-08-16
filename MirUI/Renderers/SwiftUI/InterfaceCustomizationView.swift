import SwiftUI

/// Compatibility entry point used by the existing top-bar command.
/// The implementation is delegated to the full live workspace editor.
struct InterfaceCustomizationView: View {
    @ObservedObject var appState: CADAppState

    var body: some View {
        MIR4DWorkspaceCustomizationView(appState: appState)
    }
}
