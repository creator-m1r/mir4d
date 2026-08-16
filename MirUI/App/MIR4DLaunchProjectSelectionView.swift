import SwiftUI
import AppKit

/// Project Hub matching the MIR 4D launch concept: the diagnostics card stays
/// on the left while the project-selection door opens from the right.
struct MIR4DLaunchProjectSelectionView: View {
    @EnvironmentObject private var appState: CADAppState
    @ObservedObject var diagnostic: MIR4DBootCoordinator

    @State private var showOpenProject = false
    @State private var showNewProject = false
    @State private var appeared = false

    private var recentProjects: [MIR4DRecentProject] {
        MIR4DProjectSession.shared.recentProjectsList()
    }

    private var continueProject: MIR4DRecentProject? { recentProjects.first }

    var body: some View {
        GeometryReader { proxy in
            ZStack(alignment: .leading) {
                Color.black.ignoresSafeArea()
                MIR4DStartupMotionLayer().opacity(0.20)

                HStack(spacing: 0) {
                    brandCard
                        .frame(width: min(430, proxy.size.width * 0.34))
                        .padding(.leading, max(34, proxy.size.width * 0.055))
                    Spacer(minLength: 30)
                    projectDoor
                        .frame(width: min(760, proxy.size.width * 0.57))
                        .padding(.trailing, max(20, proxy.size.width * 0.025))
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
        }
        .sheet(isPresented: $showOpenProject) {
            MIR4DProjectOpenView().environmentObject(appState)
        }
        .sheet(isPresented: $showNewProject) {
            MIR4DNewProjectView().environmentObject(appState)
        }
        .onAppear {
            withAnimation(.timingCurve(0.16, 0.82, 0.22, 1.0, duration: 0.85)) { appeared = true }
        }
    }

    private var brandCard: some View {
        VStack(spacing: 0) {
            Spacer(minLength: 30)
            VStack(spacing: 12) {
                MIR4DLaunchGeometricMark()
                Text("МИР 4D")
                    .font(.system(size: 38, weight: .medium, design: .rounded))
                    .tracking(1.8)
                    .foregroundStyle(.white)
                Text("Мечтай · Изобретай · Развивай")
                    .font(.system(size: 12))
                    .foregroundStyle(.white.opacity(0.52))
            }
            Spacer(minLength: 40)
            diagnostics.padding(.horizontal, 32).padding(.bottom, 32)
        }
        .frame(maxHeight: .infinity)
        .background(
            RoundedRectangle(cornerRadius: 12).fill(
                LinearGradient(
                    colors: [Color(red: 0.035, green: 0.055, blue: 0.085), Color(red: 0.010, green: 0.016, blue: 0.026)],
                    startPoint: .topLeading, endPoint: .bottomTrailing
                )
            )
        )
        .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.white.opacity(0.20), lineWidth: 1))
        .overlay(alignment: .trailing) {
            Rectangle()
                .fill(LinearGradient(colors: [.clear, .blue.opacity(0.72), .clear], startPoint: .top, endPoint: .bottom))
                .frame(width: 2).blur(radius: 4)
        }
        .shadow(color: .blue.opacity(0.14), radius: 34, x: 12, y: 0)
        .offset(x: appeared ? 0 : -90)
        .opacity(appeared ? 1 : 0)
    }

