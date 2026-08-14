import SwiftUI

struct SketchStatusBar: View {
    let status: Status
    let geometryCount: Int
    let constraintCount: Int

    enum Status {
        case fullyConstrained
        case underConstrained
        case overConstrained
        case conflict

        var title: String {
            switch self {
            case .fullyConstrained: return "Полностью определён"
            case .underConstrained: return "Недоопределён"
            case .overConstrained: return "Переопределён"
            case .conflict: return "Конфликт"
            }
        }

        var icon: String {
            switch self {
            case .fullyConstrained: return "checkmark.circle.fill"
            case .underConstrained: return "circle.dashed"
            case .overConstrained: return "exclamationmark.triangle.fill"
            case .conflict: return "xmark.octagon.fill"
            }
        }
    }

    var body: some View {
        HStack(spacing: 12) {
            Label("Эскиз", systemImage: "pencil.and.ruler")
                .font(.system(size: 11, weight: .semibold))

            Divider().frame(height: 14)

            Label("Геометрия: \(geometryCount)", systemImage: "point.3.connected.trianglepath.dotted")
            Label("Ограничения: \(constraintCount)", systemImage: "lock.rotation")

            Spacer()

            Label(status.title, systemImage: status.icon)
                .fontWeight(.semibold)
        }
        .font(.system(size: 10))
        .foregroundStyle(.secondary)
        .padding(.horizontal, 12)
        .frame(height: 28)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 8))
    }
}
