import SwiftUI
import AppKit

/// MIR 4D start hub.
///
/// UX principle: the start screen answers three questions immediately:
/// 1. Continue the work I was doing.
/// 2. Start a new engineering project.
/// 3. Find/open an existing project.
///
/// Advanced laboratories and secondary actions remain available, but never
/// compete visually with the primary workflow.
struct MIR4DLaunchProjectSelectionView: View {
    @EnvironmentObject private var appState: CADAppState
    @ObservedObject var diagnostic: MIR4DBootCoordinator
    @Binding var isLeaving: Bool

    @State private var showOpenProject = false
    @State private var showNewProject = false
    @State private var newProjectPreset: CADWorkbench?
    @State private var newProjectDefaultName: String?
    @State private var recentFilter = ""
    @State private var viewport: CGSize = .zero
    @State private var appeared = false
    @State private var recentRefreshToken = UUID()
    @State private var showAdvanced = false
    @State private var showWhatsNew = false
    @State private var autoOpenLastProject = MIR4DProjectSession.shared.isAutoOpenLastProjectEnabled

    private var commands: MIR4DProjectCommands { .shared }

    private var appVersion: String {
        Bundle.main.infoDictionary?["CFBundleShortVersionString"] as? String
        ?? Bundle.main.infoDictionary?["CFBundleVersion"] as? String
        ?? "0.0"
    }

    private var recentProjects: [MIR4DRecentProject] {
        _ = recentRefreshToken
        return MIR4DProjectSession.shared.recentProjectsList()
    }

    private var continueProject: MIR4DRecentProject? { recentProjects.first }

    private var canContinue: Bool {
        guard let project = continueProject else { return false }
        return MIR4DProjectSession.shared.availability(of: project) == .available
    }

    private let templates: [CADProjectTemplate] = [
        CADProjectTemplate(title: "Деталь", subtitle: "Создать 3D модель", icon: "cube", workbench: .model),
        CADProjectTemplate(title: "Сборка", subtitle: "Соединить компоненты", icon: "square.stack.3d.up", workbench: .assembly),
        CADProjectTemplate(title: "Чертёж", subtitle: "Создать документацию", icon: "doc.text", workbench: .drawing)
    ]

    private let laboratories: [CADLaboratory] = [
        CADLaboratory(title: "4D", subtitle: "Время и сценарии", icon: "clock.arrow.2.circlepath", workbench: .fourD),
        CADLaboratory(title: "Расчёты", subtitle: "Математика и физика", icon: "function", workbench: .simulation),
        CADLaboratory(title: "Программирование", subtitle: "Логика и автоматизация", icon: "chevron.left.forwardslash.chevron.right", workbench: .model),
        CADLaboratory(title: "Знания", subtitle: "Обучение и справка", icon: "books.vertical", workbench: .model)
    ]

    private let cardGradients: [[Color]] = [
        [Color(red: 0.30, green: 0.55, blue: 1.0), Color(red: 0.05, green: 0.10, blue: 0.24)],
        [Color(red: 0.26, green: 0.85, blue: 1.0), Color(red: 0.04, green: 0.13, blue: 0.21)],
        [Color(red: 0.42, green: 0.48, blue: 0.70), Color(red: 0.06, green: 0.08, blue: 0.15)]
    ]

    private var isCompact: Bool {
        viewport.width < 900 || viewport.height < 650
    }

    private var sideMargin: CGFloat { isCompact ? 16 : max(28, viewport.width * 0.045) }

