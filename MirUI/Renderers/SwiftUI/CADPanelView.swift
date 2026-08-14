import SwiftUI

/// Renders a single dockable panel body.
/// Used both inside the main workspace and inside floating windows.
struct CADPanelView: View {
    let panel: CADPanel
    @ObservedObject var appState: CADAppState

    var body: some View {
        VStack(spacing: 0) {
            panelHeader
            panelContent
        }
        .background(MirTheme.Colors.panel)
        .overlay(alignment: .bottom) {
            Rectangle().fill(MirTheme.Colors.border).frame(height: 1)
        }
    }

    private var panelHeader: some View {
        HStack(spacing: 8) {
            Image(systemName: panelIcon)
                .font(.system(size: 11, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)

            VStack(alignment: .leading, spacing: 1) {
                Text(panelTitle)
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Text(panelSubtitle)
                    .font(MirTheme.Typography.status)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }

            Spacer()

            Button { appState.togglePanel(panel) } label: {
                Image(systemName: "xmark")
                    .font(.system(size: 9, weight: .semibold))
                    .frame(width: 22, height: 22)
            }
            .buttonStyle(.plain)
            .foregroundStyle(MirTheme.Colors.textTertiary)
            .contentShape(Rectangle())
            .help(appState.ui.language == .russian ? "Скрыть панель" : "Hide panel")
        }
        .padding(.horizontal, 11)
        .padding(.vertical, 8)
        .background(MirTheme.Colors.surface.opacity(0.55))
        .overlay(alignment: .bottom) {
            Rectangle().fill(MirTheme.Colors.border.opacity(0.8)).frame(height: 1)
        }
    }

    @ViewBuilder
    private var panelContent: some View {
        switch panel {
        case .project:
            SidebarView(appState: appState)
        case .properties:
            ScrollView {
                VStack(spacing: 0) {
                    SelectionIdentityInspector(appState: appState)
                    InspectorTabsView(appState: appState)
                }
                .frame(maxWidth: .infinity, alignment: .top)
            }
            .scrollIndicators(.hidden)
        case .timeline:
            timeline
        default:
            EmptyView()
        }
    }

    @ViewBuilder private var timeline: some View {
        switch appState.workbench {
        case .fourD, .simulation:
            FourDTimelineView(appState: appState)
                .frame(minHeight: 170, idealHeight: 230, maxHeight: 300)
        case .assembly:
            TimelinePanelView(appState: appState)
                .frame(minHeight: 170, idealHeight: 220, maxHeight: 280)
        default:
            TimelinePanelView(appState: appState)
                .frame(minHeight: 190, idealHeight: 250, maxHeight: 320)
        }
    }

    private var panelTitle: String {
        switch panel {
        case .project: return appState.ui.language == .russian ? "Навигатор" : "Navigator"
        case .properties: return appState.ui.language == .russian ? "Инспектор" : "Inspector"
        case .timeline: return appState.ui.language == .russian ? "Временная шкала" : "Timeline"
        default: return panel.rawValue.capitalized
        }
    }

    private var panelSubtitle: String {
        switch panel {
        case .project: return appState.ui.language == .russian ? "Структура проекта" : "Project structure"
        case .properties: return appState.ui.language == .russian ? "Свойства выбранного объекта" : "Selected object properties"
        case .timeline:
            return appState.workbench == .fourD || appState.workbench == .simulation
                ? (appState.ui.language == .russian ? "4D и сценарии" : "4D and scenarios")
                : (appState.ui.language == .russian ? "Временная последовательность" : "Time sequence")
        default: return ""
        }
    }

    private var panelIcon: String {
        switch panel {
        case .project: return "list.bullet.indent"
        case .properties: return "slider.horizontal.3"
        case .timeline: return "timeline.selection"
        default: return "rectangle"
        }
    }
}
