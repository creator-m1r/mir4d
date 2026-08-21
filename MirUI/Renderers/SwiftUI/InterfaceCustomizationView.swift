import SwiftUI

struct InterfaceCustomizationView: View {
    @ObservedObject var appState: CADAppState

    var body: some View {
        MIR4DWorkspaceCustomizationView(appState: appState)
    }
}
