import SwiftUI

/// Spatial project launcher used by the MIR 4D start experience.
/// It intentionally contains presentation only; project/session operations remain
/// owned by MIR4DProjectCommands and MIR4DProjectSession.
struct MIR4DProjectWindowHub: View {
    @EnvironmentObject private var appState: CADAppState
    let recentProjects: [MIR4DRecentProject]
    let canContinue: Bool
    let continueProject: MIR4DRecentProject?
    let onNewProject: () -> Void
    let onOpenProject: () -> Void
    let onRecentProjects: () -> Void
    let onContinue: (MIR4DRecentProject) -> Void

    var body: some View {
        ZStack {
            ambientBackground

            VStack(spacing: 22) {
                title
                mainWindow
                secondaryWindows
            }
            .padding(28)
            .frame(maxWidth: 980, maxHeight: 760)
        }
    }

    private var ambientBackground: some View {
        ZStack {
            Color.black
            Circle()
                .fill(Color.cyan.opacity(0.055))
                .frame(width: 520, height: 520)
                .blur(radius: 90)
                .offset(x: -260, y: -180)
            Circle()
                .fill(Color.blue.opacity(0.045))
                .frame(width: 480, height: 480)
                .blur(radius: 100)
                .offset(x: 300, y: 220)
        }
        .ignoresSafeArea()
    }

    private var title: some View {
        VStack(spacing: 5) {
            Text("МИР 4D")
                .font(.system(size: 30, weight: .bold, design: .rounded))
                .foregroundStyle(.white)
            Text("Пространство инженера")
                .font(.system(size: 12, weight: .medium))
                .foregroundStyle(.white.opacity(0.48))
        }
    }

    private var mainWindow: some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack {
                Text("ПРОЕКТ")
                    .font(.system(size: 10, weight: .bold, design: .rounded))
                    .tracking(1.4)
                    .foregroundStyle(.cyan.opacity(0.8))
                Spacer()
                Circle()
                    .fill(.green)
                    .frame(width: 6, height: 6)
                Text("готово")
                    .font(.system(size: 10))
                    .foregroundStyle(.secondary)
            }

            HStack(spacing: 14) {
                actionWindow(
                    icon: "plus",
                    title: "Новый проект",
                    subtitle: "Начать создание",
                    accent: .cyan
                ) {
                    onNewProject()
                }

                if canContinue, let project = continueProject {
                    actionWindow(
                        icon: "arrow.right",
                        title: "Продолжить",
                        subtitle: project.name,
                        accent: .blue
                    ) {
                        onContinue(project)
                    }
                } else {
                    actionWindow(
                        icon: "folder",
                        title: "Открыть проект",
                        subtitle: "Выбрать .mir4d",
                        accent: .blue
                    ) {
                        onOpenProject()
                    }
                }
            }
        }
        .padding(22)
        .background(windowBackground(cornerRadius: 22))
    }

    private var secondaryWindows: some View {
        HStack(spacing: 14) {
            smallWindow(icon: "folder", title: "Открыть проект", subtitle: "Проекты и файлы") {
                onOpenProject()
            }
            smallWindow(icon: "clock", title: "Недавние", subtitle: recentProjects.isEmpty ? "Нет проектов" : "\(recentProjects.count) проектов") {
                onRecentProjects()
            }
        }
    }

    private func actionWindow(
        icon: String,
        title: String,
        subtitle: String,
        accent: Color,
        action: @escaping () -> Void
    ) -> some View {
        Button(action: action) {
            VStack(alignment: .leading, spacing: 14) {
                Image(systemName: icon)
                    .font(.system(size: 24, weight: .medium))
                    .foregroundStyle(accent)
                    .frame(width: 44, height: 44)
                    .background(accent.opacity(0.10), in: RoundedRectangle(cornerRadius: 12))

                Spacer(minLength: 2)

                Text(title)
                    .font(.system(size: 16, weight: .semibold))
                    .foregroundStyle(.white)
                Text(subtitle)
                    .font(.system(size: 11))
                    .foregroundStyle(.white.opacity(0.48))
            }
            .frame(maxWidth: .infinity, minHeight: 160, alignment: .topLeading)
            .padding(18)
            .background(Color.white.opacity(0.035), in: RoundedRectangle(cornerRadius: 16))
            .overlay(RoundedRectangle(cornerRadius: 16).stroke(accent.opacity(0.16), lineWidth: 1))
        }
        .buttonStyle(.plain)
    }

    private func smallWindow(
        icon: String,
        title: String,
        subtitle: String,
        action: @escaping () -> Void
    ) -> some View {
        Button(action: action) {
            HStack(spacing: 12) {
                Image(systemName: icon)
                    .font(.system(size: 16))
                    .foregroundStyle(.cyan.opacity(0.85))
                VStack(alignment: .leading, spacing: 3) {
                    Text(title)
                        .font(.system(size: 12, weight: .semibold))
                        .foregroundStyle(.white)
                    Text(subtitle)
                        .font(.system(size: 10))
                        .foregroundStyle(.white.opacity(0.42))
                }
                Spacer()
                Image(systemName: "chevron.right")
                    .font(.system(size: 9, weight: .bold))
                    .foregroundStyle(.secondary)
            }
            .padding(16)
            .frame(maxWidth: .infinity, minHeight: 62)
            .background(windowBackground(cornerRadius: 14))
        }
        .buttonStyle(.plain)
    }

    private func windowBackground(cornerRadius: CGFloat) -> some View {
        RoundedRectangle(cornerRadius: cornerRadius)
            .fill(.ultraThinMaterial.opacity(0.42))
            .overlay(
                RoundedRectangle(cornerRadius: cornerRadius)
                    .stroke(Color.white.opacity(0.10), lineWidth: 1)
            )
            .shadow(color: .black.opacity(0.35), radius: 24, y: 12)
    }
}
