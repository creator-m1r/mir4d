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
        HStack(spacing: 4) {
            Image(systemName: "cursorarrow.rays")
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .padding(.leading, 7)

            ForEach(MirSelectionFilter.allCases) { item in
                Button {
                    filter = item
                } label: {
                    Label(item.title(appState.ui.language), systemImage: item.systemImage)
                        .font(.system(size: 10, weight: filter == item ? .semibold : .regular))
                        .labelStyle(.titleAndIcon)
                        .padding(.horizontal, 7)
                        .padding(.vertical, 5)
                        .background(filter == item ? MirTheme.Colors.selection.opacity(0.22) : .clear)
                        .clipShape(RoundedRectangle(cornerRadius: 5))
                }
                .buttonStyle(.plain)
            }

            Spacer(minLength: 4)
        }
        .frame(height: 30)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 7))
        .overlay(RoundedRectangle(cornerRadius: 7).stroke(MirTheme.Colors.border.opacity(0.45)))
    }
}
