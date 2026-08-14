import SwiftUI

struct WorkbenchSwitcher: View {
    @ObservedObject var appState: CADAppState

    var body: some View {
        Menu {
            ForEach(CADWorkbench.allCases) { workbench in
                Button {
                    appState.selectWorkbench(workbench)
                } label: {
                    Label {
                        Text(localizedTitle(workbench))
                    } icon: {
                        Image(systemName: workbench.icon)
                    }
                }
            }
        } label: {
            HStack(spacing: MirTheme.Spacing.sm) {
                Image(systemName: appState.workbench.icon)
                Text(localizedTitle(appState.workbench))
                Image(systemName: "chevron.down")
                    .font(.system(size: 9, weight: .semibold))
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .font(MirTheme.Typography.bodySemibold)
            .foregroundStyle(MirTheme.Colors.textPrimary)
            .padding(.horizontal, MirTheme.Spacing.md)
            .padding(.vertical, MirTheme.Spacing.sm)
            .background(MirTheme.Colors.surfaceRaised)
            .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
            .overlay {
                RoundedRectangle(cornerRadius: MirTheme.Radius.medium)
                    .stroke(MirTheme.Colors.border, lineWidth: 1)
            }
        }
        .menuStyle(.borderlessButton)
        .help(appState.ui.language == .russian ? "Рабочая среда" : "Workbench")
    }

    private func localizedTitle(_ workbench: CADWorkbench) -> String {
        appState.ui.language == .russian ? workbench.titleRU : workbench.titleEN
    }
}
