import SwiftUI

/// Dedicated UI-only layout editor for MIR 4D panels.
///
/// This view changes presentation state only. It deliberately does not touch
/// MirEngine or any engineering model data.
struct InterfaceCustomizationView: View {
    @ObservedObject var appState: CADAppState
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        VStack(spacing: 0) {
            header
            Divider().overlay(MirTheme.Colors.border)

            ScrollView {
                VStack(alignment: .leading, spacing: MirTheme.Spacing.lg) {
                    intro
                    panelList
                    presets
                }
                .padding(MirTheme.Spacing.xl)
            }

            footer
        }
        .frame(minWidth: 720, idealWidth: 820, minHeight: 560, idealHeight: 640)
        .background(MirTheme.Colors.background)
        .preferredColorScheme(.dark)
    }

    private var header: some View {
        HStack(spacing: MirTheme.Spacing.md) {
            Image(systemName: "rectangle.3.group")
                .font(.system(size: 18, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)

            VStack(alignment: .leading, spacing: 2) {
                Text(appState.ui.language == .russian ? "Настройка интерфейса" : "Customize Interface")
                    .font(MirTheme.Typography.title)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Text(appState.ui.language == .russian ? "Панели и рабочее пространство МИР 4D" : "MIR 4D panels and workspace")
                    .font(MirTheme.Typography.caption)
                    .foregroundStyle(MirTheme.Colors.textSecondary)
            }

            Spacer()

            Text(appState.workbench.titleRU)
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .padding(.horizontal, 9)
                .padding(.vertical, 5)
                .background(MirTheme.Colors.surfaceRaised, in: Capsule())
        }
        .padding(.horizontal, MirTheme.Spacing.xl)
        .padding(.vertical, MirTheme.Spacing.lg)
    }

    private var intro: some View {
        HStack(alignment: .top, spacing: MirTheme.Spacing.md) {
            Image(systemName: "hand.draw")
                .foregroundStyle(MirTheme.Colors.info)
            VStack(alignment: .leading, spacing: 5) {
                Text(appState.ui.language == .russian ? "Отдельный режим редактирования" : "Dedicated editing mode")
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Text(appState.ui.language == .russian
                     ? "Изменяйте расположение панелей здесь. Рабочая область модели и инженерные данные не изменяются."
                     : "Change panel placement here. The modeling workspace and engineering data remain untouched.")
                    .font(MirTheme.Typography.body)
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                    .fixedSize(horizontal: false, vertical: true)
            }
        }
        .padding(MirTheme.Spacing.lg)
        .background(MirTheme.Colors.surface, in: RoundedRectangle(cornerRadius: MirTheme.Radius.panel))
        .overlay {
            RoundedRectangle(cornerRadius: MirTheme.Radius.panel)
                .stroke(MirTheme.Colors.border, lineWidth: 1)
        }
    }

    private var panelList: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.sm) {
            Text(appState.ui.language == .russian ? "ПАНЕЛИ" : "PANELS")
                .font(MirTheme.Typography.section)
                .foregroundStyle(MirTheme.Colors.textTertiary)

            ForEach(CADPanel.allCases) { panel in
                panelRow(panel)
            }
        }
    }

    private func panelRow(_ panel: CADPanel) -> some View {
        let visible = appState.visiblePanels.contains(panel)

        return HStack(spacing: MirTheme.Spacing.md) {
            Image(systemName: icon(for: panel))
                .frame(width: 24)
                .foregroundStyle(visible ? MirTheme.Colors.accentBright : MirTheme.Colors.textTertiary)

            VStack(alignment: .leading, spacing: 2) {
                Text(appState.ui.language == .russian ? panel.titleRU : panel.titleEN)
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(visible ? MirTheme.Colors.textPrimary : MirTheme.Colors.textSecondary)
                Text(placementTitle(for: panel))
                    .font(MirTheme.Typography.status)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }

            Spacer()

            Menu {
                ForEach(PanelPlacement.allCases) { placement in
                    Button {
                        appState.setPanelPlacement(placement, for: panel)
                    } label: {
                        Label(placementTitle(placement), systemImage: placement.icon)
                    }
                }
            } label: {
                Label(placementTitle(for: panel), systemImage: appState.panelPlacement(for: panel).icon)
                    .font(MirTheme.Typography.caption)
            }
            .menuStyle(.borderlessButton)
            .frame(minWidth: 145, alignment: .trailing)

            Toggle("", isOn: Binding(
                get: { visible },
                set: { _ in appState.togglePanel(panel) }
            ))
            .labelsHidden()
            .toggleStyle(.switch)
            .controlSize(.small)
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .padding(.vertical, 10)
        .background(MirTheme.Colors.surface, in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay {
            RoundedRectangle(cornerRadius: MirTheme.Radius.medium)
                .stroke(MirTheme.Colors.border, lineWidth: 1)
        }
    }

    private var presets: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.sm) {
            Text(appState.ui.language == .russian ? "РАЗМЕЩЕНИЕ" : "PLACEMENT")
                .font(MirTheme.Typography.section)
                .foregroundStyle(MirTheme.Colors.textTertiary)

            HStack(spacing: MirTheme.Spacing.sm) {
                ForEach(PanelPlacement.allCases) { placement in
                    Button {
                        moveVisiblePanels(to: placement)
                    } label: {
                        Label(placementTitle(placement), systemImage: placement.icon)
                            .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.bordered)
                    .controlSize(.regular)
                }
            }
        }
    }

    private var footer: some View {
        HStack {
            Image(systemName: "checkmark.circle.fill")
                .foregroundStyle(MirTheme.Colors.success)
            Text(appState.ui.language == .russian ? "Настройки применяются сразу" : "Changes apply immediately")
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textSecondary)
            Spacer()
            Button(appState.ui.language == .russian ? "Готово" : "Done") {
                dismiss()
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.regular)
        }
        .padding(.horizontal, MirTheme.Spacing.xl)
        .padding(.vertical, MirTheme.Spacing.lg)
        .background(MirTheme.Colors.surface)
    }

    private func moveVisiblePanels(to placement: PanelPlacement) {
        for panel in appState.visiblePanels {
            appState.setPanelPlacement(placement, for: panel)
        }
    }

    private func placementTitle(for panel: CADPanel) -> String {
        placementTitle(appState.panelPlacement(for: panel))
    }

    private func placementTitle(_ placement: PanelPlacement) -> String {
        appState.ui.language == .russian ? placement.titleRU : placement.titleEN
    }

    private func icon(for panel: CADPanel) -> String {
        switch panel {
        case .project: return "folder"
        case .properties: return "slider.horizontal.3"
        case .timeline: return "clock"
        case .simulation: return "waveform.path.ecg"
        case .history: return "clock.arrow.circlepath"
        case .aiInspector: return "sparkles"
        }
    }
}
