import SwiftUI

struct TimelinePanelView: View {
    @ObservedObject var appState: CADAppState
    @State private var viewMode = "День"

    private let tasks: [(wbs: String, name: String, duration: String, progress: Int, color: Color, offset: CGFloat, width: CGFloat)] = [
        ("1", "Концепция изделия", "5", 100, Color(hex: "4D8DFF"), 0.02, 0.22),
        ("1.1", "Эскизное проектирование", "4", 100, Color(hex: "36D98C"), 0.18, 0.16),
        ("1.2", "3D-моделирование корпуса", "6", 100, Color(hex: "4D8DFF"), 0.02, 0.24),
        ("1.3", "Проектирование визора", "4", 75, Color(hex: "4D8DFF"), 0.30, 0.18),
        ("1.4", "CAE-анализ", "3", 20, Color(hex: "36D98C"), 0.44, 0.14),
        ("1.5", "4D-моделирование корпуса", "4", 0, Color(hex: "FFB84D"), 0.56, 0.16),
        ("1.6", "Подготовка CAM", "3", 45, Color(hex: "4D8DFF"), 0.70, 0.18)
    ]

    private var russian: Bool { appState.ui.language == .russian }

    var body: some View {
        VStack(spacing: 0) {
            timelineHeader
            timeSlider
            legend
            gantt
        }
        .background(MirTheme.Colors.panel)
        .overlay(alignment: .top) {
            Rectangle().fill(MirTheme.Colors.panelBorder).frame(height: 1)
        }
    }

