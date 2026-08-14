import SwiftUI
import AppKit
import UniformTypeIdentifiers

struct SidebarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var modelRuntime = MIR4DModelRuntime.shared
    @State private var search = ""
    @State private var activeSection = 0

    private var sections: [String] { appState.ui.language == .russian ? ["Дерево", "Фильтр", "Слои"] : ["Tree", "Filter", "Layers"] }

    var body: some View {
        VStack(spacing: 0) {
            searchField
            sectionPicker
            switch activeSection { case 0: modelTree; case 1: filterList; default: layerList }
            Spacer(minLength: 0)
            footer
        }
        .background(MirTheme.Colors.panel)
        .overlay(alignment: .trailing) { Rectangle().fill(MirTheme.Colors.border).frame(width: 1) }
    }

    private var searchField: some View {
        HStack(spacing: 7) {
            Image(systemName: "magnifyingglass").foregroundStyle(MirTheme.Colors.textTertiary)
            TextField(appState.ui.language == .russian ? "Поиск по модели…" : "Search model…", text: $search)
                .textFieldStyle(.plain).font(MirTheme.Typography.caption).foregroundStyle(MirTheme.Colors.textPrimary)
            if !search.isEmpty { Button { search = "" } label: { Image(systemName: "xmark.circle.fill") }.buttonStyle(.plain).foregroundStyle(MirTheme.Colors.textTertiary) }
        }
        .padding(10).background(MirTheme.Colors.surfaceRaised.opacity(0.72))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium)).padding(MirTheme.Spacing.md)
    }

    private var sectionPicker: some View {
        HStack(spacing: 4) {
            ForEach(sections.indices, id: \.self) { index in
                Button(sections[index]) { withAnimation(MirTheme.Animation.fast) { activeSection = index } }
                    .buttonStyle(.plain).font(MirTheme.Typography.caption).padding(.vertical, 6).frame(maxWidth: .infinity)
                    .background(activeSection == index ? MirTheme.Colors.accentSoft : Color.clear)
                    .foregroundStyle(activeSection == index ? MirTheme.Colors.accentBright : MirTheme.Colors.textTertiary)
                    .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            }
        }
        .padding(4).background(MirTheme.Colors.surface.opacity(0.72))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium)).padding(.horizontal, MirTheme.Spacing.md)
    }

    private var modelTree: some View {
        ScrollView {
            LazyVStack(alignment: .leading, spacing: MirTheme.Spacing.sm) {
                ForEach(treeSections) { section in
                    VStack(alignment: .leading, spacing: 1) {
                        sectionHeader(section)
                        ForEach(section.nodes) { node in
                            TreeItemView(node: node, appState: appState, level: 0)
                        }
                    }
                }
            }
            .padding(.horizontal, 10).padding(.top, MirTheme.Spacing.md)
        }
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
            section("bodies", localized("Тела", "Bodies"), "cube.transparent", { $0 == .body }),
            section("sketches", localized("Эскизы", "Sketches"), "pencil.and.ruler", { $0 == .sketch }),
            section("features", localized("Функции", "Features"), "gearshape.2", { $0 == .operation || $0 == .result }),
            section("construction", localized("Конструктив", "Construction"), "square.stack.3d.up", { $0 == .component }),
            section("references", localized("Ссылки", "References"), "link", { $0 == .project })
        ].filter { !$0.nodes.isEmpty }
    }

    private func sectionHeader(_ section: TreeSection) -> some View {
        HStack(spacing: 6) {
            Image(systemName: section.icon)
                .font(.system(size: 9, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text(section.title.uppercased())
                .font(.system(size: 9, weight: .semibold))
                .tracking(0.4)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer()
        }
        .padding(.horizontal, 6)
        .padding(.top, MirTheme.Spacing.xs)
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

        // Preserve the persisted model UUID. Tree rows must not invent a
        // second identity layer for CAD objects.
        return TreeNodeData(
            id: node.id,
            name: node.title,
            icon: icon,
            children: node.children.map(convert)
        )
    }

    private var layerList: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 6) {
                layerRow(localized("Геометрия", "Geometry"), true)
                layerRow(localized("Эскизы", "Sketches"), true)
                layerRow(localized("Сетки", "Meshes"), appState.gridVisible)
                layerRow(localized("Оси", "Axes"), appState.axesVisible)
                layerRow(localized("Сечения", "Sections"), appState.sectionMode)
            }
            .padding(MirTheme.Spacing.md)
        }
    }

    private var filterList: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 8) {
                Text(localized("Фильтры модели", "Model Filters"))
                    .font(MirTheme.Typography.caption)
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                Text(localized("Фильтрация дерева модели", "Model tree filtering"))
                    .font(MirTheme.Typography.body)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
            }
            .frame(maxWidth: .infinity, alignment: .leading)
            .padding(MirTheme.Spacing.md)
        }
    }

    private func layerRow(_ title: String, _ visible: Bool) -> some View {
        HStack {
            Text(title)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textPrimary)
            Spacer()
            Image(systemName: visible ? "eye" : "eye.slash")
                .foregroundStyle(visible ? MirTheme.Colors.accentBright : MirTheme.Colors.textTertiary)
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
        .background(MirTheme.Colors.surface.opacity(0.45))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
    }

    private var footer: some View {
        HStack {
            Text(localized("Модель", "Model"))
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer()
            Text("v\(modelRuntime.revision)")
                .font(.system(size: 10, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
        }
        .padding(MirTheme.Spacing.md)
    }

    private func localized(_ russian: String, _ english: String) -> String {
        appState.ui.language == .russian ? russian : english
    }
}