    var body: some View {
        GeometryReader { proxy in
            let _ = syncViewport(proxy.size)

            ZStack {
                Color.black.ignoresSafeArea()
                MIR4DStartupMotionLayer().opacity(0.10)

                hub
                    .frame(maxWidth: 1180, maxHeight: .infinity)
                    .padding(.horizontal, sideMargin)
                    .padding(.vertical, isCompact ? 12 : 24)
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
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .offset(x: isLeaving ? proxy.size.width : 0)
            .opacity(isLeaving ? 0.25 : 1)
            .animation(.timingCurve(0.18, 0.80, 0.22, 1.0, duration: 0.68), value: isLeaving)
        }
        .sheet(isPresented: $showOpenProject) {
            MIR4DProjectOpenView().environmentObject(appState)
        }
        .sheet(isPresented: $showNewProject) {
            MIR4DNewProjectView(
                presetWorkbench: newProjectPreset,
                defaultName: newProjectDefaultName
            )
            .environmentObject(appState)
        }
        .sheet(isPresented: $showWhatsNew) { whatsNewSheet }
        .onAppear {
            withAnimation(.easeOut(duration: 0.7)) { appeared = true }
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in refreshRecents() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectSaved)) { _ in refreshRecents() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectClosed)) { _ in refreshRecents() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRequestNewProject)) { _ in presentNewProject() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DOpenProject)) { _ in presentOpenProject() }
    }

    private var hub: some View {
        VStack(spacing: 0) {
            header
            Divider().overlay(Color.white.opacity(0.08))
            ScrollView(.vertical, showsIndicators: false) {
                VStack(alignment: .leading, spacing: isCompact ? 16 : 22) {
                    hero
                    primaryWorkflows
                    recentProjectsSection
                    secondaryTools
                }
                .padding(.horizontal, isCompact ? 16 : 24)
                .padding(.vertical, isCompact ? 16 : 24)
            }
        }
        .background(
            RoundedRectangle(cornerRadius: 18)
                .fill(
                    LinearGradient(
                        colors: [
                            Color(red: 0.035, green: 0.055, blue: 0.085),
                            Color(red: 0.008, green: 0.012, blue: 0.020)
                        ],
                        startPoint: .topLeading,
                        endPoint: .bottomTrailing
                    )
                )
        )
        .overlay(
            RoundedRectangle(cornerRadius: 18)
                .stroke(Color.white.opacity(0.13), lineWidth: 1)
        )
        .shadow(color: Color(red: 0.30, green: 0.55, blue: 1.0).opacity(0.10), radius: 35, y: 8)
        .opacity(appeared ? 1 : 0)
        .scaleEffect(appeared ? 1 : 0.985)
    }

    // MARK: - Header

    private var header: some View {
        HStack(spacing: 14) {
            MIR4DLaunchGeometricMark()
                .scaleEffect(0.48)
                .frame(width: 42, height: 42)

            VStack(alignment: .leading, spacing: 2) {
                Text("МИР 4D")
                    .font(.system(size: 17, weight: .bold, design: .rounded))
                    .foregroundStyle(.white)
                Text("Инженерная среда")
                    .font(.system(size: 10))
                    .foregroundStyle(.secondary)
            }

            Spacer()

            bootStatus

            Menu {
                Button("Что нового") { showWhatsNew = true }
                Toggle("Автооткрывать последний проект", isOn: Binding(
                    get: { autoOpenLastProject },
                    set: {
                        autoOpenLastProject = $0
                        MIR4DProjectSession.shared.isAutoOpenLastProjectEnabled = $0
                    }
                ))
            } label: {
                Image(systemName: "ellipsis.circle")
                    .font(.system(size: 16))
                    .foregroundStyle(.secondary)
                    .frame(width: 28, height: 28)
            }
            .menuStyle(.borderlessButton)
            .help("Дополнительные настройки")

            Button("Открыть") { presentOpenProject() }
                .buttonStyle(MIR4DTopBarButtonStyle(prominent: false))

            Button("Новый проект") { presentNewProject() }
                .buttonStyle(MIR4DTopBarButtonStyle(prominent: true))
        }
        .padding(.horizontal, isCompact ? 16 : 24)
        .frame(height: isCompact ? 58 : 66)
    }

    private var bootStatus: some View {
        HStack(spacing: 7) {
            Circle()
                .fill(diagnostic.state == .failed ? .red : (diagnostic.state == .warning ? .yellow : .green))
                .frame(width: 7, height: 7)
            Text(diagnostic.state == .ready ? "Готово" : (diagnostic.state == .failed ? "Ошибка" : "Проверка…"))
                .font(.system(size: 10, weight: .medium))
                .foregroundStyle(.secondary)
        }
        .help("Самодиагностика: \(Int(diagnostic.progress * 100))% · \(diagnostic.currentTitle)")
    }

    // MARK: - Hero

    private var hero: some View {
        HStack(alignment: .center, spacing: 18) {
            VStack(alignment: .leading, spacing: 8) {
                Text("С чего начнём?")
                    .font(.system(size: isCompact ? 25 : 32, weight: .bold, design: .rounded))
                    .foregroundStyle(.white)
                Text("Создайте инженерную модель, продолжите проект или откройте существующий.")
                    .font(.system(size: 13))
                    .foregroundStyle(.secondary)
                    .fixedSize(horizontal: false, vertical: true)
            }

            Spacer(minLength: 10)

            if canContinue, let project = continueProject {
                Button {
                    commands.open(appState: appState, url: project.url)
                } label: {
                    HStack(spacing: 11) {
                        Image(systemName: "arrow.forward.circle.fill")
                            .font(.system(size: 22))
                        VStack(alignment: .leading, spacing: 2) {
                            Text("Продолжить")
                                .font(.system(size: 12, weight: .bold))
                            Text(project.name)
                                .font(.system(size: 10))
                                .lineLimit(1)
                        }
                        Image(systemName: "chevron.right")
                            .font(.system(size: 9, weight: .bold))
                    }
                    .foregroundStyle(.white)
                    .padding(.horizontal, 16)
                    .frame(minHeight: 54)
                    .background(
                        RoundedRectangle(cornerRadius: 11)
                            .fill(Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.14))
                    )
                    .overlay(
                        RoundedRectangle(cornerRadius: 11)
                            .stroke(Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.30), lineWidth: 1)
                    )
                }
                .buttonStyle(MIR4DStartCardButtonStyle())
            }
        }
    }

    // MARK: - Primary workflows

    private var primaryWorkflows: some View {
        VStack(alignment: .leading, spacing: 10) {
            sectionHeader("СОЗДАТЬ", caption: "Главные инженерные сценарии")

            LazyVGrid(
                columns: Array(repeating: GridItem(.flexible(), spacing: 12), count: isCompact ? 1 : 3),
                spacing: 12
            ) {
                ForEach(Array(templates.enumerated()), id: \.element.id) { index, template in
                    primaryCard(template, gradient: cardGradients[index]) {
                        presentNewProject(preset: template.workbench, defaultName: template.title)
                    }
                }
            }
        }
    }

    private func primaryCard(
        _ template: CADProjectTemplate,
        gradient: [Color],
        action: @escaping () -> Void
    ) -> some View {
        Button(action: action) {
            HStack(spacing: 14) {
                ZStack {
                    RoundedRectangle(cornerRadius: 10)
                        .fill(LinearGradient(colors: gradient, startPoint: .topLeading, endPoint: .bottomTrailing))
                    Image(systemName: template.icon)
                        .font(.system(size: 24, weight: .medium))
                        .foregroundStyle(.white.opacity(0.95))
                }
                .frame(width: 58, height: 58)

                VStack(alignment: .leading, spacing: 4) {
                    Text(template.title)
                        .font(.system(size: 14, weight: .semibold))
                        .foregroundStyle(.white)
                    Text(template.subtitle)
                        .font(.system(size: 10))
                        .foregroundStyle(.secondary)
                }

                Spacer()
                Image(systemName: "plus.circle")
                    .font(.system(size: 17))
                    .foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0))
            }
            .padding(12)
            .frame(maxWidth: .infinity, minHeight: 84)
            .background(RoundedRectangle(cornerRadius: 13).fill(Color.white.opacity(0.035)))
            .overlay(RoundedRectangle(cornerRadius: 13).stroke(Color.white.opacity(0.08), lineWidth: 1))
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
    }

    // MARK: - Recent projects

    private var recentProjectsSection: some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack(alignment: .firstTextBaseline) {
                sectionHeader("ПРОЕКТЫ", caption: "Недавние")
                Spacer()
                Button("Открыть все") { presentOpenProject() }
                    .buttonStyle(MIR4DQuietButtonStyle())
            }

            HStack(spacing: 8) {
                Image(systemName: "magnifyingglass")
                    .font(.system(size: 11))
                    .foregroundStyle(.secondary)
                TextField("Найти проект…", text: $recentFilter)
                    .textFieldStyle(.plain)
                    .font(.system(size: 11))
                    .foregroundStyle(.white)
            }
            .padding(.horizontal, 11)
            .frame(height: 34)
            .background(RoundedRectangle(cornerRadius: 9).fill(Color.white.opacity(0.035)))
            .overlay(RoundedRectangle(cornerRadius: 9).stroke(Color.white.opacity(0.07), lineWidth: 1))

            let displayed = (recentFilter.isEmpty ? recentProjects : recentProjects.filter {
                $0.name.localizedCaseInsensitiveContains(recentFilter) ||
                $0.path.localizedCaseInsensitiveContains(recentFilter)
            }).prefix(isCompact ? 3 : 4)

            if displayed.isEmpty {
                emptyProjectsState
            } else {
                LazyVGrid(
                    columns: [GridItem(.flexible(), spacing: 10), GridItem(.flexible(), spacing: 10)],
                    spacing: 10
                ) {
                    ForEach(Array(displayed)) { project in
                        recentTile(project)
                    }
                }
            }
        }
    }

    private var emptyProjectsState: some View {
        HStack(spacing: 10) {
            Image(systemName: recentFilter.isEmpty ? "folder.badge.plus" : "magnifyingglass")
                .font(.system(size: 17))
                .foregroundStyle(.secondary)
            VStack(alignment: .leading, spacing: 2) {
                Text(recentFilter.isEmpty ? "Пока нет недавних проектов" : "Ничего не найдено")
                    .font(.system(size: 11, weight: .medium))
                    .foregroundStyle(.white.opacity(0.85))
                Text(recentFilter.isEmpty ? "Создайте первый проект — он появится здесь." : "Измените запрос поиска.")
                    .font(.system(size: 9.5))
                    .foregroundStyle(.secondary)
            }
            Spacer()
            if recentFilter.isEmpty {
                Button("Создать") { presentNewProject() }
                    .buttonStyle(MIR4DQuietButtonStyle())
            }
        }
        .padding(12)
        .background(RoundedRectangle(cornerRadius: 11).fill(Color.white.opacity(0.025)))
        .overlay(RoundedRectangle(cornerRadius: 11).stroke(Color.white.opacity(0.06), lineWidth: 1))
    }

    private func recentTile(_ project: MIR4DRecentProject) -> some View {
        let available = MIR4DProjectSession.shared.availability(of: project) == .available
        let workbench = (try? MIR4DProjectStore.shared.load(from: project.url))
            .flatMap { CADWorkbench(rawValue: $0.workbench) }

        return Button {
            guard available else { return }
            commands.open(appState: appState, url: project.url)
        } label: {
            HStack(spacing: 10) {
                ZStack {
                    RoundedRectangle(cornerRadius: 8)
                        .fill(Color.white.opacity(0.06))
                    projectThumbnail(project, available: available)
                        .frame(width: 54, height: 40)
                }
                .frame(width: 54, height: 40)

                VStack(alignment: .leading, spacing: 3) {
                    Text(project.name)
                        .font(.system(size: 11, weight: .medium))
                        .foregroundStyle(.white)
                        .lineLimit(1)
                    if let workbench {
                        Text(workbench.titleRU)
                            .font(.system(size: 8, weight: .semibold))
                            .foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0))
                    }
                    Text(project.lastOpened.formatted(date: .abbreviated, time: .shortened))
                        .font(.system(size: 8))
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                }

                Spacer(minLength: 4)
                Image(systemName: available ? "chevron.right" : "exclamationmark.triangle")
                    .font(.system(size: 9))
                    .foregroundStyle(available ? Color.secondary : Color.orange)
            }
            .padding(10)
            .frame(maxWidth: .infinity, minHeight: 64, alignment: .leading)
            .background(RoundedRectangle(cornerRadius: 10).fill(Color.white.opacity(0.025)))
            .overlay(RoundedRectangle(cornerRadius: 10).stroke(Color.white.opacity(0.06), lineWidth: 1))
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
        .disabled(!available)
        .contextMenu {
            Button("Открыть") { commands.open(appState: appState, url: project.url) }
            Divider()
            Button("Показать в Finder") {
                NSWorkspace.shared.selectFile(project.url.path, inFileViewerRootedAtPath: "")
            }
            Button("Копировать путь") {
                NSPasteboard.general.clearContents()
                NSPasteboard.general.setString(project.url.path, forType: .string)
            }
            Divider()
            Button("Удалить из недавних") {
                MIR4DProjectSession.shared.removeFromRecents(id: project.id)
                refreshRecents()
            }
        }
    }

    // MARK: - Secondary tools

    private var secondaryTools: some View {
        VStack(alignment: .leading, spacing: 10) {
            Button {
                withAnimation(.easeInOut(duration: 0.2)) { showAdvanced.toggle() }
            } label: {
                HStack(spacing: 8) {
                    Image(systemName: showAdvanced ? "chevron.down" : "chevron.right")
                        .font(.system(size: 9, weight: .bold))
                    Text("Дополнительные возможности")
                        .font(.system(size: 11, weight: .semibold))
                    Text("4D · расчёты · программирование · знания")
                        .font(.system(size: 9))
                        .foregroundStyle(.secondary)
                    Spacer()
                }
                .foregroundStyle(.white.opacity(0.85))
                .padding(.vertical, 5)
            }
            .buttonStyle(.plain)

            if showAdvanced {
                LazyVGrid(
                    columns: Array(repeating: GridItem(.flexible(), spacing: 10), count: isCompact ? 2 : 4),
                    spacing: 10
                ) {
                    ForEach(Array(laboratories.enumerated()), id: \.element.id) { index, lab in
                        Button {
                            enterWorkspace(workbench: lab.workbench, message: "\(lab.title) активирована")
                        } label: {
                            HStack(spacing: 9) {
                                Image(systemName: lab.icon)
                                    .font(.system(size: 15))
                                    .foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0))
                                    .frame(width: 24)
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(lab.title)
                                        .font(.system(size: 10, weight: .medium))
                                        .foregroundStyle(.white)
                                    Text(lab.subtitle)
                                        .font(.system(size: 8))
                                        .foregroundStyle(.secondary)
                                        .lineLimit(1)
                                }
                                Spacer(minLength: 0)
                            }
                            .padding(10)
                            .frame(maxWidth: .infinity, minHeight: 50)
                            .background(RoundedRectangle(cornerRadius: 9).fill(Color.white.opacity(0.025)))
                            .overlay(RoundedRectangle(cornerRadius: 9).stroke(Color.white.opacity(0.06), lineWidth: 1))
                        }
                        .buttonStyle(MIR4DStartCardButtonStyle())
                        .help("\(lab.title): \(lab.subtitle)")
                        .id(index)
                    }
                }
            }
        }
    }

    // MARK: - What's new

    private var whatsNewSheet: some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack(spacing: 12) {
                Image(systemName: "sparkles")
                    .font(.system(size: 22))
                    .foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0))
                VStack(alignment: .leading, spacing: 3) {
                    Text("Что нового в МИР 4D")
                        .font(.system(size: 18, weight: .bold, design: .rounded))
                    Text("Версия \(appVersion)")
                        .font(.system(size: 11, design: .monospaced))
                        .foregroundStyle(.secondary)
                }
                Spacer()
            }

            VStack(alignment: .leading, spacing: 10) {
                whatsNewRow(icon: "arrow.forward.circle", text: "Стартовый экран теперь строится вокруг трёх простых действий: продолжить, создать, открыть.")
                whatsNewRow(icon: "square.stack.3d.up", text: "Основные CAD-сценарии вынесены в единый блок создания проекта.")
                whatsNewRow(icon: "folder", text: "Недавние проекты стали самостоятельной точкой входа с поиском и быстрым открытием.")
                whatsNewRow(icon: "slider.horizontal.3", text: "4D, расчёты и другие лаборатории скрыты в дополнительных возможностях и не перегружают стартовый экран.")
            }

            Spacer()
            HStack {
                Spacer()
                Button("Понятно") { showWhatsNew = false }
                    .buttonStyle(MIR4DTopBarButtonStyle(prominent: true))
                    .keyboardShortcut(.defaultAction)
            }
        }
        .padding(26)
        .frame(width: 560, height: 360)
        .background(Color(NSColor.windowBackgroundColor).opacity(0.97))
    }

    private func whatsNewRow(icon: String, text: String) -> some View {
        HStack(spacing: 12) {
            Image(systemName: icon)
                .font(.system(size: 14))
                .foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0))
                .frame(width: 20)
            Text(text)
                .font(.system(size: 12))
                .foregroundStyle(.white.opacity(0.85))
                .fixedSize(horizontal: false, vertical: true)
        }
    }

    // MARK: - Helpers

    private func sectionHeader(_ title: String, caption: String) -> some View {
        HStack(alignment: .firstTextBaseline, spacing: 8) {
            Text(title)
                .font(.system(size: 10, weight: .semibold, design: .monospaced))
                .foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.88))
            Text(caption)
                .font(.system(size: 9))
                .foregroundStyle(.secondary)
        }
    }

    private func syncViewport(_ size: CGSize) {
        if size != viewport { viewport = size }
    }

    private func presentOpenProject() {
        showNewProject = false
        DispatchQueue.main.async { showOpenProject = true }
    }

    private func presentNewProject() {
        showOpenProject = false
        newProjectPreset = nil
        newProjectDefaultName = nil
        DispatchQueue.main.async { showNewProject = true }
    }

    private func presentNewProject(preset: CADWorkbench, defaultName: String) {
        showOpenProject = false
        newProjectPreset = preset
        newProjectDefaultName = defaultName
        DispatchQueue.main.async { showNewProject = true }
    }

    private func enterWorkspace(workbench: CADWorkbench, message: String) {
        if MIR4DProjectSession.shared.projectURL == nil {
            let parent = defaultProjectsDirectory
            let name = uniqueProjectName(base: "Проект MIR 4D", in: parent)
            MIR4DProjectSession.shared.createProject(
                appState: appState,
                name: name,
                parentURL: parent,
                workbench: workbench
            )
        } else {
            appState.selectWorkbench(workbench)
        }

        appState.showNotification(message, type: .success)
        NotificationCenter.default.post(
            name: .mir4DStartWorkspace,
            object: nil,
            userInfo: ["workbench": workbench.rawValue]
        )
    }

    private var defaultProjectsDirectory: URL {
        let documents =
            FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first
            ?? URL(fileURLWithPath: NSTemporaryDirectory(), isDirectory: true)
        let directory = documents.appendingPathComponent("MIR4D", isDirectory: true)
        try? FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        return directory
    }

    private func uniqueProjectName(base: String, in parent: URL) -> String {
        let packageExtension = MIR4DProjectStore.packageExtension
        var candidate = base
        var index = 1
        while FileManager.default.fileExists(
            atPath: parent.appendingPathComponent("\(candidate).\(packageExtension)", isDirectory: true).path
        ) {
            index += 1
            candidate = "\(base) \(index)"
        }
        return candidate
    }

    private func refreshRecents() {
        recentRefreshToken = UUID()
    }

    @ViewBuilder
    private func projectThumbnail(_ project: MIR4DRecentProject, available: Bool) -> some View {
        let url = project.url.appendingPathComponent("Thumbnails/preview.png")
        if available, let nsImage = NSImage(contentsOf: url), nsImage.isValid {
            Image(nsImage: nsImage)
                .resizable()
                .interpolation(.medium)
                .scaledToFill()
        } else {
            Image(systemName: available ? "cube.transparent" : "exclamationmark.triangle")
                .font(.system(size: 15))
                .foregroundStyle(available ? .white : .orange)
        }
    }
}

