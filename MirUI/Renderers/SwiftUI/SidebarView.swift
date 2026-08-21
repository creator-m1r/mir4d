import SwiftUI
import AppKit

struct SidebarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var modelRuntime = MIR4DModelRuntime.shared
    @State private var search = ""
    @State private var showTools = false

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        VStack(spacing: 0) {
            header
            searchField
            modelTree
            footer
        }
        .background(MirTheme.Colors.panel)
        .overlay(alignment: .trailing) { Rectangle().fill(MirTheme.Colors.border).frame(width: 1) }
        .sheet(isPresented: $showTools) { navigatorTools }
    }

    private var header: some View {
        HStack(spacing: 8) {
            Image(systemName: "list.bullet.indent")
                .font(.system(size: 12, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)
            VStack(alignment: .leading, spacing: 1) {
                Text(russian ? "Структура" : "Structure")
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Text(russian ? "Инженерная модель" : "Engineering model")
                    .font(MirTheme.Typography.status)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }
            Spacer()
            Menu {
                Button { showTools = true } label: { Label(russian ? "Навигатор и инструменты" : "Navigator tools", systemImage: "slider.horizontal.3") }
                Divider()
                Button { search = "" } label: { Label(russian ? "Очистить поиск" : "Clear search", systemImage: "xmark.circle") }
            } label: {
                Image(systemName: "ellipsis")
                    .font(.system(size: 12, weight: .bold))
                    .frame(width: 28, height: 28)
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                    .background(MirTheme.Colors.surfaceRaised.opacity(0.7), in: RoundedRectangle(cornerRadius: 7))
            }
            .menuStyle(.borderlessButton)
            .help(russian ? "Дополнительные возможности" : "Additional options")
        }
        .padding(.horizontal, MirTheme.Spacing.md)
        .padding(.vertical, 9)
    }

    private var searchField: some View {
        HStack(spacing: 7) {
            Image(systemName: "magnifyingglass").foregroundStyle(MirTheme.Colors.textTertiary)
            TextField(russian ? "Найти в модели…" : "Find in model…", text: $search)
                .textFieldStyle(.plain)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textPrimary)
            if !search.isEmpty {
                Button { search = "" } label: { Image(systemName: "xmark.circle.fill") }
                    .buttonStyle(.plain)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }
            Text("⌘F")
                .font(.system(size: 8, weight: .bold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
        }
        .padding(.horizontal, 9)
        .frame(height: 32)
        .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.medium).stroke(MirTheme.Colors.border, lineWidth: 1))
        .padding(.horizontal, MirTheme.Spacing.md)
        .padding(.vertical, 6)
    }

    private var modelTree: some View {
        ScrollView {
            LazyVStack(alignment: .leading, spacing: 7) {
                ForEach(treeSections) { section in
                    VStack(alignment: .leading, spacing: 2) {
                        sectionHeader(section)
                        ForEach(section.nodes) { node in
                            TreeItemView(node: node, appState: appState, level: 0)
                        }
                    }
                }
                if treeSections.isEmpty { emptyState }
            }
            .padding(.horizontal, 10)
            .padding(.top, 6)
            .padding(.bottom, 12)
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
            TreeSection(id: id, title: title, icon: icon, nodes: visible.filter { predicate($0.kind) }.map(convert))
        }
        return [
            section("bodies", russian ? "Тела" : "Bodies", "cube.transparent", { $0 == .body }),
            section("sketches", russian ? "Эскизы" : "Sketches", "pencil.and.ruler", { $0 == .sketch }),
            section("features", russian ? "Операции" : "Features", "gearshape.2", { $0 == .operation || $0 == .result }),
            section("components", russian ? "Компоненты" : "Components", "square.stack.3d.up", { $0 == .component }),
            section("references", russian ? "Ссылки" : "References", "link", { $0 == .project })
        ].filter { !$0.nodes.isEmpty }
    }

    private func sectionHeader(_ section: TreeSection) -> some View {
        HStack(spacing: 6) {
            Image(systemName: section.icon).font(.system(size: 9, weight: .semibold)).foregroundStyle(MirTheme.Colors.textTertiary)
            Text(section.title.uppercased()).font(.system(size: 9, weight: .semibold)).tracking(0.4).foregroundStyle(MirTheme.Colors.textTertiary)
            Text("\(section.nodes.count)").font(.system(size: 9, weight: .semibold, design: .monospaced)).foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer()
        }
        .padding(.horizontal, 6)
        .padding(.top, 5)
    }

    private func contains(_ node: MIR4DModelNode, query: String) -> Bool {
        node.title.lowercased().contains(query) || node.children.contains { contains($0, query: query) }
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

    private var emptyState: some View {
        VStack(spacing: 8) {
            Image(systemName: search.isEmpty ? "cube.transparent" : "magnifyingglass")
                .font(.system(size: 22, weight: .light))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text(search.isEmpty ? (russian ? "Проект пуст" : "Project is empty") : (russian ? "Ничего не найдено" : "Nothing found"))
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
            Text(search.isEmpty ? (russian ? "Создайте тело или эскиз, чтобы начать работу" : "Create a body or sketch to begin") : (russian ? "Попробуйте другое название" : "Try another name"))
                .font(MirTheme.Typography.status)
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .multilineTextAlignment(.center)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 34)
    }

    private var footer: some View {
        HStack(spacing: 6) {
            Circle().fill(MirTheme.Colors.success).frame(width: 6, height: 6)
            Text(russian ? "Модель готова" : "Model ready")
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer()
            Text("v\(modelRuntime.revision)")
                .font(.system(size: 10, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
        }
        .padding(.horizontal, MirTheme.Spacing.md)
        .padding(.vertical, 9)
    }

    private var navigatorTools: some View {
        VStack(alignment: .leading, spacing: 16) {
            Text(russian ? "Инструменты навигатора" : "Navigator tools")
                .font(MirTheme.Typography.title)
            Text(russian ? "Дополнительные представления вынесены сюда, чтобы основная структура проекта оставалась чистой." : "Secondary views live here so the main project structure stays clean.")
                .font(MirTheme.Typography.body)
                .foregroundStyle(MirTheme.Colors.textSecondary)
            VStack(alignment: .leading, spacing: 8) {
                Label(russian ? "Фильтры модели" : "Model filters", systemImage: "line.3.horizontal.decrease.circle")
                Label(russian ? "Слои и видимость" : "Layers and visibility", systemImage: "square.3.layers.3d")
                Label(russian ? "Ссылки и зависимости" : "References and dependencies", systemImage: "link")
            }
            .font(MirTheme.Typography.body)
            Spacer()
        }
        .padding(24)
        .frame(width: 360, height: 280, alignment: .topLeading)
    }
}
