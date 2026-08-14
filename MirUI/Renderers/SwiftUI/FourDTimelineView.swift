import SwiftUI

struct FourDTimelineView: View {
    @ObservedObject var appState: CADAppState

    private struct Track: Identifiable {
        let id: String
        let ru: String
        let en: String
        let color: Color
        let span: ClosedRange<Double>
        let keyframes: [Double]
    }

    private var tracks: [Track] {
        var result: [Track] = [
            Track(id: "geometry", ru: "Геометрия", en: "Geometry", color: MirTheme.Colors.timeline, span: 0...7.5, keyframes: [1.0, 3.2, 5.8]),
            Track(id: "materials", ru: "Материалы", en: "Materials", color: MirTheme.Colors.simulationThermal, span: 1.5...8.4, keyframes: [2.0, 6.1]),
            Track(id: "simulation", ru: "Физика", en: "Physics", color: MirTheme.Colors.simulation, span: 2.0...10.0, keyframes: [2.0, 4.0, 7.0, 9.0]),
            Track(id: "events", ru: "События", en: "Events", color: MirTheme.Colors.event, span: 0.8...9.2, keyframes: [0.8, 3.8, 7.2, 9.2])
        ]

        if appState.workbench == .assembly {
            result.insert(
                Track(id: "assembly", ru: "Кинематика", en: "Kinematics", color: MirTheme.Colors.accentBright, span: 0.5...9.5, keyframes: [1.2, 4.5, 7.8]),
                at: 2
            )
        }

        return result
    }

    var body: some View {
        VStack(spacing: 0) {
            header
            scrubber
            timelineRuler
            tracksView
        }
        .background(MirTheme.Colors.panel)
        .overlay(alignment: .top) {
            Rectangle()
                .fill(MirTheme.Colors.border)
                .frame(height: 1)
        }
    }