private struct CADProjectTemplate: Identifiable {
    let id = UUID()
    let title: String
    let subtitle: String
    let icon: String
    let workbench: CADWorkbench
}

private struct CADLaboratory: Identifiable {
    let id = UUID()
    let title: String
    let subtitle: String
    let icon: String
    let workbench: CADWorkbench
}

struct MIR4DTopBarButtonStyle: ButtonStyle {
    let prominent: Bool

    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .font(.system(size: 11, weight: .semibold))
            .padding(.horizontal, 14)
            .frame(height: 32)
            .background {
                if prominent {
                    RoundedRectangle(cornerRadius: 8).fill(
                        LinearGradient(
                            colors: [
                                Color(red: 0.30, green: 0.55, blue: 1.0),
                                Color(red: 0.26, green: 0.85, blue: 1.0)
                            ],
                            startPoint: .leading,
                            endPoint: .trailing
                        )
                    )
                } else {
                    RoundedRectangle(cornerRadius: 8).fill(Color.white.opacity(0.055))
                }
            }
            .overlay(
                RoundedRectangle(cornerRadius: 8)
                    .stroke(Color.white.opacity(prominent ? 0.22 : 0.08), lineWidth: 1)
            )
            .opacity(configuration.isPressed ? 0.72 : 1)
    }
}

