import SwiftUI

enum MirSelectionFilter: String, CaseIterable, Identifiable {
    case auto, vertex, edge, face, body, feature, sketch

    var id: String { rawValue }

    func title(_ language: CADLanguage) -> String {
        switch (self, language) {
        case (.auto, .russian): return "Авто"
        case (.vertex, .russian): return "Вершина"
        case (.edge, .russian): return "Ребро"
        case (.face, .russian): return "Грань"
        case (.body, .russian): return "Тело"
        case (.feature, .russian): return "Элемент"
        case (.sketch, .russian): return "Эскиз"
        case (.auto, .english): return "Auto"
        case (.vertex, .english): return "Vertex"
        case (.edge, .english): return "Edge"
        case (.face, .english): return "Face"
        case (.body, .english): return "Body"
        case (.feature, .english): return "Feature"
        case (.sketch, .english): return "Sketch"
        }
    }

    var systemImage: String {
        switch self {
        case .auto: return "cursorarrow"
        case .vertex: return "circle.fill"
        case .edge: return "line.diagonal"
        case .face: return "square.fill"
        case .body: return "cube.fill"
        case .feature: return "gearshape.fill"
        case .sketch: return "pencil.and.ruler"
        }
    }
}

struct SelectionFilterBar: View {
    @ObservedObject var appState: CADAppState
    @State private var filter: MirSelectionFilter = .auto

    var body: some View {
        HStack(spacing: 0) {
            titleBlock
            Divider().frame(height: 22).padding(.horizontal, 7)
            filterButtons
            Spacer(minLength: 4)
        }
        .frame(minHeight: 40)
        .padding(.horizontal, 4)
        .background(MirTheme.Colors.surfaceRaised.opacity(0.96))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay { RoundedRectangle(cornerRadius: MirTheme.Radius.medium).stroke(MirTheme.Colors.panelBorder.opacity(0.8), lineWidth: 1) }
        .shadow(color: .black.opacity(0.08), radius: 7, y: 2)
        .accessibilityElement(children: .contain)
        .accessibilityLabel(appState.ui.language == .russian ? "Фильтр выбора геометрии" : "Geometry selection filter")
    }

    private var titleBlock: some View {
        HStack(spacing: 6) {
            Image(systemName: "scope").font(.system(size: 11, weight: .semibold)).foregroundStyle(MirTheme.Colors.accentBright)
            VStack(alignment: .leading, spacing: 0) {
                Text(appState.ui.language == .russian ? "Выбор" : "Selection").font(.system(size: 10, weight: .semibold)).foregroundStyle(MirTheme.Colors.textPrimary)
                Text(filter.title(appState.ui.language)).font(.system(size: 8)).foregroundStyle(MirTheme.Colors.textTertiary)
            }
        }.padding(.leading, 7).frame(minWidth: 72, alignment: .leading)
    }

    private var filterButtons: some View {
        HStack(spacing: 2) { ForEach(MirSelectionFilter.allCases) { filterButton($0) } }
    }

    private func filterButton(_ item: MirSelectionFilter) -> some View {
        let selected = filter == item
        return Button {
            withAnimation(MirTheme.Animation.fast) { filter = item }
            NotificationCenter.default.post(
                name: .mir4DSelectionFilterChanged,
                object: item.rawValue
            )
        } label: {
            VStack(spacing: 2) {
                Image(systemName: item.systemImage).font(.system(size: 10, weight: selected ? .semibold : .regular))
                Text(item.title(appState.ui.language)).font(.system(size: 8.5, weight: selected ? .semibold : .regular)).lineLimit(1)
            }
            .foregroundStyle(selected ? MirTheme.Colors.textPrimary : MirTheme.Colors.textSecondary)
            .frame(minWidth: 46, minHeight: 32)
            .background(selected ? MirTheme.Colors.accentSoft : Color.clear)
            .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay { if selected { RoundedRectangle(cornerRadius: MirTheme.Radius.small).stroke(MirTheme.Colors.accent.opacity(0.55), lineWidth: 1) } }
        }
        .buttonStyle(.plain)
        .help(item.title(appState.ui.language))
        .accessibilityLabel(item.title(appState.ui.language))
        .accessibilityAddTraits(selected ? .isSelected : [])
    }
}

extension Notification.Name {
    static let mir4DSelectionFilterChanged = Notification.Name("MIR4D.SelectionFilterChanged")
}
