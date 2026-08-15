import SwiftUI

struct SelectionIdentityInspector: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var runtime = MIR4DModelRuntime.shared

    var body: some View {
        VStack(alignment: .leading, spacing: 9) {
            HStack(spacing: 8) {
                Image(systemName: appState.selection.hasSelection ? "scope" : "circle.dashed")
                    .font(.system(size: 13, weight: .semibold))
                    .foregroundStyle(appState.selection.hasSelection ? MirTheme.Colors.selection : MirTheme.Colors.textTertiary)

                VStack(alignment: .leading, spacing: 1) {
                    Text(appState.ui.language == .russian ? "Инспектор" : "Inspector")
                        .font(MirTheme.Typography.bodySemibold)
                        .foregroundStyle(MirTheme.Colors.textPrimary)
                    Text(appState.selection.hasSelection ? selectionKindTitle : (appState.ui.language == .russian ? "Ничего не выбрано" : "Nothing selected"))
                        .font(MirTheme.Typography.status)
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                }

                Spacer()

                Text(appState.selection.hasSelection ? "LIVE" : "IDLE")
                    .font(.system(size: 9, weight: .bold, design: .monospaced))
                    .foregroundStyle(appState.selection.hasSelection ? MirTheme.Colors.success : MirTheme.Colors.textTertiary)
                    .padding(.horizontal, 6)
                    .padding(.vertical, 4)
                    .background(MirTheme.Colors.surfaceRaised, in: Capsule())
            }

            Divider()

            identityRow("ObjectId", resolvedSelectionID)
            identityRow(appState.ui.language == .russian ? "Тип" : "Kind", selectionKindTitle)
            identityRow(appState.ui.language == .russian ? "Количество" : "Count", String(appState.selection.count))

            if let body = resolvedBody {
                identityRow(appState.ui.language == .russian ? "Тело" : "Body", body.name)
            }
        }
        .padding(MirTheme.Spacing.md)
        .background(MirTheme.Colors.surfaceRaised.opacity(0.55))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay {
            RoundedRectangle(cornerRadius: MirTheme.Radius.medium)
                .stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 1)
        }
        .padding(.horizontal, MirTheme.Spacing.md)
        .padding(.top, MirTheme.Spacing.md)
        .animation(MirTheme.Animation.fast, value: appState.selection)
        .onChange(of: appState.selection.ids) { _, _ in normalizeEngineSelection() }
        .onAppear { normalizeEngineSelection() }
    }

    private func identityRow(_ label: String, _ value: String) -> some View {
        HStack(spacing: 8) {
            Text(label)
                .font(MirTheme.Typography.status)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer(minLength: 10)
            Text(value)
                .font(.system(size: 10, weight: .semibold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textPrimary)
                .lineLimit(1)
                .truncationMode(.middle)
        }
        .frame(minHeight: 20)
    }

    private var resolvedSelectionID: String {
        guard let rawID = appState.selection.ids.first,
              let engineID = UInt64(rawID),
              let persistedID = runtime.persistedSelectionID(forEngineObjectID: engineID) else {
            return appState.selection.ids.first ?? "—"
        }
        return persistedID
    }

    private var resolvedBody: MIR4DBody? {
        guard let id = UUID(uuidString: resolvedSelectionID) else { return nil }
        return runtime.document.body(id: id)
    }

    private func normalizeEngineSelection() {
        guard appState.selection.primaryKind == .body,
              let rawID = appState.selection.ids.first,
              let engineID = UInt64(rawID),
              let persistedID = runtime.persistedSelectionID(forEngineObjectID: engineID),
              persistedID != rawID else { return }
        appState.setSelection(ids: [persistedID], kind: .body)
    }

    private var selectionKindTitle: String {
        switch appState.selection.primaryKind {
        case .none: return appState.ui.language == .russian ? "Нет" : "None"
        case .vertex: return appState.ui.language == .russian ? "Вершина" : "Vertex"
        case .edge: return appState.ui.language == .russian ? "Ребро" : "Edge"
        case .face: return appState.ui.language == .russian ? "Грань" : "Face"
        case .body: return appState.ui.language == .russian ? "Тело" : "Body"
        case .feature: return appState.ui.language == .russian ? "Функция" : "Feature"
        case .sketch: return appState.ui.language == .russian ? "Эскиз" : "Sketch"
        case .component: return appState.ui.language == .russian ? "Компонент" : "Component"
        case .simulationResult: return appState.ui.language == .russian ? "Результат расчёта" : "Simulation result"
        case .drawingView: return appState.ui.language == .russian ? "Вид чертежа" : "Drawing view"
        case .unknown: return appState.ui.language == .russian ? "Неизвестно" : "Unknown"
        }
    }
}