struct MIR4DQuietButtonStyle: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .font(.system(size: 10, weight: .medium))
            .foregroundStyle(.secondary)
            .opacity(configuration.isPressed ? 0.55 : 1)
    }
}

struct MIR4DStartCardButtonStyle: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .scaleEffect(configuration.isPressed ? 0.985 : 1.0)
            .opacity(configuration.isPressed ? 0.84 : 1.0)
            .animation(.easeOut(duration: 0.12), value: configuration.isPressed)
    }
}

struct MIR4DLaunchGeometricMark: View {
    @State private var glow = false

    var body: some View {
        ZStack {
            RoundedRectangle(cornerRadius: 5)
                .stroke(Color(red: 0.26, green: 0.85, blue: 1.0).opacity(glow ? 0.9 : 0.55), lineWidth: 2)
                .frame(width: 43, height: 43)
                .rotationEffect(.degrees(45))
            RoundedRectangle(cornerRadius: 4)
                .stroke(Color.white.opacity(0.75), lineWidth: 1.2)
                .frame(width: 30, height: 30)
                .rotationEffect(.degrees(45))
            Rectangle()
                .fill(Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.8))
                .frame(width: 1.5, height: 25)
                .rotationEffect(.degrees(45))
                .offset(x: 11, y: 11)
        }
        .frame(width: 76, height: 76)
        .shadow(color: Color(red: 0.26, green: 0.85, blue: 1.0).opacity(glow ? 0.45 : 0.16), radius: 15)
        .onAppear {
            withAnimation(.easeInOut(duration: 1.8).repeatForever(autoreverses: true)) { glow = true }
        }
    }
}
