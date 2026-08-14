import SwiftUI
import AppKit
import UniformTypeIdentifiers

struct SidebarView: View {
    @ObservedObject var appState: CADAppState
    @State private var search = ""
    @State private var activeSection = 0

    private var sections: [String] {
        appState.ui.language == .russian
            ? ["Дерево", "Фильтр", "Слои"]
            : ["Tree", "Filter", "Layers"]
    }

    var body: some View {
        VStack(spacing: 0) {
            searchField
            sectionPicker

            switch activeSection {
            case 0:
                modelTree
            case 1:
                filterList
            default:
                layerList
            }

            Spacer(minLength: 0)

            footer
        }
        .background(MirTheme.Colors.panel)
        .overlay(alignment: .trailing) {
            Rectangle()
                .fill(MirTheme.Colors.border)
                .frame(width: 1)
        }
    }

    private var searchField: some View {
        HStack(spacing: 7) {
            Image(systemName: "magnifyingglass")
                .foregroundStyle(MirTheme.Colors.textTertiary)

            TextField(
                appState.ui.language == .russian ? "Поиск по модели…" : "Search model…",
                text: $search
            )
            .textFieldStyle(.plain)
            .font(MirTheme.Typography.caption)
            .foregroundStyle(MirTheme.Colors.textPrimary)

            if !search.isEmpty {
                Button { search = "" } label: {
                    Image(systemName: "xmark.circle.fill")
                }
                .buttonStyle(.plain)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            }
        }
        .padding(10)
        .background(MirTheme.Colors.surfaceRaised.opacity(0.72))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .padding(MirTheme.Spacing.md)
    }

    private var sectionPicker: some View {
        HStack(spacing: 4) {
            ForEach(sections.indices, id: \.self) { index in
                Button(sections[index]) {
                    withAnimation(MirTheme.Animation.fast) {
                        activeSection = index
                    }
                }
                .buttonStyle(.plain)
                .font(MirTheme.Typography.caption)
                .padding(.vertical, 6)
                .frame(maxWidth: .infinity)
                .background(
                    activeSection == index
                        ? MirTheme.Colors.accentSoft
                        : Color.clear
                )
                .foregroundStyle(
                    activeSection == index
                        ? MirTheme.Colors.accentBright
                        : MirTheme.Colors.textTertiary
                )
                .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            }
        }
        .padding(4)
        .background(MirTheme.Colors.surface.opacity(0.72))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .padding(.horizontal, MirTheme.Spacing.md)
    }

    private var modelTree: some View {
        ScrollView {
            LazyVStack(alignment: .leading, spacing: 1) {
                ForEach(filteredTreeData) { node in
                    TreeItemView(node: node, appState: appState, level: 0)
                }
            }
            .padding(.horizontal, 10)
            .padding(.top, MirTheme.Spacing.md)
        }
    }

    private var filteredTreeData: [TreeNodeData] {
        let query = search.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
        guard !query.isEmpty else { return appState.treeData }
        return appState.treeData.filter { contains($0, query: query) }
    }

    private func contains(_ node: TreeNodeData, query: String) -> Bool {
        if node.name.lowercased().contains(query) { return true }
        return node.children.contains { contains($0, query: query) }
    }

    private var layerList: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 6) {
                layerRow(localized("Геометрия", "Geometry"), true)
                layerRow(localized("Эскизы", "Sketches"), true)
                layerRow(localized("Сетки", "Meshes"), appState.gridVisible)
                layerRow(localized("Оси", "Axes"), appState.axesVisible)
                layerRow(localized("Сечения", "Sections"), appState.sectionMode)
                layerRow(localized("Инженерные данные", "Engineering data"), true)
                layerRow(localized("Симуляции", "Simulations"), appState.workbench == .simulation)
                layerRow(localized("Временные события", "Time events"), appState.workbench == .fourD)
            }
            .padding(MirTheme.Spacing.md)
        }
    }

    private var filterList: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 10) {
                Text(localized("ФИЛЬТР ОБЪЕКТОВ", "OBJECT FILTER"))
                    .font(.system(size: 9, weight: .semibold))
                    .foregroundStyle(MirTheme.Colors.textTertiary)
                    .tracking(0.5)

                filterButton(localized("Тела", "Bodies"), "cube")
                filterButton(localized("Компоненты", "Components"), "square.stack.3d.up")
                filterButton(localized("Эскизы", "Sketches"), "pencil.and.ruler")
                filterButton(localized("Конструктивные элементы", "Construction geometry"), "point.3.connected.trianglepath.dotted")
                filterButton(localized("Результаты расчёта", "Simulation results"), "waveform.path.ecg")
            }
            .padding(MirTheme.Spacing.md)
        }
    }

    private func layerRow(_ title: String, _ enabled: Bool) -> some View {
        HStack {
            Image(systemName: enabled ? "eye" : "eye.slash")
                .foregroundStyle(enabled ? MirTheme.Colors.accent : MirTheme.Colors.textTertiary)
            Text(title)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textPrimary)
            Spacer()
        }
        .padding(.vertical, 5)
    }

    private func filterButton(_ title: String, _ icon: String) -> some View {
        Button {
            appState.showNotification(
                localized("Фильтр: \(title)", "Filter: \(title)"),
                type: .info
            )
        } label: {
            Label(title, systemImage: icon)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
        }
        .buttonStyle(.plain)
    }

    private var footer: some View {
        VStack(spacing: 8) {
            Button { openModelPanel() } label: {
                Label(
                    localized("Импорт 3D-модели", "Import 3D model"),
                    systemImage: "square.and.arrow.down"
                )
                .font(MirTheme.Typography.bodyMedium)
                .frame(maxWidth: .infinity)
                .padding(.vertical, 10)
            }
            .buttonStyle(.plain)
            .background(MirTheme.Colors.accentSoft)
            .foregroundStyle(MirTheme.Colors.accentBright)
            .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.large))

            HStack(spacing: 6) {
                Circle()
                    .fill(MirTheme.Colors.success)
                    .frame(width: 6, height: 6)
                Text(localized("Документ синхронизирован", "Document synchronized"))
                Spacer()
            }
            .font(MirTheme.Typography.status)
            .foregroundStyle(MirTheme.Colors.textTertiary)
        }
        .padding(MirTheme.Spacing.md)
    }

    private func localized(_ ru: String, _ en: String) -> String {
        appState.ui.language == .russian ? ru : en
    }

    private func openModelPanel() {
        let panel = NSOpenPanel()
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        panel.allowedContentTypes = [
            UTType(filenameExtension: "obj"),
            UTType(filenameExtension: "stl"),
            UTType(filenameExtension: "gltf"),
            UTType(filenameExtension: "glb"),
            UTType(filenameExtension: "fbx"),
            UTType(filenameExtension: "step"),
            UTType(filenameExtension: "stp"),
            UTType(filenameExtension: "iges"),
            UTType(filenameExtension: "igs")
        ].compactMap { $0 }

        panel.begin { response in
            if response == .OK, let url = panel.url {
                appState.importModel(url: url)
            }
        }
    }
}
