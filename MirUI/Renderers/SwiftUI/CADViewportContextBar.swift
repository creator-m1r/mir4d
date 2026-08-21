import SwiftUI

/// Context controls over the viewport. Presentation only; engine actions are
/// routed through the existing event/notification interfaces.
struct CADViewportContextBar: View {
    @ObservedObject var appState: CADAppState
    @State private var selectionMode: SelectionMode = .body
    @State private var snapEnabled = true
    @State private var gridEnabled = true
    @State private var coordinates = "X 0.000   Y 0.000   Z 0.000"

    private enum SelectionMode: String, CaseIterable {
        case vertex, edge, face, body
        var icon: String {
            switch self { case .vertex: return "circle.fill"; case .edge: return "line.diagonal"; case .face: return "square.fill"; case .body: return "cube.fill" }
        }
        func title(_ ru: Bool) -> String {
            switch self { case .vertex: return ru ? "Вершина" : "Vertex"; case .edge: return ru ? "Ребро" : "Edge"; case .face: return ru ? "Грань" : "Face"; case .body: return ru ? "Тело" : "Body" }
        }
    }

    private var ru: Bool { appState.ui.language == .russian }

    var body: some View {
        VStack(spacing: 0) {
            HStack(spacing: 5) {
                selectionMenu
                Divider().frame(height: 22)
                compactToggle("scope", ru ? "Только выбор" : "Selection scope", enabled: true) {}
                compactToggle("dot.squareshape.split.2x2", ru ? "Привязка" : "Snap", enabled: snapEnabled) { snapEnabled.toggle() }
                compactToggle("grid", ru ? "Сетка" : "Grid", enabled: gridEnabled) { gridEnabled.toggle(); appState.toggleGrid() }
                Spacer()
                Text(coordinates).font(.system(size: 9, weight: .medium, design: .monospaced)).foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .padding(.horizontal, 9).frame(height: 34)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.94), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border, lineWidth: 1))
            .padding(8)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .bottom)
        .allowsHitTesting(true)
    }

    private var selectionMenu: some View {
        Menu {
            ForEach(SelectionMode.allCases, id: \.self) { mode in
                Button { selectionMode = mode } label: { Label(mode.title(ru), systemImage: mode.icon) }
            }
        } label: {
            Label(selectionMode.title(ru), systemImage: selectionMode.icon)
                .font(.system(size: 9, weight: .semibold)).foregroundStyle(MirTheme.Colors.textSecondary)
                .padding(.horizontal, 7).frame(height: 24)
                .background(MirTheme.Colors.surface, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .menuStyle(.borderlessButton)
    }

    private func compactToggle(_ icon: String, _ title: String, enabled: Bool, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            HStack(spacing: 4) {
                Image(systemName: icon)
                Text(title)
            }
            .font(.system(size: 9, weight: .medium))
            .foregroundStyle(enabled ? MirTheme.Colors.accentBright : MirTheme.Colors.textTertiary)
            .padding(.horizontal, 7).frame(height: 24)
            .background((enabled ? MirTheme.Colors.accentSoft : MirTheme.Colors.surface).opacity(0.8), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }.buttonStyle(.plain).help(title)
    }
}

struct CADStatusBar: View {
    @ObservedObject var appState: CADAppState
    @State private var memory = "—"

    private var ru: Bool { appState.ui.language == .russian }

    var body: some View {
        HStack(spacing: 12) {
            status("circle.fill", appState.documentDirty ? (ru ? "Изменено" : "Modified") : (ru ? "Готово" : "Ready"), appState.documentDirty ? MirTheme.Colors.warning : MirTheme.Colors.success)
            Divider().frame(height: 14)
            Text(ru ? "Объектов: \(appState.treeData.count)" : "Objects: \(appState.treeData.count)").foregroundStyle(MirTheme.Colors.textTertiary)
            Text(ru ? "Выбрано: \(appState.selection.ids.count)" : "Selected: \(appState.selection.ids.count)").foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer()
            Text(appState.workbench.titleRU).foregroundStyle(MirTheme.Colors.textSecondary)
            Text("•").foregroundStyle(MirTheme.Colors.textDisabled)
            Text(memory).foregroundStyle(MirTheme.Colors.textTertiary)
            Text("MIR 4D").fontWeight(.semibold).foregroundStyle(MirTheme.Colors.accentBright)
        }
        .font(.system(size: 9, weight: .medium, design: .monospaced))
        .padding(.horizontal, 10).frame(height: 23).background(MirTheme.Colors.topBar)
        .overlay(alignment: .top) { Rectangle().fill(MirTheme.Colors.border).frame(height: 1) }
        .onAppear { memory = "UI" }
    }

    private func status(_ icon: String, _ text: String, _ color: Color) -> some View {
        HStack(spacing: 5) { Image(systemName: icon).foregroundStyle(color); Text(text).foregroundStyle(MirTheme.Colors.textSecondary) }
    }
}
