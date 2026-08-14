import SwiftUI

/// Recursive model-tree row used by SidebarView.
/// Keeps tree presentation local to SwiftUI; selection remains owned by CADAppState.
struct TreeItemView: View {
    let node: TreeNodeData
    @ObservedObject var appState: CADAppState
    let level: Int
    @State private var isExpanded: Bool

    init(node: TreeNodeData, appState: CADAppState, level: Int) {
        self.node = node
        self.appState = appState
        self.level = level
        _isExpanded = State(initialValue: level == 0)
    }

    private var isSelected: Bool {
        appState.selectedTreeItem == node.name
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 1) {
            row

            if isExpanded {
                ForEach(node.children) { child in
                    TreeItemView(node: child, appState: appState, level: level + 1)
                }
            }
        }
    }

    private var row: some View {
        HStack(spacing: 5) {
            if !node.children.isEmpty {
                Button {
                    withAnimation(MirTheme.Animation.fast) {
                        isExpanded.toggle()
                    }
                } label: {
                    Image(systemName: isExpanded ? "chevron.down" : "chevron.right")
                        .font(.system(size: 8, weight: .semibold))
                        .frame(width: 12, height: 18)
                }
                .buttonStyle(.plain)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            } else {
                Color.clear
                    .frame(width: 12, height: 18)
            }

            Image(systemName: node.icon)
                .font(.system(size: 11, weight: .medium))
                .frame(width: 16)
                .foregroundStyle(iconColor)

            Text(node.name)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(
                    isSelected
                        ? MirTheme.Colors.textPrimary
                        : MirTheme.Colors.textSecondary
                )
                .lineLimit(1)

            Spacer(minLength: 4)

            statusIndicator
        }
        .padding(.leading, CGFloat(level) * 14)
        .padding(.horizontal, 6)
        .padding(.vertical, 4)
        .background(
            RoundedRectangle(cornerRadius: MirTheme.Radius.small)
                .fill(isSelected ? MirTheme.Colors.accentSoft : Color.clear)
        )
        .contentShape(Rectangle())
        .onTapGesture {
            appState.selectedTreeItem = node.name
        }
        .contextMenu {
            Button("Select") {
                appState.selectedTreeItem = node.name
            }
            if !node.children.isEmpty {
                Button(isExpanded ? "Collapse" : "Expand") {
                    isExpanded.toggle()
                }
            }
        }
    }

    @ViewBuilder
    private var statusIndicator: some View {
        switch node.status {
        case .none:
            EmptyView()
        case .approved:
            Circle()
                .fill(MirTheme.Colors.success)
                .frame(width: 5, height: 5)
        case .inProgress:
            Circle()
                .fill(MirTheme.Colors.accent)
                .frame(width: 5, height: 5)
        case .issue:
            Circle()
                .fill(MirTheme.Colors.error)
                .frame(width: 5, height: 5)
        }
    }

    private var iconColor: Color {
        if isSelected { return MirTheme.Colors.accentBright }
        switch node.status {
        case .issue: return MirTheme.Colors.error
        case .approved: return MirTheme.Colors.success
        case .inProgress: return MirTheme.Colors.accent
        case .none: return MirTheme.Colors.textTertiary
        }
    }
}
