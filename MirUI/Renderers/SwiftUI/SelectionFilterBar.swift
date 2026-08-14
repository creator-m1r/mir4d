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
        HStack(spacing: 8) {
            HStack(spacing: 6) {
                Image(systemName: "scope")
                    .font(.system(size: 11, weight: .semibold))
                    .foregroundStyle(MirTheme.Colors.accentBright)

                Text(appState.ui.language == .russian ? "Выбор" : "Selection")
                    .font(MirTheme.Typography.caption)
                    .foregroundStyle(MirTheme.Colors.textSecondary)
            }
            .padding(.leading, 10)

            Divider().frame(height: 18)

            ForEach(MirSelectionFilter.allCases) { item in
                filterButton(item)
            }

            Spacer(minLength: 6)
        }
        .frame(height: 36)
        .padding(.horizontal, 3)
        .background(MirTheme.Colors.surfaceRaised.opacity(0.92))
        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay {
            RoundedRectangle(cornerRadius: MirTheme.Radius.medium)
                .stroke(MirTheme.Colors.panelBorder.opacity(0.72), lineWidth: 1)
        }
        .shadow(color: .black.opacity(0.10), radius: 8, y: 3)
        .accessibilityElement(children: .contain)
        .accessibilityLabel(appState.ui.language == .russian ? "Фильтр выбора" : "Selection filter")
    }

    private func filterButton(_ item: MirSelectionFilter) -> some View {
        let selected = filter == item

        return Button {
            withAnimation(MirTheme.Animation.fast) {
                filter = item
            }
        } label: {
            HStack(spacing: 5) {
                Image(systemName: item.systemImage)
                    .font(.system(size: 10, weight: selected ? .semibold : .regular))
                Text(item.title(appState.ui.language))
                    .font(.system(size: 10, weight: selected ? .semibold : .regular))
            }
            .foregroundStyle(selected ? MirTheme.Colors.textPrimary : MirTheme.Colors.textSecondary)
            .padding(.horizontal, 8)
            .frame(minHeight: 27)
            .background(selected ? MirTheme.Colors.accentSoft : Color.clear)
            .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            .overlay {
                if selected {
                    RoundedRectangle(cornerRadius: MirTheme.Radius.small)
                        .stroke(MirTheme.Colors.accent.opacity(0.55), lineWidth: 1)
                }
            }
        }
        .buttonStyle(.plain)
        .help(item.title(appState.ui.language))
    }
}
