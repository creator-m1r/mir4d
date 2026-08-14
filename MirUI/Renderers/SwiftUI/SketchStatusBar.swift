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

        var emphasis: Color {
            switch self {
            case .fullyConstrained: return MirTheme.Colors.success
            case .underConstrained: return MirTheme.Colors.warning
            case .overConstrained, .conflict: return MirTheme.Colors.error
            }
        }
    }

    var body: some View {
        HStack(spacing: 0) {
            statusGroup
            Divider().frame(height: 18).padding(.horizontal, 10)
            metricGroup(title: "Геометрия", value: geometryCount, icon: "point.3.connected.trianglepath.dotted")
            Divider().frame(height: 18).padding(.horizontal, 10)
            metricGroup(title: "Ограничения", value: constraintCount, icon: "lock.rotation")
            Spacer(minLength: 12)
            interactionHints
        }
        .font(MirTheme.Typography.status)
        .foregroundStyle(MirTheme.Colors.textSecondary)
        .padding(.horizontal, 10)
        .frame(minHeight: 32)
        .background(MirTheme.Colors.surfaceRaised.opacity(0.9))
        .overlay(alignment: .top) {
            Rectangle().fill(MirTheme.Colors.panelBorder).frame(height: 1)
        }
        .accessibilityElement(children: .contain)
        .accessibilityLabel("Статус эскиза")
    }

    private var statusGroup: some View {
        HStack(spacing: 6) {
            Image(systemName: status.icon)
                .font(.system(size: 11, weight: .semibold))
                .foregroundStyle(status.emphasis)
            Text(status.title)
                .font(MirTheme.Typography.caption)
                .fontWeight(.semibold)
                .foregroundStyle(status.emphasis)
        }
        .padding(.horizontal, 7)
        .padding(.vertical, 5)
        .background(status.emphasis.opacity(0.10), in: Capsule())
    }

    private func metricGroup(title: String, value: Int, icon: String) -> some View {
        HStack(spacing: 5) {
            Image(systemName: icon)
                .font(.system(size: 10, weight: .medium))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text(title)
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text("\(value)")
                .font(.system(size: 10, weight: .semibold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textPrimary)
        }
        .padding(.horizontal, 4)
    }

    private var interactionHints: some View {
        HStack(spacing: 8) {
            hint("Esc", "Отмена")
            hint("⌘Z", "Отменить")
            hint("⌘⇧Z", "Повторить")
        }
        .foregroundStyle(MirTheme.Colors.textTertiary)
    }

    private func hint(_ key: String, _ title: String) -> some View {
        HStack(spacing: 4) {
            Text(key)
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .padding(.horizontal, 4)
                .padding(.vertical, 2)
                .background(MirTheme.Colors.surface, in: RoundedRectangle(cornerRadius: 3))
            Text(title)
                .font(.system(size: 9))
        }
    }
}
