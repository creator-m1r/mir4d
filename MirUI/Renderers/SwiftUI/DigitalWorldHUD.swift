import SwiftUI

struct DigitalWorldHUD: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var store: ProductionWorldStore

    var body: some View {
        VStack(alignment: .trailing, spacing: 8) {
            header
            machineCard
            metricsCard
            scenarioCard
        }
        .padding(.trailing, 14)
        .padding(.top, 94)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
    }

    private var header: some View {
        HStack(spacing: 8) {
            Circle()
                .fill(store.worldRunning ? MirTheme.Colors.success : MirTheme.Colors.time)
                .frame(width: 7, height: 7)
            Text(appState.ui.language == .russian ? "ЦИФРОВОЙ МИР · LIVE" : "DIGITAL WORLD · LIVE")
                .font(.system(size: 11, weight: .bold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textPrimary)
            Spacer(minLength: 0)
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 8)
        .frame(width: 290)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 10))
    }

    private var machineCard: some View {
        panel {
            VStack(alignment: .leading, spacing: 8) {
                HStack {
                    Label(appState.ui.language == .russian ? "Виртуальный стенд" : "Virtual Stand", systemImage: "gearshape.2")
                        .font(.system(size: 11, weight: .semibold))
                    Spacer()
                    Text("RUN-01")
                        .font(.system(size: 9, weight: .bold, design: .monospaced))
                        .foregroundStyle(MirTheme.Colors.textTertiary)
                }

                Text(store.currentMachine == "Станок не выбран" ? (appState.ui.language == .russian ? "Система жизнеобеспечения" : "Life Support System") : store.currentMachine)
                    .font(.system(size: 13, weight: .bold))
                    .foregroundStyle(MirTheme.Colors.textPrimary)

                metricRow("Состояние", store.worldRunning ? "Работа" : "Готов", store.worldRunning ? MirTheme.Colors.success : MirTheme.Colors.warning)
                metricRow("Нагрузка", "\(Int((0.32 + appState.timeState.normalizedProgress * 0.46) * 100))%", MirTheme.Colors.simulation)
                metricRow("Температура", "\(Int(42 + appState.timeState.normalizedProgress * 18)) °C", MirTheme.Colors.time)
                metricRow("Мощность", "\(String(format: "%.1f", 12.5 + appState.timeState.normalizedProgress * 4.2)) kW", MirTheme.Colors.accent)
            }
        }
    }

    private var metricsCard: some View {
        panel {
            VStack(alignment: .leading, spacing: 7) {
                Text(appState.ui.language == .russian ? "Датчики" : "Sensors")
                    .font(.system(size: 11, weight: .semibold))
                sensor("VIB", value: "0.18", unit: "g", good: true)
                sensor("PRESS", value: "2.6", unit: "bar", good: true)
                sensor("FLOW", value: "84", unit: "%", good: true)
                sensor("EFF", value: "91.4", unit: "%", good: true)
            }
        }
    }

    private var scenarioCard: some View {
        panel {
            VStack(alignment: .leading, spacing: 8) {
                HStack {
                    Label(appState.ui.language == .russian ? "Сценарий" : "Scenario", systemImage: "point.3.connected.trianglepath.dotted")
                        .font(.system(size: 11, weight: .semibold))
                    Spacer()
                }
                Text(appState.ui.language == .russian ? "Нормальный производственный цикл" : "Normal production cycle")
                    .font(.system(size: 12, weight: .medium))
                    .lineLimit(2)
                HStack(spacing: 6) {
                    Button {
                        store.runDigitalTest()
                        appState.selectWorkbench(.simulation)
                    } label: {
                        Label(appState.ui.language == .russian ? "Тест" : "Test", systemImage: "play.fill")
                    }
                    .buttonStyle(.borderedProminent)
                    .controlSize(.small)

                    Button {
                        store.advance(to: .scenario)
                        appState.selectWorkbench(.fourD)
                    } label: {
                        Label(appState.ui.language == .russian ? "Что если" : "What-if", systemImage: "sparkles")
                    }
                    .buttonStyle(.bordered)
                    .controlSize(.small)
                }
            }
        }
    }

    private func panel<Content: View>(@ViewBuilder content: () -> Content) -> some View {
        content()
            .padding(10)
            .frame(width: 290, alignment: .leading)
            .background(.ultraThinMaterial)
            .clipShape(RoundedRectangle(cornerRadius: 10))
            .overlay {
                RoundedRectangle(cornerRadius: 10)
                    .stroke(MirTheme.Colors.panelBorder.opacity(0.65), lineWidth: 1)
            }
    }

    private func metricRow(_ title: String, _ value: String, _ color: Color) -> some View {
        HStack {
            Text(title)
                .font(.system(size: 10))
                .foregroundStyle(MirTheme.Colors.textSecondary)
            Spacer()
            Text(value)
                .font(.system(size: 10, weight: .bold, design: .monospaced))
                .foregroundStyle(color)
        }
    }

    private func sensor(_ name: String, value: String, unit: String, good: Bool) -> some View {
        HStack(spacing: 7) {
            Circle()
                .fill(good ? MirTheme.Colors.success : MirTheme.Colors.error)
                .frame(width: 5, height: 5)
            Text(name)
                .font(.system(size: 9, weight: .bold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textSecondary)
            Spacer()
            Text(value)
                .font(.system(size: 10, weight: .bold, design: .monospaced))
            Text(unit)
                .font(.system(size: 9))
                .foregroundStyle(MirTheme.Colors.textTertiary)
        }
    }
}
