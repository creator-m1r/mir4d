import SwiftUI

struct SelectionIdentityInspector: View {
    @ObservedObject var appState: CADAppState

    var body: some View {
        VStack(alignment: .leading, spacing: 7) {
            HStack(spacing: 7) {
                Image(systemName: appState.selection.hasSelection ? "scope" : "scope")
                    .foregroundStyle(appState.selection.hasSelection ? MirTheme.Colors.selection : MirTheme.Colors.textTertiary)

                Text(appState.ui.language == .russian ? "Идентичность объекта" : "Object Identity")
                    .font(.system(size: 10, weight: .semibold))

                Spacer()

                Text(appState.selection.hasSelection ? "LIVE" : "—")
                    .font(.system(size: 9, weight: .bold, design: .monospaced))
                    .foregroundStyle(appState.selection.hasSelection ? MirTheme.Colors.success : MirTheme.Colors.textTertiary)
            }

            identityRow(
                appState.ui.language == .russian ? "ObjectId" : "ObjectId",
                appState.selection.ids.first ?? "—"
            )

            identityRow(
                appState.ui.language == .russian ? "Тип" : "Kind",
                selectionKindTitle
            )

            identityRow(
                appState.ui.language == .russian ? "Количество" : "Count",
                String(appState.selection.count)
            )
        }
        .padding(10)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 10))
        .overlay {
            RoundedRectangle(cornerRadius: 10)
                .stroke(MirTheme.Colors.panelBorder.opacity(0.65), lineWidth: 1)
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .padding(.top, MirTheme.Spacing.md)
        .animation(MirTheme.Animation.fast, value: appState.selection)
    }

    private func identityRow(_ label: String, _ value: String) -> some View {
        HStack(spacing: 8) {
            Text(label)
                .font(.system(size: 9))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Spacer()
            Text(value)
                .font(.system(size: 10, weight: .bold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textPrimary)
                .lineLimit(1)
        }
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
