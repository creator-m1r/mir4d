import SwiftUI

struct FeatureTreeNode: Identifiable, Hashable {
    let id: UUID
    var title: String
    var icon: String
    var status: FeatureTreeStatus = .ok
    var children: [FeatureTreeNode] = []
    var isSuppressed: Bool = false

    init(id: UUID = UUID(), title: String, icon: String, status: FeatureTreeStatus = .ok, children: [FeatureTreeNode] = [], isSuppressed: Bool = false) {
        self.id = id; self.title = title; self.icon = icon; self.status = status; self.children = children; self.isSuppressed = isSuppressed
    }
}

enum FeatureTreeStatus: Hashable {
    case ok, warning, error, rebuild, suppressed
    var icon: String {
        switch self { case .ok: return "checkmark.circle.fill"; case .warning: return "exclamationmark.triangle.fill"; case .error: return "xmark.circle.fill"; case .rebuild: return "arrow.triangle.2.circlepath"; case .suppressed: return "minus.circle" }
    }
}

struct FeatureTreeView: View {
    @ObservedObject var appState: CADAppState
    @State private var expanded: Set<UUID> = []
    @State private var selectedID: UUID?

    private var roots: [FeatureTreeNode] {
        [
            FeatureTreeNode(title: appState.ui.language == .russian ? "Начало" : "Origin", icon: "cube.transparent"),
            FeatureTreeNode(title: appState.ui.language == .russian ? "Тело" : "Body", icon: "cube.fill", children: [
                FeatureTreeNode(title: "Sketch001", icon: "pencil.and.ruler", status: .ok),
                FeatureTreeNode(title: "Extrude001", icon: "arrow.up.right.square", status: .ok),
                FeatureTreeNode(title: "Sketch002", icon: "pencil.and.ruler", status: .warning),
                FeatureTreeNode(title: "Fillet001", icon: "circle.dashed", status: .rebuild)
            ]),
            FeatureTreeNode(title: appState.ui.language == .russian ? "Конструктивные элементы" : "Construction", icon: "square.dashed")
        ]
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            HStack {
                Label(appState.ui.language == .russian ? "Дерево построения" : "Feature Tree", systemImage: "list.bullet.indent").font(.system(size: 12, weight: .semibold))
                Spacer(); Image(systemName: "ellipsis.circle").foregroundStyle(MirTheme.Colors.textSecondary)
            }.padding(.horizontal, 12).padding(.vertical, 9)
            Divider().opacity(0.35)
            ScrollView {
                LazyVStack(alignment: .leading, spacing: 1) { ForEach(roots) { node in nodeView(node, level: 0) } }.padding(.vertical, 6)
            }
        }.background(MirTheme.Colors.panel)
    }

    private func nodeView(_ node: FeatureTreeNode, level: Int) -> AnyView {
        let hasChildren = !node.children.isEmpty
        let isExpanded = expanded.contains(node.id)
        let row = HStack(spacing: 6) {
            if hasChildren {
                Button {
                    withAnimation(.easeOut(duration: 0.12)) { if isExpanded { expanded.remove(node.id) } else { expanded.insert(node.id) } }
                } label: {
                    Image(systemName: isExpanded ? "chevron.down" : "chevron.right").font(.system(size: 9, weight: .bold)).frame(width: 12)
                }.buttonStyle(.plain)
            } else { Spacer().frame(width: 12) }
            Image(systemName: node.icon).font(.system(size: 12)).foregroundStyle(MirTheme.Colors.accent)
            Text(node.title).font(.system(size: 11)).lineLimit(1)
            Spacer()
            Image(systemName: node.status.icon).font(.system(size: 9)).foregroundStyle(statusColor(node.status))
        }
        .padding(.leading, CGFloat(level * 16)).padding(.trailing, 8).padding(.vertical, 4)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(selectedID == node.id ? MirTheme.Colors.selection.opacity(0.18) : .clear)
        .contentShape(Rectangle())
        .onTapGesture { selectedID = node.id }
        .contextMenu {
            Button(appState.ui.language == .russian ? "Редактировать" : "Edit") { }
            Button(appState.ui.language == .russian ? "Подавить" : "Suppress") { }
            Divider()
            Button(appState.ui.language == .russian ? "Удалить" : "Delete", role: .destructive) { }
        }

        if isExpanded {
            return AnyView(VStack(alignment: .leading, spacing: 0) {
                row
                ForEach(node.children) { child in nodeView(child, level: level + 1).eraseToAnyView() }
            })
        }
        return AnyView(row)
    }

    private func statusColor(_ status: FeatureTreeStatus) -> Color {
        switch status { case .ok: return MirTheme.Colors.success; case .warning: return MirTheme.Colors.warning; case .error: return MirTheme.Colors.error; case .rebuild: return MirTheme.Colors.accent; case .suppressed: return MirTheme.Colors.textTertiary }
    }
}

private extension View { func eraseToAnyView() -> AnyView { AnyView(self) } }
