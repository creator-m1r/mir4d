import SwiftUI

/// Reusable viewport interaction HUD. Keeps presentation state local and emits
/// neutral notifications so the rendering/engine layers remain decoupled.
struct CADViewportInteractionOverlay: View {
    @ObservedObject var appState: CADAppState
    @State private var selectionMode: SelectionMode = .body
    @State private var snapEnabled = true
    @State private var gridEnabled = true
    @State private var coordinates = SIMD3<Double>(0, 0, 0)

    private enum SelectionMode: String, CaseIterable {
        case vertex, edge, face, body
        var icon: String {
            switch self {
            case .vertex: return "circle.fill"
            case .edge: return "line.diagonal"
            case .face: return "square.fill"
            case .body: return "cube.fill"
            }
        }
        func title(russian: Bool) -> String {
            switch self {
            case .vertex: return russian ? "Вершина" : "Vertex"
            case .edge: return russian ? "Ребро" : "Edge"
            case .face: return russian ? "Грань" : "Face"
            case .body: return russian ? "Тело" : "Body"
            }
        }
    }

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        VStack {
            HStack(spacing: 5) {
                selectionMenu
                Divider().frame(height: 20)
                modeButton("scope", russian ? "Выбор" : "Selection") {
                    NotificationCenter.default.post(name: .mir4DSelectionScopeRequested, object: nil)
                }
                modeButton("dot.squareshape.split.2x2", russian ? "Привязка" : "Snap", active: snapEnabled) {
                    snapEnabled.toggle()
                    NotificationCenter.default.post(name: .mir4DSnapChanged, object: nil, userInfo: ["enabled": snapEnabled])
                }
                modeButton("grid", russian ? "Сетка" : "Grid", active: gridEnabled) {
                    gridEnabled.toggle()
                    appState.toggleGrid()
                }
                Spacer(minLength: 12)
                coordinateReadout
            }
            .padding(.horizontal, 8)
            .frame(height: 32)
            .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.border, lineWidth: 1))
            .padding(8)
            Spacer()
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    private var selectionMenu: some View {
        Menu {
            ForEach(SelectionMode.allCases, id: \.self) { mode in
                Button {
                    selectionMode = mode
                    NotificationCenter.default.post(name: .mir4DSelectionModeChanged, object: nil, userInfo: ["mode": mode.rawValue])
                } label: {
                    Label(mode.title(russian: russian), systemImage: mode.icon)
                }
            }
        } label: {
            Label(selectionMode.title(russian: russian), systemImage: selectionMode.icon)
                .font(.system(size: 9, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .padding(.horizontal, 7)
                .frame(height: 23)
                .background(MirTheme.Colors.surface, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .menuStyle(.borderlessButton)
    }

    private func modeButton(_ icon: String, _ title: String, active: Bool = true, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            HStack(spacing: 4) {
                Image(systemName: icon)
                Text(title)
            }
            .font(.system(size: 9, weight: .medium))
            .foregroundStyle(active ? MirTheme.Colors.accentBright : MirTheme.Colors.textTertiary)
            .padding(.horizontal, 7)
            .frame(height: 23)
            .background((active ? MirTheme.Colors.accentSoft : MirTheme.Colors.surface).opacity(0.8), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain)
        .help(title)
    }

    private var coordinateReadout: some View {
        Text(String(format: "X %+.3f   Y %+.3f   Z %+.3f", coordinates.x, coordinates.y, coordinates.z))
            .font(.system(size: 9, weight: .medium, design: .monospaced))
            .foregroundStyle(MirTheme.Colors.textTertiary)
    }
}

/// Persistent bottom status surface for the CAD workspace.
struct CADWorkspaceStatusBar: View {
    @ObservedObject var appState: CADAppState

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        HStack(spacing: 12) {
            statusDot
            Divider().frame(height: 13)
            Text(russian ? "Объектов: \(appState.treeData.count)" : "Objects: \(appState.treeData.count)")
            Text(russian ? "Выбрано: \(appState.selection.ids.count)" : "Selected: \(appState.selection.ids.count)")
            Spacer()
            Text(russian ? appState.workbench.titleRU : appState.workbench.titleEN)
            Text("•")
            Text("MIR 4D")
                .fontWeight(.semibold)
                .foregroundStyle(MirTheme.Colors.accentBright)
        }
        .font(.system(size: 9, weight: .medium, design: .monospaced))
        .foregroundStyle(MirTheme.Colors.textTertiary)
        .padding(.horizontal, 10)
        .frame(height: 23)
        .background(MirTheme.Colors.topBar)
        .overlay(alignment: .top) { Rectangle().fill(MirTheme.Colors.border).frame(height: 1) }
    }

    private var statusDot: some View {
        HStack(spacing: 5) {
            Circle()
                .fill(appState.documentDirty ? MirTheme.Colors.warning : MirTheme.Colors.success)
                .frame(width: 6, height: 6)
            Text(appState.documentDirty ? (russian ? "Изменено" : "Modified") : (russian ? "Готово" : "Ready"))
                .foregroundStyle(MirTheme.Colors.textSecondary)
        }
    }
}
