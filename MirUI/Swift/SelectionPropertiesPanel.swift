import SwiftUI

public struct SelectionPropertiesPanel: View {
    @ObservedObject private var inspector: SelectionInspector

    public init(inspector: SelectionInspector) {
        self.inspector = inspector
    }

    public var body: some View {
        VStack(spacing: 0) {
            header
            Divider()

            if inspector.properties.isEmpty {
                emptyState
            } else {
                propertyList
            }
        }
        .background(MirTheme.Colors.panel)
        .overlay {
            RoundedRectangle(cornerRadius: MirTheme.Radius.medium)
                .stroke(MirTheme.Colors.panelBorder.opacity(0.8), lineWidth: 1)
        }
    }

    private var header: some View {
        HStack(spacing: 8) {
            Image(systemName: "slider.horizontal.3")
                .font(.system(size: 11, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)

            VStack(alignment: .leading, spacing: 1) {
                Text("ИНСПЕКТОР")
                    .font(.system(size: 10, weight: .semibold))
                    .tracking(0.5)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Text(inspector.properties.isEmpty ? "Нет выбранного объекта" : "Свойства выбранного объекта")
                    .font(MirTheme.Typography.status)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }

            Spacer()

            Circle()
                .fill(inspector.properties.isEmpty ? MirTheme.Colors.textTertiary : MirTheme.Colors.success)
                .frame(width: 7, height: 7)
                .help(inspector.properties.isEmpty ? "Нет выделения" : "Объект выбран")
        }
        .padding(.horizontal, 12)
        .frame(height: 46)
        .background(MirTheme.Colors.surface.opacity(0.55))
    }

    private var emptyState: some View {
        VStack(spacing: 10) {
            Spacer()
            Image(systemName: "square.dashed")
                .font(.system(size: 25, weight: .light))
                .foregroundStyle(MirTheme.Colors.textTertiary)
            Text("Нет выделения")
                .font(MirTheme.Typography.bodySemibold)
                .foregroundStyle(MirTheme.Colors.textSecondary)
            Text("Выберите объект в 3D-виде или в навигаторе")
                .font(MirTheme.Typography.status)
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .multilineTextAlignment(.center)
                .frame(maxWidth: 190)
            Spacer()
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding(16)
    }

    private var propertyList: some View {
        ScrollView {
            LazyVStack(spacing: 0) {
                ForEach(inspector.properties) { property in
                    HStack(alignment: .top, spacing: 12) {
                        Text(property.name)
                            .font(MirTheme.Typography.caption)
                            .foregroundStyle(MirTheme.Colors.textTertiary)
                            .frame(maxWidth: .infinity, alignment: .leading)

                        Text(property.value)
                            .font(.system(size: 11, weight: .medium, design: .monospaced))
                            .foregroundStyle(MirTheme.Colors.textPrimary)
                            .multilineTextAlignment(.trailing)
                            .textSelection(.enabled)
                    }
                    .padding(.horizontal, 12)
                    .padding(.vertical, 9)
                    .background(MirTheme.Colors.surface.opacity(0.18))

                    Divider().opacity(0.45)
                }
            }
            .padding(.vertical, 4)
        }
        .scrollIndicators(.automatic)
    }
}