    private var header: some View {
        HStack(spacing: MirTheme.Spacing.sm) {
            Button { appState.resetTime() } label: {
                Image(systemName: "backward.end.fill")
            }
            .help(localized("Начало", "Start"))

            Button { appState.stepBackward() } label: {
                Image(systemName: "backward.fill")
            }
            .help(localized("Шаг назад", "Step backward"))

            Button { appState.togglePlayback() } label: {
                Image(systemName: appState.isPlaying ? "pause.fill" : "play.fill")
            }
            .foregroundStyle(MirTheme.Colors.time)
            .help(localized("Воспроизвести", "Play"))

            Button { appState.stepForward() } label: {
                Image(systemName: "forward.fill")
            }
            .help(localized("Шаг вперёд", "Step forward"))

            Button { appState.finishTime() } label: {
                Image(systemName: "forward.end.fill")
            }
            .help(localized("Конец", "End"))

            Divider().frame(height: 20)

            Image(systemName: "clock.arrow.circlepath")
                .foregroundStyle(MirTheme.Colors.time)

            Text("4D")
                .font(MirTheme.Typography.bodySemibold)
                .foregroundStyle(MirTheme.Colors.textPrimary)

            Text(String(format: "T = %.3f s", appState.currentTime))
                .font(MirTheme.Typography.numeric)
                .foregroundStyle(MirTheme.Colors.keyframe)

            Spacer()

            Button {
                appState.createTimeScenario()
            } label: {
                Label(localized("Сценарий", "Scenario"), systemImage: "plus")
            }
            .buttonStyle(.borderless)
            .help(localized("Создать сценарий", "Create scenario"))

            Button {
                appState.createTimeBranch()
            } label: {
                Label(localized("Ветка", "Branch"), systemImage: "arrow.triangle.branch")
            }
            .buttonStyle(.borderless)
            .help(localized("Создать ветку", "Create branch"))

            Text(appState.timeState.scenarioID)
                .font(.system(size: 9, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .lineLimit(1)

            Text("/")
                .foregroundStyle(MirTheme.Colors.textTertiary)

            Text(appState.timeState.branchID)
                .font(.system(size: 9, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .lineLimit(1)
        }
        .buttonStyle(.plain)
        .foregroundStyle(MirTheme.Colors.textSecondary)
        .padding(.horizontal, MirTheme.Spacing.lg)
        .frame(height: 42)
    }

    private var scrubber: some View {
        HStack(spacing: MirTheme.Spacing.md) {
            Text("T")
                .font(MirTheme.Typography.numeric)
                .foregroundStyle(MirTheme.Colors.time)
                .frame(width: 24)

            Slider(
                value: Binding(
                    get: { appState.currentTime },
                    set: { appState.seek($0) }
                ),
                in: appState.timeState.start...appState.timeState.end
            )
            .tint(MirTheme.Colors.timeline)

            Text(String(format: "%.3f", appState.currentTime))
                .font(MirTheme.Typography.numeric)
                .foregroundStyle(MirTheme.Colors.textPrimary)
                .frame(width: 58, alignment: .trailing)
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .padding(.vertical, MirTheme.Spacing.xs)
        .background(MirTheme.Colors.surface.opacity(0.65))
    }

    private var timelineRuler: some View {
        HStack(spacing: 0) {
            Text(appState.ui.language == .russian ? "Дорожки" : "Tracks")
                .font(MirTheme.Typography.caption)
                .foregroundStyle(MirTheme.Colors.textTertiary)
                .frame(width: 150, alignment: .leading)

            GeometryReader { geometry in
                ZStack(alignment: .leading) {
                    HStack(spacing: 0) {
                        ForEach(0...10, id: \.self) { index in
                            VStack(spacing: 2) {
                                Rectangle()
                                    .fill(MirTheme.Colors.borderStrong)
                                    .frame(width: 1, height: index.isMultiple(of: 5) ? 9 : 5)
                                Text("\(index)")
                                    .font(.system(size: 8, design: .monospaced))
                                    .foregroundStyle(MirTheme.Colors.textTertiary)
                            }
                            .frame(maxWidth: .infinity, alignment: .leading)
                        }
                    }

                    playhead(in: geometry.size.width)
                }
            }
        }
        .padding(.horizontal, MirTheme.Spacing.lg)
        .frame(height: 28)
        .background(Color.black.opacity(0.12))
    }

    private var tracksView: some View {
        ScrollView(.vertical, showsIndicators: false) {
            VStack(spacing: 0) {
                ForEach(tracks) { track in
                    HStack(spacing: 0) {
                        HStack(spacing: 7) {
                            Circle()
                                .fill(track.color)
                                .frame(width: 7, height: 7)
                            Text(localizedTrackName(track))
                                .font(MirTheme.Typography.caption)
                                .foregroundStyle(MirTheme.Colors.textSecondary)
                        }
                        .frame(width: 150, alignment: .leading)

                        GeometryReader { geometry in
                            ZStack(alignment: .leading) {
                                Rectangle()
                                    .fill(MirTheme.Colors.border.opacity(0.25))
                                    .frame(height: 1)
                                    .offset(y: 15)

                                RoundedRectangle(cornerRadius: 3)
                                    .fill(track.color.opacity(0.30))
                                    .frame(
                                        width: geometry.size.width * normalizedWidth(track.span),
                                        height: 10
                                    )
                                    .offset(
                                        x: geometry.size.width * normalizedPosition(track.span.lowerBound),
                                        y: 10
                                    )

                                ForEach(track.keyframes, id: \.self) { time in
                                    keyframe(
                                        x: geometry.size.width * normalizedPosition(time),
                                        color: track.color
                                    )
                                }
                            }
                        }
                    }
                    .frame(height: 34)
                }
            }
            .padding(.horizontal, MirTheme.Spacing.lg)
            .padding(.bottom, MirTheme.Spacing.sm)
        }
        .frame(minHeight: 140)
    }

    private func playhead(in width: CGFloat) -> some View {
        Rectangle()
            .fill(MirTheme.Colors.keyframe)
            .frame(width: 2, height: 18)
            .offset(x: max(0, min(width - 2, width * appState.timeState.normalizedProgress)))
            .animation(.linear(duration: 0.05), value: appState.currentTime)
    }

    private func keyframe(x: CGFloat, color: Color) -> some View {
        Circle()
            .fill(color)
            .frame(width: 7, height: 7)
            .overlay(Circle().stroke(color.opacity(0.35), lineWidth: 3))
            .offset(x: x, y: 10)
    }

    private func normalizedPosition(_ time: Double) -> Double {
        let range = appState.timeState.end - appState.timeState.start
        guard range > 0 else { return 0 }
        return min(max((time - appState.timeState.start) / range, 0), 1)
    }

    private func normalizedWidth(_ span: ClosedRange<Double>) -> Double {
        max(0.02, normalizedPosition(span.upperBound) - normalizedPosition(span.lowerBound))
    }

    private func localizedTrackName(_ track: Track) -> String {
        appState.ui.language == .russian ? track.ru : track.en
    }

    private func localized(_ ru: String, _ en: String) -> String {
        appState.ui.language == .russian ? ru : en
    }
}
