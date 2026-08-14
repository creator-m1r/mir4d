import SwiftUI

/// Dedicated UI-only layout editor for MIR 4D panels.
/// This view changes presentation state only; MirEngine and engineering data remain untouched.
struct InterfaceCustomizationView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject private var navigation = MirNavigationSettingsStore.shared
    @Environment(\.dismiss) private var dismiss
    @State private var selectedPanel: CADPanel?

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        VStack(spacing: 0) {
            header
            Divider().overlay(MirTheme.Colors.border)
            ScrollView {
                VStack(alignment: .leading, spacing: MirTheme.Spacing.lg) {
                    modeBanner
                    panelList
                    placementPresets
                    navigationSection
                    workspaceRules
                }
                .padding(MirTheme.Spacing.xl)
            }
            footer
        }
        .frame(minWidth: 820, idealWidth: 920, minHeight: 650, idealHeight: 720)
        .background(MirTheme.Colors.background)
        .preferredColorScheme(.dark)
    }

    private var header: some View {
        HStack(spacing: MirTheme.Spacing.md) {
            Image(systemName: "rectangle.3.group")
                .font(.system(size: 18, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.accentBright)
            VStack(alignment: .leading, spacing: 2) {
                Text(russian ? "Настройка интерфейса" : "Customize Interface")
                    .font(MirTheme.Typography.title)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Text(russian ? "Редактор рабочего пространства МИР 4D" : "MIR 4D workspace editor")
                    .font(MirTheme.Typography.caption)
                    .foregroundStyle(MirTheme.Colors.textSecondary)
            }
            Spacer()
            Label(russian ? "РЕЖИМ РЕДАКТИРОВАНИЯ" : "EDIT MODE", systemImage: "slider.horizontal.3")
                .font(.system(size: 10, weight: .semibold))
                .tracking(0.35)
                .foregroundStyle(MirTheme.Colors.accentBright)
                .padding(.horizontal, 10).padding(.vertical, 6)
                .background(MirTheme.Colors.accentSoft, in: Capsule())
        }
        .padding(.horizontal, MirTheme.Spacing.xl)
        .padding(.vertical, MirTheme.Spacing.lg)
    }

    private var modeBanner: some View {
        HStack(alignment: .top, spacing: MirTheme.Spacing.md) {
            Image(systemName: "hand.draw")
                .font(.system(size: 16, weight: .medium))
                .foregroundStyle(MirTheme.Colors.info)
                .frame(width: 28)
            VStack(alignment: .leading, spacing: 5) {
                Text(russian ? "Отдельный режим редактирования" : "Dedicated editing mode")
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Text(russian ? "Изменяйте расположение и видимость панелей здесь. Модель, геометрия и данные ядра не изменяются." : "Change panel placement and visibility here. The model, geometry and core data remain untouched.")
                    .font(MirTheme.Typography.body)
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                    .fixedSize(horizontal: false, vertical: true)
            }
            Spacer()
        }
        .padding(MirTheme.Spacing.lg)
        .background(MirTheme.Colors.surface, in: RoundedRectangle(cornerRadius: MirTheme.Radius.panel))
        .overlay { RoundedRectangle(cornerRadius: MirTheme.Radius.panel).stroke(MirTheme.Colors.border, lineWidth: 1) }
    }

    private var panelList: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.sm) {
            sectionTitle(russian ? "ПАНЕЛИ" : "PANELS", subtitle: russian ? "Выберите панель для настройки" : "Select a panel to customize")
            ForEach(CADPanel.allCases) { panel in panelRow(panel) }
        }
    }

    private func panelRow(_ panel: CADPanel) -> some View {
        let visible = appState.visiblePanels.contains(panel)
        let placement = appState.panelPlacement(for: panel)
        let selected = selectedPanel == panel
        return HStack(spacing: MirTheme.Spacing.md) {
            Button { selectedPanel = panel } label: {
                Image(systemName: icon(for: panel))
                    .frame(width: 28, height: 28)
                    .foregroundStyle(selected || visible ? MirTheme.Colors.accentBright : MirTheme.Colors.textTertiary)
                    .background(selected ? MirTheme.Colors.accentSoft : MirTheme.Colors.surfaceRaised.opacity(0.55), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            }
            .buttonStyle(.plain)
            VStack(alignment: .leading, spacing: 2) {
                Text(russian ? panel.titleRU : panel.titleEN)
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(visible ? MirTheme.Colors.textPrimary : MirTheme.Colors.textSecondary)
                Text(placementTitle(placement))
                    .font(MirTheme.Typography.status)
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }
            Spacer()
            Menu {
                ForEach(PanelPlacement.allCases) { target in
                    Button { appState.setPanelPlacement(target, for: panel); selectedPanel = panel } label: {
                        Label(placementTitle(target), systemImage: target.icon)
                    }
                }
            } label: {
                Label(placementTitle(placement), systemImage: placement.icon)
                    .font(MirTheme.Typography.caption)
                    .padding(.horizontal, 9).padding(.vertical, 6)
            }
            .menuStyle(.borderlessButton)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.65), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
            Toggle("", isOn: Binding(get: { visible }, set: { _ in appState.togglePanel(panel) }))
                .labelsHidden().toggleStyle(.switch).controlSize(.small)
        }
        .padding(.horizontal, MirTheme.Spacing.lg).padding(.vertical, 10)
        .background(selected ? MirTheme.Colors.accentSoft.opacity(0.28) : MirTheme.Colors.surface, in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        .overlay { RoundedRectangle(cornerRadius: MirTheme.Radius.medium).stroke(selected ? MirTheme.Colors.accent.opacity(0.65) : MirTheme.Colors.border, lineWidth: 1) }
    }

    private var placementPresets: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.sm) {
            sectionTitle(russian ? "БЫСТРОЕ РАЗМЕЩЕНИЕ" : "QUICK PLACEMENT", subtitle: russian ? "Переместить все видимые панели" : "Move all visible panels")
            HStack(spacing: MirTheme.Spacing.sm) {
                ForEach(PanelPlacement.allCases) { placement in
                    Button { moveVisiblePanels(to: placement) } label: {
                        Label(placementTitle(placement), systemImage: placement.icon).frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.bordered)
                    .controlSize(.regular)
                }
            }
        }
    }

    private var navigationSection: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.sm) {
            sectionTitle(russian ? "НАВИГАЦИЯ · УПРАВЛЕНИЕ ТАЧПАДОМ" : "NAVIGATION · TRACKPAD CONTROL", subtitle: russian ? "Схема как в Blender: два пальца — поворот, Shift — панорама, Control — масштаб" : "Blender-style: two fingers orbit, Shift pans, Control zooms")
            HStack(spacing: MirTheme.Spacing.md) {
                Text(russian ? "Жест двумя пальцами" : "Two-finger gesture")
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Spacer()
                Picker("", selection: $navigation.settings.trackpadGesture) {
                    ForEach(MirTrackpadGesture.allCases) { gesture in
                        Text(russian ? gesture.titleRU : gesture.titleEN).tag(gesture)
                    }
                }
                .labelsHidden()
                .pickerStyle(.segmented)
                .frame(width: 260)
            }
            HStack(spacing: MirTheme.Spacing.md) {
                Text(russian ? "Чувствительность поворота" : "Orbit sensitivity")
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Spacer()
                Slider(value: $navigation.settings.orbitSensitivity, in: 0.001 ... 0.012, step: 0.0005)
                    .frame(width: 220)
                Text(String(format: "%.4f", navigation.settings.orbitSensitivity))
                    .font(MirTheme.Typography.status.monospacedDigit())
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                    .frame(width: 56, alignment: .trailing)
            }
            HStack(spacing: MirTheme.Spacing.md) {
                Text(russian ? "Чувствительность масштаба" : "Zoom sensitivity")
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
                Spacer()
                Slider(value: $navigation.settings.zoomSensitivity, in: 0.002 ... 0.05, step: 0.001)
                    .frame(width: 220)
                Text(String(format: "%.3f", navigation.settings.zoomSensitivity))
                    .font(MirTheme.Typography.status.monospacedDigit())
                    .foregroundStyle(MirTheme.Colors.textSecondary)
                    .frame(width: 56, alignment: .trailing)
            }
            HStack(spacing: MirTheme.Spacing.md) {
                Toggle(russian ? "Инвертировать поворот по X" : "Invert orbit X", isOn: $navigation.settings.invertOrbitX)
                    .toggleStyle(.switch).controlSize(.small)
                Spacer()
                Toggle(russian ? "Инвертировать поворот по Y" : "Invert orbit Y", isOn: $navigation.settings.invertOrbitY)
                    .toggleStyle(.switch).controlSize(.small)
                Spacer()
                Toggle(russian ? "Инвертировать масштаб" : "Invert zoom", isOn: $navigation.settings.invertZoom)
                    .toggleStyle(.switch).controlSize(.small)
            }
            .font(MirTheme.Typography.caption)
            .foregroundStyle(MirTheme.Colors.textSecondary)
        }
        .padding(MirTheme.Spacing.lg)
        .background(MirTheme.Colors.surface.opacity(0.65), in: RoundedRectangle(cornerRadius: MirTheme.Radius.panel))
    }

    private var workspaceRules: some View {
        VStack(alignment: .leading, spacing: 8) {
            sectionTitle(russian ? "ПРАВИЛА РАБОЧЕГО МЕСТА" : "WORKSPACE RULES", subtitle: nil)
            ruleRow("1", russian ? "3D-вид остаётся главным рабочим пространством" : "3D view remains the primary workspace")
            ruleRow("2", russian ? "Верхняя полоса принадлежит программе и не перекрывает рамку окна" : "The application top bar never covers the system window frame")
            ruleRow("3", russian ? "Панели не должны закрывать модель без необходимости" : "Panels should not unnecessarily cover the model")
            ruleRow("4", russian ? "Настройки изменяют только интерфейс" : "Settings change presentation only")
        }
        .padding(MirTheme.Spacing.lg)
        .background(MirTheme.Colors.surface.opacity(0.65), in: RoundedRectangle(cornerRadius: MirTheme.Radius.panel))
    }

    private func ruleRow(_ number: String, _ text: String) -> some View {
        HStack(alignment: .top, spacing: 9) {
            Text(number).font(.system(size: 10, weight: .bold, design: .monospaced)).foregroundStyle(MirTheme.Colors.accentBright).frame(width: 20)
            Text(text).font(MirTheme.Typography.caption).foregroundStyle(MirTheme.Colors.textSecondary)
            Spacer()
        }
    }

    private func sectionTitle(_ title: String, subtitle: String?) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(title).font(MirTheme.Typography.section).foregroundStyle(MirTheme.Colors.textTertiary)
            if let subtitle { Text(subtitle).font(MirTheme.Typography.status).foregroundStyle(MirTheme.Colors.textTertiary) }
        }
    }

    private var footer: some View {
        HStack {
            Image(systemName: "checkmark.circle.fill").foregroundStyle(MirTheme.Colors.success)
            Text(russian ? "Настройки применяются сразу" : "Changes apply immediately")
                .font(MirTheme.Typography.caption).foregroundStyle(MirTheme.Colors.textSecondary)
            Spacer()
            Button(russian ? "Готово" : "Done") { dismiss() }
                .buttonStyle(.borderedProminent).controlSize(.regular)
        }
        .padding(.horizontal, MirTheme.Spacing.xl).padding(.vertical, MirTheme.Spacing.lg)
        .background(MirTheme.Colors.surface)
    }

    private func moveVisiblePanels(to placement: PanelPlacement) {
        for panel in appState.visiblePanels { appState.setPanelPlacement(placement, for: panel) }
    }
    private func placementTitle(_ placement: PanelPlacement) -> String { russian ? placement.titleRU : placement.titleEN }
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
