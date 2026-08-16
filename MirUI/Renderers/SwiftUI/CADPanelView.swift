import SwiftUI

/// Renders a single dockable panel body.
/// UI-only presentation layer: MirEngine remains untouched.
struct CADPanelView: View {
    let panel: CADPanel
    @ObservedObject var appState: CADAppState
    @ObservedObject private var workspace = MIR4DWorkspaceCustomizationStore.shared

    var body: some View {
        VStack(spacing: 0) {
            if workspace.showPanelHeaders { panelHeader }
            panelContent
        }
        .background(MirTheme.Colors.panel.opacity(workspace.panelOpacity))
        .clipShape(RoundedRectangle(cornerRadius: workspace.panelCornerRadius))
        .overlay(alignment: .bottom) {
            Rectangle().fill(MirTheme.Colors.border).frame(height: 1)
        }
    }

    private var panelHeader: some View {
        HStack(spacing: workspace.compactPanels ? 6 : 9) {
            Image(systemName: panelIcon)
                .font(.system(size: workspace.compactPanels ? 10 : 11, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)
                .frame(width: workspace.compactPanels ? 20 : 22, height: workspace.compactPanels ? 20 : 22)
                .background(MirTheme.Colors.accentSoft)
                .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))

            VStack(alignment: .leading, spacing: 2) {
                Text(panelTitle)
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                if !workspace.compactPanels {
                    Text(panelSubtitle)
                        .font(MirTheme.Typography.status)
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                }
            }

            Spacer(minLength: 8)

            Menu {
                Button { appState.togglePanel(panel) } label: { Label(appState.ui.language == .russian ? "Скрыть панель" : "Hide panel", systemImage: "eye.slash") }
                Button { appState.setPanelPlacement(.left, for: panel) } label: { Label(appState.ui.language == .russian ? "Переместить влево" : "Move left", systemImage: "sidebar.leading") }
                Button { appState.setPanelPlacement(.right, for: panel) } label: { Label(appState.ui.language == .russian ? "Переместить вправо" : "Move right", systemImage: "sidebar.trailing") }
                Button { appState.setPanelPlacement(.bottom, for: panel) } label: { Label(appState.ui.language == .russian ? "Переместить вниз" : "Move bottom", systemImage: "rectangle.bottomhalf.inset.filled") }
                Button { appState.setPanelPlacement(.floating, for: panel) } label: { Label(appState.ui.language == .russian ? "Отделить в окно" : "Detach as window", systemImage: "macwindow.on.rectangle") }
            } label: {
                Image(systemName: "ellipsis").font(.system(size: 10, weight: .semibold)).frame(width: 24, height: 24)
            }
            .menuStyle(.borderlessButton)
            .foregroundStyle(MirTheme.Colors.textTertiary)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.85), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border, lineWidth: 1))

            Button { appState.togglePanel(panel) } label: {
                Image(systemName: "xmark").font(.system(size: 9, weight: .semibold)).frame(width: 24, height: 24)
            }
            .buttonStyle(.plain)
            .foregroundStyle(MirTheme.Colors.textTertiary)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.85), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .help(appState.ui.language == .russian ? "Скрыть панель" : "Hide panel")
        }
        .padding(.horizontal, workspace.compactPanels ? 8 : 11)
        .padding(.vertical, workspace.compactPanels ? 5 : 8)
        .background(MirTheme.Colors.surface.opacity(workspace.panelOpacity))
        .overlay(alignment: .bottom) { Rectangle().fill(MirTheme.Colors.border.opacity(0.8)).frame(height: 1) }
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
            FourDTimelineView(appState: appState).frame(minHeight: 170, idealHeight: 230, maxHeight: 300)
        case .assembly:
            TimelinePanelView(appState: appState).frame(minHeight: 170, idealHeight: 220, maxHeight: 280)
        default:
            TimelinePanelView(appState: appState).frame(minHeight: 190, idealHeight: 250, maxHeight: 320)
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
            return appState.workbench == .fourD || appState.workbench == .simulation ? (appState.ui.language == .russian ? "4D и сценарии" : "4D and scenarios") : (appState.ui.language == .russian ? "Временная последовательность" : "Time sequence")
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
