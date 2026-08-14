import SwiftUI

/// Renders a single dockable panel body.
/// Used both inside the main workspace and inside floating windows.
struct CADPanelView: View {
    let panel: CADPanel
    @ObservedObject var appState: CADAppState

    var body: some View {
        switch panel {
        case .project:
            SidebarView(appState: appState)
        case .properties:
            VStack(spacing: 0) {
                SelectionIdentityInspector(appState: appState)
                InspectorTabsView(appState: appState)
            }
        case .timeline:
            timeline
        default:
            EmptyView()
        }
    }

    @ViewBuilder private var timeline: some View {
        switch appState.workbench {
        case .fourD, .simulation:
            FourDTimelineView(appState: appState).frame(minHeight: 170, idealHeight: 230, maxHeight: 300)
        case .assembly:
            TimelinePanelView(appState: appState).frame(minHeight: 170, idealHeight: 220, maxHeight: 280)
        default:
            TimelinePanelView(appState: appState).frame(minHeight: 190, idealHeight: 250, maxHeight: 320)
        }
    }
}