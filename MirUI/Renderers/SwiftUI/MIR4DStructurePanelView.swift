import SwiftUI

/// Primary CAD structure panel.
/// Keeps the model tree focused on the engineering task: find, select, and inspect.
struct MIR4DStructurePanelView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var modelRuntime = MIR4DModelRuntime.shared
    @State private var search = ""
    @State private var showLayers = false

    private var ru: Bool { appState.ui.language == .russian }

    var body: some View {
        VStack(spacing: 0) {
            searchBar
            tree
            Divider().overlay(MirTheme.Colors.border)
            quickBar
        }
        .background(MirTheme.Colors.panel)
    }

    private var searchBar: some View {
        HStack(spacing: 8) {
            Image(systemName: "magnifyingglass")
                .foregroundStyle(MirTheme.Colors.textTertiary)
            TextField(ru ? "Найти в модели…" : "Find in model…", text: $search)
                .textFieldStyle(.plain)
                .font(MirTheme.Typography.caption)
            if !search.isEmpty {
                Button { search = "" } label: {
                    Image(systemName: "xmark.circle.fill")
                }
                .buttonStyle(.plain)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            }
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 8)
        .background(MirTheme.Colors.surfaceRaised)
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay {
            RoundedRectangle(cornerRadius: MirTheme.Radius.medium)
                .stroke(MirTheme.Colors.border, lineWidth: 1)
        }
        .padding(10)
    }

    private var tree: some View {
        ScrollView {
            LazyVStack(alignment: .leading, spacing: 4) {
                let sections = treeSections
                if sections.isEmpty {
                    emptyState
                } else {
                    ForEach(sections) { section in
                        sectionHeader(section)
                        ForEach(section.nodes) { node in
                            TreeItemView(node: node, appState: appState, level: 0)
                        }
                    }
                }
            }
            .padding(.horizontal, 8)
            .padding(.bottom, 12)
        }
        .scrollIndicators(.hidden)
        .animation(MirTheme.Animation.normal, value: modelRuntime.revision)
    }

    private struct TreeSection: Identifiable {
        let id: String
        let title: String
        let icon: String
        let nodes: [TreeNodeData]
    }

    private var treeSections: [TreeSection] {
        let children = modelRuntime.document.root.children
        let query = search.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
        let visible = query.isEmpty ? children : children.filter { contains($0, query: query) }

        func section(_ id: String, _ title: String, _ icon: String, _ predicate: (MIR4DModelNode.Kind) -> Bool) -> TreeSection {
            TreeSection(
                id: id,
                title: title,
                icon: icon,
                nodes: visible.filter { predicate($0.kind) }.map(convert)
            )
        }

        return [
            section("bodies", ru ? "Тела" : "Bodies", "cube.transparent") { $0 == .body },
            section("sketches", ru ? "Эскизы" : "Sketches", "pencil.and.ruler") { $0 == .sketch },
            section("features", ru ? "Операции" : "Features", "gearshape.2") { $0 == .operation || $0 == .result },
            section("components", ru ? "Компоненты" : "Components", "square.stack.3d.up") { $0 == .component },
            section("references", ru ? "Проект" : "Project", "cube") { $0 == .project }
        ].filter { !$0.nodes.isEmpty }
    }

    private func sectionHeader(_ section: TreeSection) -> some View {
        HStack(spacing: 6) {
            Image(systemName: section.icon)
                .font(.system(size: 9, weight: .semibold))
            Text(section.title)
                .font(.system(size: 9, weight: .semibold))
                .tracking(0.3)
            Text("\(section.nodes.count)")
                .font(.system(size: 9, weight: .medium, design: .monospaced))
            Spacer()
        }
        .foregroundStyle(MirTheme.Colors.textTertiary)
        .padding(.horizontal, 7)
        .padding(.top, 8)
        .padding(.bottom, 2)
    }

    private var emptyState: some View {
        VStack(spacing: 8) {
            Image(systemName: search.isEmpty ? "cube.transparent" : "magnifyingglass")
                .font(.system(size: 24, weight: .light))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text(search.isEmpty ? (ru ? "Модель пуста" : "Model is empty") : (ru ? "Ничего не найдено" : "Nothing found"))
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
            if search.isEmpty {
                Text(ru ? "Создайте тело или эскиз, чтобы начать." : "Create a body or sketch to begin.")
                    .font(MirTheme.Typography.status)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
                    .multilineTextAlignment(.center)
            }
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 36)
    }

    private var quickBar: some View {
        HStack(spacing: 6) {
            quickAction("plus", ru ? "Создать" : "Create") {
                _ = MIR4DModelCommands.shared.createBox(appState: appState, width: 40, depth: 40, height: 40)
            }
            quickAction("square.3.layers.3d", ru ? "Слои" : "Layers") {
                withAnimation(MirTheme.Animation.fast) { showLayers.toggle() }
            }
            Spacer()
            Text(ru ? "\(modelRuntime.document.root.children.count) объекта" : "\(modelRuntime.document.root.children.count) objects")
                .font(.system(size: 9, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
        }
        .padding(8)
        .background(MirTheme.Colors.surface.opacity(0.8))
        .overlay(alignment: .top) { Rectangle().fill(MirTheme.Colors.border).frame(height: 1) }
        .popover(isPresented: $showLayers, arrowEdge: .bottom) {
            layerPopover
        }
    }

    private func quickAction(_ icon: String, _ title: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Label(title, systemImage: icon)
                .font(.system(size: 10, weight: .medium))
        }
        .buttonStyle(.borderless)
        .padding(.horizontal, 7)
        .padding(.vertical, 5)
        .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
    }

    private var layerPopover: some View {
        VStack(alignment: .leading, spacing: 10) {
            Text(ru ? "Видимость" : "Visibility")
                .font(.headline)
            Text(ru ? "Слои отображения управляются из состояния проекта." : "Display layers are controlled by project state.")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .padding(14)
        .frame(width: 260)
    }

    private func contains(_ node: MIR4DModelNode, query: String) -> Bool {
        if node.title.lowercased().contains(query) { return true }
        return node.children.contains { contains($0, query: query) }
    }

    private func convert(_ node: MIR4DModelNode) -> TreeNodeData {
        let icon: String
        switch node.kind {
        case .project: icon = "cube"
        case .component: icon = "square.stack.3d.up"
        case .body: icon = "cube.transparent"
        case .sketch: icon = "pencil.and.ruler"
        case .operation: icon = "arrow.up.right"
        case .result: icon = "square.3d.up"
        }
        return TreeNodeData(id: node.id, name: node.title, icon: icon, children: node.children.map(convert))
    }
}
