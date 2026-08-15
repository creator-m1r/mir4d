import SwiftUI

struct SidebarView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var modelRuntime = MIR4DModelRuntime.shared
    @State private var search = ""
    @State private var activeSection = 0

    private var sections: [String] {
        appState.ui.language == .russian ? ["Дерево", "Фильтр", "Слои"] : ["Tree", "Filter", "Layers"]
    }

    var body: some View {
        VStack(spacing: 0) {
            searchField
            sectionPicker
            Group {
                switch activeSection {
                case 0: modelTree
                case 1: filterList
                default: layerList
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            footer
        }
        .background(MirTheme.Colors.panel)
    }

    private var searchField: some View {
        HStack(spacing: 7) {
            Image(systemName: "magnifyingglass")
                .foregroundStyle(MirTheme.Colors.textTertiary)
            TextField(localized("Поиск по модели…", "Search model…"), text: $search)
                .textFieldStyle(.plain)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textPrimary)
                .onSubmit { activeSection = 0 }
            if !search.isEmpty {
                Button { search = "" } label: {
                    Image(systemName: "xmark.circle.fill")
                }
                .buttonStyle(.plain)
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .accessibilityLabel(localized("Очистить поиск", "Clear search"))
            }
        }
        .padding(9)
        .background(MirTheme.Colors.surfaceRaised)
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.medium).stroke(MirTheme.Colors.border, lineWidth: 1))
        .padding(.horizontal, MirTheme.Spacing.md)
        .padding(.vertical, 8)
    }

    private var sectionPicker: some View {
        HStack(spacing: 3) {
            ForEach(sections.indices, id: \.self) { index in
                Button(sections[index]) {
                    withAnimation(MirTheme.Animation.fast) { activeSection = index }
                }
                .buttonStyle(.plain)
                .font(MirTheme.Typography.caption)
                .padding(.vertical, 6)
                .frame(maxWidth: .infinity)
                .background(activeSection == index ? MirTheme.Colors.accentSoft : Color.clear)
                .foregroundStyle(activeSection == index ? MirTheme.Colors.accentBright : MirTheme.Colors.textTertiary)
                .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
                .accessibilityAddTraits(activeSection == index ? .isSelected : [])
            }
        }
        .padding(4)
        .background(MirTheme.Colors.surface)
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.medium).stroke(MirTheme.Colors.border, lineWidth: 1))
        .padding(.horizontal, MirTheme.Spacing.md)
        .padding(.bottom, 6)
    }

    private var modelTree: some View {
        ScrollView {
            LazyVStack(alignment: .leading, spacing: 8) {
                ForEach(treeSections) { section in
                    VStack(alignment: .leading, spacing: 2) {
                        sectionHeader(section)
                        ForEach(section.nodes) { node in
                            TreeItemView(node: node, appState: appState, level: 0)
                        }
                    }
                }
                if treeSections.isEmpty { emptyTreeState }
            }
            .padding(.horizontal, 10)
            .padding(.top, 6)
            .padding(.bottom, 10)
        }
        .scrollIndicators(.automatic)
        .animation(MirTheme.Animation.normal, value: modelRuntime.revision)
    }

    private var emptyTreeState: some View {
        VStack(spacing: 8) {
            Image(systemName: search.isEmpty ? "cube.transparent" : "magnifyingglass")
                .font(.system(size: 22, weight: .light))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text(search.isEmpty ? localized("Проект пуст", "Project is empty") : localized("Ничего не найдено", "Nothing found"))
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
            if search.isEmpty {
                Text(localized("Создайте тело или эскиз, чтобы начать работу", "Create a body or sketch to begin"))
                    .font(MirTheme.Typography.status)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
                    .multilineTextAlignment(.center)
            }
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 34)
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
            Text("\(section.nodes.count)")
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer()
        }
        .padding(.horizontal, 6)
        .padding(.top, 5)
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

    private var layerList: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 6) {
                layerInfoRow(localized("Геометрия", "Geometry"), "cube.transparent", true)
                layerInfoRow(localized("Эскизы", "Sketches"), "pencil.and.ruler", true)
                layerToggle(localized("Сетка", "Grid"), "grid", appState.gridVisible) {
                    appState.toggleGrid()
                }
                layerToggle(localized("Оси", "Axes"), "axis.3d", appState.axesVisible) {
                    appState.toggleAxes()
                }
                layerToggle(localized("Сечения", "Sections"), "scissors", appState.sectionMode) {
                    appState.toggleSection()
                }
            }
            .padding(MirTheme.Spacing.md)
        }
        .scrollIndicators(.automatic)
    }

    private func layerInfoRow(_ title: String, _ icon: String, _ visible: Bool) -> some View {
        HStack(spacing: 8) {
            Image(systemName: icon)
                .font(.system(size: 11, weight: .medium))
                .foregroundStyle(MirTheme.Colors.accentBright)
            Text(title)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textPrimary)
            Spacer()
            Text(localized("активно", "active"))
                .font(MirTheme.Typography.status)
                .foregroundStyle(MirTheme.Colors.textTertiary)
        }
        .padding(.horizontal, 9)
        .padding(.vertical, 7)
        .background(MirTheme.Colors.surface)
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
    }

    private func layerToggle(_ title: String, _ icon: String, _ visible: Bool, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            HStack(spacing: 8) {
                Image(systemName: visible ? "eye" : "eye.slash")
                    .font(.system(size: 11, weight: .medium))
                    .foregroundStyle(visible ? MirTheme.Colors.accentBright : MirTheme.Colors.textTertiary)
                Image(systemName: icon)
                    .font(.system(size: 10, weight: .medium))
                    .foregroundStyle(MirTheme.Colors.textTertiary)
                Text(title)
                    .font(MirTheme.Typography.caption)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Spacer()
                Text(visible ? localized("ВКЛ", "ON") : localized("ВЫКЛ", "OFF"))
                    .font(.system(size: 9, weight: .semibold, design: .monospaced))
                    .foregroundStyle(visible ? MirTheme.Colors.success : MirTheme.Colors.textTertiary)
            }
            .padding(.horizontal, 9)
            .padding(.vertical, 7)
            .background(visible ? MirTheme.Colors.surfaceRaised : MirTheme.Colors.surface)
            .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border, lineWidth: 1))
        }
        .buttonStyle(.plain)
        .help(visible ? localized("Скрыть слой", "Hide layer") : localized("Показать слой", "Show layer"))
    }

    private var filterList: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 10) {
                Label(localized("Фильтры модели", "Model Filters"), systemImage: "line.3.horizontal.decrease.circle")
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Text(localized("Поиск выше фильтрует дерево по имени объекта и его дочерним элементам.", "The search above filters the tree by object name and descendants."))
                    .font(MirTheme.Typography.status)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
                    .fixedSize(horizontal: false, vertical: true)
                if !search.isEmpty {
                    Button {
                        search = ""
                        activeSection = 0
                    } label: {
                        Label(localized("Сбросить фильтр", "Clear filter"), systemImage: "xmark.circle")
                    }
                    .buttonStyle(.bordered)
                    .controlSize(.small)
                }
            }
            .frame(maxWidth: .infinity, alignment: .leading)
            .padding(MirTheme.Spacing.md)
        }
    }

    private var footer: some View {
        HStack(spacing: 6) {
            Circle()
                .fill(MirTheme.Colors.success)
                .frame(width: 6, height: 6)
            Text(localized("Модель", "Model"))
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer()
            Text("v\(modelRuntime.revision)")
                .font(.system(size: 10, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
        }
        .padding(.horizontal, MirTheme.Spacing.md)
        .padding(.vertical, 9)
        .background(MirTheme.Colors.panelRaised)
        .overlay(alignment: .top) {
            Rectangle().fill(MirTheme.Colors.border).frame(height: 1)
        }
    }

    private func localized(_ russian: String, _ english: String) -> String {
        appState.ui.language == .russian ? russian : english
    }
}