    private var timelineHeader: some View {
        HStack(spacing: 7) {
            HStack(spacing: 7) {
                Image(systemName: "clock.arrow.circlepath")
                    .font(.system(size: 12, weight: .semibold))
                    .foregroundStyle(MirTheme.Colors.accentBright)
                Text(russian ? "4D Время" : "4D Time")
                    .font(MirTheme.Typography.bodySemibold)
                    .foregroundStyle(MirTheme.Colors.textPrimary)
            }

            Divider().frame(height: 18)

            playbackButton("backward.end.fill", help: russian ? "В начало" : "To start") { appState.resetTime() }
            playbackButton("backward.fill", help: russian ? "Назад" : "Step backward") { appState.stepBackward() }
            playbackButton(appState.isPlaying ? "pause.fill" : "play.fill", active: appState.isPlaying, help: russian ? "Воспроизведение" : "Play") { appState.togglePlayback() }
            playbackButton("forward.fill", help: russian ? "Вперёд" : "Step forward") { appState.stepForward() }
            playbackButton("forward.end.fill", help: russian ? "В конец" : "To end") { appState.finishTime() }

            Text(String(format: "T = %.3f s", appState.currentTime))
                .font(.system(size: 11, weight: .semibold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.warning)
                .padding(.horizontal, 7)
                .padding(.vertical, 4)
                .background(MirTheme.Colors.surfaceRaised, in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))

            Spacer(minLength: 10)

            HStack(spacing: 2) {
                ForEach(["День", "Неделя", "Месяц"], id: \.self) { mode in
                    Button(russian ? mode : englishMode(mode)) { viewMode = mode }
                        .buttonStyle(.plain)
                        .font(MirTheme.Typography.status)
                        .padding(.horizontal, 9)
                        .padding(.vertical, 5)
                        .background(viewMode == mode ? MirTheme.Colors.accentSoft : Color.clear)
                        .foregroundStyle(viewMode == mode ? MirTheme.Colors.textPrimary : MirTheme.Colors.textTertiary)
                        .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
                }
            }
            .padding(2)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.65), in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
        }
        .padding(.horizontal, MirTheme.Spacing.md)
        .frame(height: 44)
        .background(MirTheme.Colors.surface.opacity(0.7))
    }

    private func playbackButton(_ image: String, active: Bool = false, help: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: image)
                .font(.system(size: 10, weight: .semibold))
                .frame(width: 26, height: 26)
                .background(active ? MirTheme.Colors.accentSoft : Color.clear)
                .foregroundStyle(active ? MirTheme.Colors.accentBright : MirTheme.Colors.textSecondary)
                .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain)
        .help(help)
    }

    private var timeSlider: some View {
        HStack(spacing: 10) {
            Text(String(format: "%.3f", appState.time.startTime))
                .font(.system(size: 9, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)

            Slider(value: Binding(get: { appState.currentTime }, set: { appState.seek($0) }), in: appState.time.startTime...appState.time.endTime)
                .tint(MirTheme.Colors.accent)

            Text(String(format: "%.3f s", appState.time.endTime))
                .font(.system(size: 9, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
        }
        .padding(.horizontal, MirTheme.Spacing.md)
        .frame(height: 32)
        .background(MirTheme.Colors.surface.opacity(0.55))
    }

    private var legend: some View {
        HStack(spacing: 14) {
            legendItem(color: Color(hex: "4D8DFF"), text: russian ? "Концепт" : "Concept")
            legendItem(color: Color(hex: "36D98C"), text: russian ? "3D-моделирование" : "3D Modeling")
            legendItem(color: Color(hex: "FFB84D"), text: russian ? "CAE-анализ" : "CAE Analysis")
            legendItem(color: Color(hex: "FF7B4D"), text: russian ? "4D-моделирование" : "4D Modeling")
            Spacer()
        }
        .font(MirTheme.Typography.status)
        .foregroundStyle(MirTheme.Colors.textTertiary)
        .padding(.horizontal, MirTheme.Spacing.md)
        .padding(.vertical, 5)
    }

    private var gantt: some View {
        HStack(spacing: 0) {
            VStack(spacing: 0) {
                ganttHeaderRow
                ForEach(tasks, id: \.wbs) { task in
                    HStack(spacing: 6) {
                        Text(task.wbs).frame(width: 28, alignment: .leading).foregroundStyle(MirTheme.Colors.textTertiary)
                        Text(task.name).lineLimit(1).foregroundStyle(MirTheme.Colors.textPrimary)
                        Spacer()
                        Text(task.duration).frame(width: 32, alignment: .trailing).foregroundStyle(MirTheme.Colors.textTertiary)
                        Text("\(task.progress)").frame(width: 28, alignment: .trailing).foregroundStyle(task.progress == 100 ? MirTheme.Colors.success : task.progress > 0 ? MirTheme.Colors.warning : MirTheme.Colors.textTertiary)
                    }
                    .font(MirTheme.Typography.status)
                    .padding(.horizontal, 10)
                    .frame(height: 30)
                    .background(task.wbs == "1.3" ? MirTheme.Colors.accentSoft.opacity(0.35) : Color.clear)
                }
            }
            .frame(width: 280)
            .overlay(Rectangle().fill(MirTheme.Colors.panelBorder).frame(width: 1), alignment: .trailing)

            GeometryReader { geo in
                ZStack(alignment: .topLeading) {
                    VStack(spacing: 0) {
                        HStack {
                            ForEach(["1 июн", "5", "10", "15", "20", "25", "30"], id: \.self) { d in
                                Text(d).font(.system(size: 9)).foregroundStyle(MirTheme.Colors.textTertiary)
                                if d != "30" { Spacer() }
                            }
                        }
                        .padding(.horizontal, 8)
                        .frame(height: 28)

                        ForEach(tasks, id: \.wbs) { task in
                            ZStack(alignment: .leading) {
                                Color.clear.frame(height: 30)
                                RoundedRectangle(cornerRadius: MirTheme.Radius.small)
                                    .fill(LinearGradient(colors: [task.color, task.color.opacity(0.7)], startPoint: .leading, endPoint: .trailing))
                                    .frame(width: max(8, geo.size.width * task.width), height: 16)
                                    .offset(x: geo.size.width * task.offset)
                            }
                            .background(task.wbs == "1.3" ? MirTheme.Colors.accentSoft.opacity(0.25) : Color.clear)
                        }
                    }

                    Rectangle()
                        .fill(MirTheme.Colors.warning)
                        .frame(width: 2)
                        .offset(x: geo.size.width * CGFloat(appState.currentTime / max(appState.time.endTime, 0.001)))
                        .animation(.linear(duration: 0.05), value: appState.currentTime)
                        .overlay(alignment: .top) {
                            Text(String(format: "T %.2f", appState.currentTime))
                                .font(.system(size: 9, design: .monospaced))
                                .foregroundStyle(MirTheme.Colors.warning)
                                .padding(.horizontal, 5)
                                .padding(.vertical, 2)
                                .background(MirTheme.Colors.surfaceRaised, in: Capsule())
                                .offset(y: -1)
                        }
                }
            }
        }
        .background(MirTheme.Colors.surface.opacity(0.35))
    }

    private var ganttHeaderRow: some View {
        HStack {
            Text("WBS").frame(width: 28, alignment: .leading)
            Text(russian ? "Задача" : "Task")
            Spacer()
            Text(russian ? "Длит." : "Dur.").frame(width: 32, alignment: .trailing)
            Text("%").frame(width: 28, alignment: .trailing)
        }
        .font(MirTheme.Typography.caption)
        .foregroundStyle(MirTheme.Colors.textTertiary)
        .padding(.horizontal, 10)
        .frame(height: 28)
        .background(MirTheme.Colors.surfaceRaised.opacity(0.45))
    }

    private func legendItem(color: Color, text: String) -> some View {
        HStack(spacing: 4) {
            RoundedRectangle(cornerRadius: 2).fill(color).frame(width: 10, height: 10)
            Text(text)
        }
    }

    private func englishMode(_ russianMode: String) -> String {
        switch russianMode {
        case "День": return "Day"
        case "Неделя": return "Week"
        default: return "Month"
        }
    }
}
