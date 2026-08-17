import SwiftUI

/// Context-first identity card for the Properties area.
/// It deliberately stays small: detailed editors remain in InspectorTabsView.
struct SelectionIdentityInspector: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var runtime = MIR4DModelRuntime.shared

    private var ru: Bool { appState.ui.language == .russian }

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            header

            if appState.selection.hasSelection {
                selectedContent
            } else {
                emptyContent
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

    private var header: some View {
        HStack(spacing: 8) {
            Image(systemName: appState.selection.hasSelection ? "scope" : "cursorarrow.click.2")
                .font(.system(size: 13, weight: .semibold))
                .foregroundStyle(appState.selection.hasSelection ? MirTheme.Colors.selection : MirTheme.Colors.accentBright)

            VStack(alignment: .leading, spacing: 1) {
                Text(ru ? "Свойства" : "Properties")
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Text(appState.selection.hasSelection ? selectionKindTitle : (ru ? "Контекст модели" : "Model context"))
                    .font(MirTheme.Typography.status)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }

            Spacer()

            Text(appState.selection.hasSelection ? "LIVE" : "READY")
                .font(.system(size: 9, weight: .bold, design: .monospaced))
                .foregroundStyle(appState.selection.hasSelection ? MirTheme.Colors.success : MirTheme.Colors.accentBright)
                .padding(.horizontal, 6)
                .padding(.vertical, 4)
                .background(MirTheme.Colors.surfaceRaised, in: Capsule())
        }
    }

    private var selectedContent: some View {
        VStack(alignment: .leading, spacing: 8) {
            Divider()
            identityRow("ObjectId", resolvedSelectionID)
            identityRow(ru ? "Тип" : "Kind", selectionKindTitle)
            identityRow(ru ? "Количество" : "Count", String(appState.selection.count))

            if let body = resolvedBody {
                identityRow(ru ? "Тело" : "Body", body.name)
            }

            HStack(spacing: 7) {
                contextChip(icon: "scope", title: ru ? "Выделено" : "Selected")
                if appState.selection.count > 1 {
                    contextChip(icon: "square.stack.3d.up", title: ru ? "Множественный выбор" : "Multiple")
                }
            }
        }
    }

    private var emptyContent: some View {
        VStack(alignment: .leading, spacing: 9) {
            Divider()
            Text(ru ? "Выберите объект, чтобы увидеть его параметры." : "Select an object to see its parameters.")
                .font(MirTheme.Typography.body)
                .foregroundStyle(MirTheme.Colors.textSecondary)

            VStack(alignment: .leading, spacing: 6) {
                hintRow("scope", ru ? "Нажмите объект в viewport" : "Click an object in the viewport")
                hintRow("list.bullet.indent", ru ? "Или выберите его в Structure" : "Or select it in Structure")
                hintRow("command", ru ? "⌘K — найти нужное действие" : "⌘K — find an action")
            }
        }
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

    private func hintRow(_ icon: String, _ text: String) -> some View {
        Label(text, systemImage: icon)
            .font(MirTheme.Typography.status)
            .foregroundStyle(MirTheme.Colors.textSecondary)
    }

    private func contextChip(icon: String, title: String) -> some View {
        Label(title, systemImage: icon)
            .font(.system(size: 9, weight: .medium))
            .foregroundStyle(MirTheme.Colors.textSecondary)
            .padding(.horizontal, 7)
            .padding(.vertical, 4)
            .background(MirTheme.Colors.surface, in: Capsule())
            .overlay(Capsule().stroke(MirTheme.Colors.border, lineWidth: 1))
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
        case .none: return ru ? "Нет" : "None"
        case .vertex: return ru ? "Вершина" : "Vertex"
        case .edge: return ru ? "Ребро" : "Edge"
        case .face: return ru ? "Грань" : "Face"
        case .body: return ru ? "Тело" : "Body"
        case .feature: return ru ? "Функция" : "Feature"
        case .sketch: return ru ? "Эскиз" : "Sketch"
        case .component: return ru ? "Компонент" : "Component"
        case .simulationResult: return ru ? "Результат расчёта" : "Simulation result"
        case .drawingView: return ru ? "Вид чертежа" : "Drawing view"
        case .unknown: return ru ? "Неизвестно" : "Unknown"
        }
    }
}
