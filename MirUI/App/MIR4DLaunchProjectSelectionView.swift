import SwiftUI
import AppKit

/// MIR 4D start experience.
///
/// The launch screen is deliberately a thin coordinator. Presentation is owned
/// by MIR4DProjectWindowHub / MIR4DProjectWindowHost while project operations
/// remain owned by MIR4DProjectCommands and MIR4DProjectSession.
struct MIR4DLaunchProjectSelectionView: View {
    @EnvironmentObject private var appState: CADAppState
    @ObservedObject var diagnostic: MIR4DBootCoordinator
    @Binding var isLeaving: Bool

    @State private var showOpenProject = false
    @State private var showNewProject = false
    @State private var newProjectPreset: CADWorkbench?
    @State private var newProjectDefaultName: String?
    @State private var showRecentProjects = false
    @State private var showWhatsNew = false
    @State private var recentRefreshToken = UUID()
    @State private var viewport: CGSize = .zero

    private var commands: MIR4DProjectCommands { .shared }

    private var recentProjects: [MIR4DRecentProject] {
        _ = recentRefreshToken
        return MIR4DProjectSession.shared.recentProjectsList()
    }

    private var continueProject: MIR4DRecentProject? { recentProjects.first }

    private var canContinue: Bool {
        guard let project = continueProject else { return false }
        return MIR4DProjectSession.shared.availability(of: project) == .available
    }

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                startupBackground

                MIR4DProjectWindowHub(
                    recentProjects: recentProjects,
                    canContinue: canContinue,
                    continueProject: continueProject,
                    onNewProject: { presentNewProject() },
                    onOpenProject: { presentOpenProject() },
                    onRecentProjects: { presentRecentProjects() },
                    onContinue: { project in
                        commands.open(appState: appState, url: project.url)
                    }
                )
                .environmentObject(appState)
                .opacity(isLeaving ? 0.2 : 1)
                .offset(x: isLeaving ? proxy.size.width : 0)

                if showRecentProjects {
                    MIR4DProjectWindowHost(
                        title: "Недавние проекты",
                        subtitle: "Ваше инженерное пространство",
                        isPresented: true,
                        onDismiss: { closeRecentProjects() }
                    ) {
                        MIR4DRecentProjectsWindow(
                            projects: recentProjects,
                            onOpen: { project in
                                closeRecentProjects()
                                commands.open(appState: appState, url: project.url)
                            },
                            onClose: { closeRecentProjects() }
                        )
                    }
                }

                if showOpenProject {
                    MIR4DProjectWindowHost(
                        title: "Открыть проект",
                        subtitle: "Выберите проект МИР 4D",
                        isPresented: true,
                        onDismiss: { closeOpenProject() }
                    ) {
                        MIR4DProjectOpenView()
                            .environmentObject(appState)
                    }
                }

                if showNewProject {
                    MIR4DProjectWindowHost(
                        title: "Новый проект",
                        subtitle: "Создание инженерного пространства",
                        isPresented: true,
                        onDismiss: { closeNewProject() }
                    ) {
                        MIR4DNewProjectView(
                            presetWorkbench: newProjectPreset,
                            defaultName: newProjectDefaultName
                        )
                        .environmentObject(appState)
                    }
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .animation(.spring(response: 0.34, dampingFraction: 0.86), value: showOpenProject)
            .animation(.spring(response: 0.34, dampingFraction: 0.86), value: showNewProject)
            .animation(.spring(response: 0.34, dampingFraction: 0.86), value: showRecentProjects)
            .animation(.timingCurve(0.18, 0.80, 0.22, 1.0, duration: 0.68), value: isLeaving)
            .onAppear {
                viewport = proxy.size
            }
            .onChange(of: proxy.size) { _, newSize in
                viewport = newSize
            }
            .onDrop(of: [.fileURL], isTargeted: nil) { providers in
                guard let provider = providers.first(where: {
                    $0.hasItemConformingToTypeIdentifier("public.file-url")
                }) else { return false }

                provider.loadItem(forTypeIdentifier: "public.file-url", options: nil) { item, _ in
                    guard let data = item as? Data,
                          let url = URL(dataRepresentation: data, relativeTo: nil),
                          url.pathExtension.lowercased() == "mir4d" else { return }
                    DispatchQueue.main.async {
                        commands.open(appState: appState, url: url)
                    }
                }
                return true
            }
        }
        .sheet(isPresented: $showWhatsNew) {
            whatsNewSheet
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in
            refreshRecents()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectSaved)) { _ in
            refreshRecents()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectClosed)) { _ in
            refreshRecents()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRequestNewProject)) { _ in
            presentNewProject()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DOpenProject)) { _ in
            presentOpenProject()
        }
    }

    private var startupBackground: some View {
        ZStack {
            Color.black.ignoresSafeArea()
            MIR4DStartupMotionLayer().opacity(0.10)
            RadialGradient(
                colors: [Color.cyan.opacity(0.055), Color.clear],
                center: .topLeading,
                startRadius: 20,
                endRadius: max(viewport.width, viewport.height) * 0.8
            )
            .ignoresSafeArea()
        }
    }

    private func presentNewProject(preset: CADWorkbench? = nil, defaultName: String? = nil) {
        newProjectPreset = preset
        newProjectDefaultName = defaultName
        showOpenProject = false
        showRecentProjects = false
        withAnimation { showNewProject = true }
    }

    private func presentOpenProject() {
        newProjectPreset = nil
        newProjectDefaultName = nil
        showNewProject = false
        showRecentProjects = false
        withAnimation { showOpenProject = true }
    }

    private func presentRecentProjects() {
        showOpenProject = false
        showNewProject = false
        withAnimation { showRecentProjects = true }
    }

    private func closeNewProject() {
        withAnimation { showNewProject = false }
    }

    private func closeOpenProject() {
        withAnimation { showOpenProject = false }
    }

    private func closeRecentProjects() {
        withAnimation { showRecentProjects = false }
    }

    private func refreshRecents() {
        recentRefreshToken = UUID()
    }

    private var whatsNewSheet: some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack {
                Text("Что нового")
                    .font(.title2.weight(.semibold))
                Spacer()
                Button("Готово") { showWhatsNew = false }
            }
            Divider()
            Text("МИР 4D постепенно переходит к пространственному интерфейсу: проектные действия открываются как окна внутри инженерного пространства.")
                .foregroundStyle(.secondary)
            Spacer()
        }
        .padding(24)
        .frame(minWidth: 420, minHeight: 260)
    }
}
