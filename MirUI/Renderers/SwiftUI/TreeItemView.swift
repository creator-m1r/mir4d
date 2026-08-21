import SwiftUI

struct TreeItemView: View {
    let node: TreeNodeData
    @ObservedObject var appState: CADAppState
    @ObservedObject private var modelRuntime = MIR4DModelRuntime.shared
    let level: Int
    @State private var isExpanded: Bool

    init(node: TreeNodeData, appState: CADAppState, level: Int) {
        self.node = node
        self.appState = appState
        self.level = level
        _isExpanded = State(initialValue: level == 0)
    }

    private var isSelected: Bool {
        appState.selection.ids.contains(node.id.uuidString)
    }

    private var modelNodeKind: MIR4DModelNode.Kind? {
        findModelNode(in: modelRuntime.document.root, id: node.id)?.kind
    }

    private var selectionKind: CADSelectionKind {
        switch modelNodeKind {
        case .project: return .none
        case .component: return .component
        case .body: return .body
        case .sketch: return .sketch
        case .operation: return .feature
        case .result: return .feature
        case nil: return .unknown
        }
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
            selectNode()
        }
        .contextMenu {
            Button("Select") {
                selectNode()
            }
            if !node.children.isEmpty {
                Button(isExpanded ? "Collapse" : "Expand") {
                    isExpanded.toggle()
                }
            }
        }
    }

    private func selectNode() {

        appState.selectedTreeItem = node.name
        appState.setSelection(
            ids: [node.id.uuidString],
            kind: selectionKind
        )
    }

    private func findModelNode(in root: MIR4DModelNode, id: UUID) -> MIR4DModelNode? {
        if root.id == id { return root }
        for child in root.children {
            if let match = findModelNode(in: child, id: id) {
                return match
            }
        }
        return nil
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