    private var diagnostics: some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack(spacing: 8) {
                Circle().fill(diagnostic.state == .failed ? .red : .blue).frame(width: 6, height: 6)
                Text("САМОИАГНОСТИКА")
                    .font(.system(size: 10, weight: .medium, design: .monospaced))
                    .foregroundStyle(.white.opacity(0.60))
                Spacer()
                Text("\(Int(diagnostic.progress * 100))%")
                    .font(.system(size: 10, weight: .medium, design: .monospaced))
                    .foregroundStyle(.white.opacity(0.72))
            }
            Text(diagnostic.currentTitle)
                .font(.system(size: 11, weight: .medium))
                .foregroundStyle(.white.opacity(0.82))
                .lineLimit(1)
            ProgressView(value: diagnostic.progress, total: 1)
                .tint(.blue)
                .animation(.easeInOut(duration: 0.2), value: diagnostic.progress)
            if diagnostic.warningCount > 0 {
                Text("\(diagnostic.warningCount) предупреждение")
                    .font(.system(size: 9, design: .monospaced))
                    .foregroundStyle(.yellow.opacity(0.8))
            }
        }
    }

    private var projectDoor: some View {
        VStack(alignment: .leading, spacing: 0) {
            projectHeader
            Divider().overlay(Color.white.opacity(0.08))
            HStack(alignment: .top, spacing: 0) {
                mainProjectColumn
                Divider().overlay(Color.white.opacity(0.08)).padding(.vertical, 26)
                scenariosColumn
            }
        }
        .frame(maxHeight: .infinity)
        .background(RoundedRectangle(cornerRadius: 10).fill(Color(red: 0.018, green: 0.024, blue: 0.036).opacity(0.97)))
        .overlay(RoundedRectangle(cornerRadius: 10).stroke(Color.white.opacity(0.10), lineWidth: 1))
        .overlay(alignment: .leading) {
            Rectangle()
                .fill(LinearGradient(colors: [.blue.opacity(0.85), .clear], startPoint: .leading, endPoint: .trailing))
                .frame(width: 2).shadow(color: .blue.opacity(0.9), radius: 9)
        }
        .shadow(color: .blue.opacity(0.16), radius: 28, x: -8, y: 0)
        .offset(x: appeared ? 0 : 90)
        .opacity(appeared ? 1 : 0)
    }

    private var projectHeader: some View {
        VStack(alignment: .leading, spacing: 5) {
            Text("МИР 4D")
                .font(.system(size: 24, weight: .medium, design: .rounded))
                .tracking(1.0).foregroundStyle(.white)
            Text("Инженерная среда моделирования")
                .font(.system(size: 10)).foregroundStyle(.white.opacity(0.46))
        }
        .padding(.horizontal, 28).padding(.vertical, 22)
    }

    private var mainProjectColumn: some View {
        VStack(alignment: .leading, spacing: 18) {
            Text("НАЧАЛО РАБОТЫ")
                .font(.system(size: 10, weight: .semibold, design: .monospaced))
                .foregroundStyle(.white.opacity(0.50))

            HStack(spacing: 8) {
                actionButton(title: "Продолжить", subtitle: "последний проект", icon: "arrow.forward.circle.fill", accent: true) {
                    guard let project = continueProject else { return }
                    MIR4DProjectCommands.shared.open(appState: appState, url: project.url)
                }
                actionButton(title: "Создать", subtitle: "проект", icon: "plus", accent: false) { showNewProject = true }
            }

            Button { showOpenProject = true } label: {
                HStack(spacing: 10) {
                    Image(systemName: "folder")
                    Text("Открыть проект .mir4d")
                    Spacer()
                    Image(systemName: "arrow.right")
                }
                .font(.system(size: 11, weight: .medium))
                .foregroundStyle(.white.opacity(0.86))
                .padding(.horizontal, 14).frame(height: 42)
                .background(RoundedRectangle(cornerRadius: 7).fill(Color.white.opacity(0.035)))
                .overlay(RoundedRectangle(cornerRadius: 7).stroke(Color.white.opacity(0.08), lineWidth: 1))
            }
            .buttonStyle(.plain)

            recentProjectsView
            Spacer(minLength: 20)

            HStack(spacing: 18) {
                smallFooter("gearshape", "Настройки")
                smallFooter("book", "Документация")
            }
            .foregroundStyle(.white.opacity(0.45))
        }
        .padding(.horizontal, 26).padding(.vertical, 26)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
    }

    private var recentProjectsView: some View {
        VStack(alignment: .leading, spacing: 9) {
            Text("НЕДАВНИЕ ПРОЕКТЫ")
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundStyle(.white.opacity(0.42))
            ForEach(Array(recentProjects.prefix(3))) { project in
                Button {
                    MIR4DProjectCommands.shared.open(appState: appState, url: project.url)
                } label: {
                    HStack(spacing: 9) {
                        Image(systemName: "doc").font(.system(size: 12)).foregroundStyle(.white.opacity(0.65))
                        VStack(alignment: .leading, spacing: 2) {
                            Text(project.name).font(.system(size: 10, weight: .medium)).foregroundStyle(.white.opacity(0.86)).lineLimit(1)
                            Text(project.lastOpened.formatted(date: .abbreviated, time: .shortened)).font(.system(size: 8)).foregroundStyle(.white.opacity(0.32))
                        }
                        Spacer()
                    }
                    .padding(.horizontal, 10).frame(height: 43)
                    .background(RoundedRectangle(cornerRadius: 6).fill(Color.white.opacity(0.035)))
                    .overlay(RoundedRectangle(cornerRadius: 6).stroke(Color.white.opacity(0.055), lineWidth: 1))
                }
                .buttonStyle(.plain)
            }
        }
    }

    private var scenariosColumn: some View {
        VStack(alignment: .leading, spacing: 24) {
            Text("СЦЕНАРИИ РАБОТЫ")
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundStyle(.white.opacity(0.42))
            scenario("40 Время", icon: "clock")
            scenario("Математика\nи Моделирование", icon: "function")
            scenario("Программирование\nи Скрипты", icon: "chevron.left.forwardslash.chevron.right")
            Spacer()
        }
        .padding(.horizontal, 24).padding(.vertical, 26)
        .frame(width: 220, maxHeight: .infinity, alignment: .topLeading)
    }

    private func scenario(_ text: String, icon: String) -> some View {
        HStack(alignment: .top, spacing: 10) {
            Image(systemName: icon).font(.system(size: 12)).foregroundStyle(.white.opacity(0.55)).frame(width: 18)
            Text(text).font(.system(size: 10)).foregroundStyle(.white.opacity(0.64)).fixedSize(horizontal: false, vertical: true)
        }
    }

    private func actionButton(title: String, subtitle: String, icon: String, accent: Bool, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            VStack(alignment: .leading, spacing: 5) {
                HStack { Image(systemName: icon); Spacer() }
                Text(title).font(.system(size: 11, weight: .semibold))
                Text(subtitle).font(.system(size: 9)).opacity(0.72)
            }
            .foregroundStyle(.white).padding(11)
            .frame(maxWidth: .infinity, minHeight: 72, alignment: .topLeading)
            .background(RoundedRectangle(cornerRadius: 7).fill(accent ? Color.blue.opacity(0.88) : Color.white.opacity(0.045)))
            .overlay(RoundedRectangle(cornerRadius: 7).stroke(Color.white.opacity(accent ? 0.22 : 0.08), lineWidth: 1))
        }
        .buttonStyle(.plain)
    }

    private func smallFooter(_ icon: String, _ title: String) -> some View {
        HStack(spacing: 5) { Image(systemName: icon); Text(title) }.font(.system(size: 9))
    }
}

struct MIR4DLaunchGeometricMark: View {
    @State private var glow = false

    var body: some View {
        ZStack {
            RoundedRectangle(cornerRadius: 5).stroke(Color.blue.opacity(glow ? 0.9 : 0.55), lineWidth: 2).frame(width: 43, height: 43).rotationEffect(.degrees(45))
            RoundedRectangle(cornerRadius: 4).stroke(Color.white.opacity(0.75), lineWidth: 1.2).frame(width: 30, height: 30).rotationEffect(.degrees(45))
            Rectangle().fill(Color.blue.opacity(0.8)).frame(width: 1.5, height: 25).rotationEffect(.degrees(45)).offset(x: 11, y: 11)
        }
        .frame(width: 76, height: 76)
        .shadow(color: .blue.opacity(glow ? 0.45 : 0.16), radius: 15)
        .onAppear {
            withAnimation(.easeInOut(duration: 1.8).repeatForever(autoreverses: true)) { glow = true }
        }
    }
}
