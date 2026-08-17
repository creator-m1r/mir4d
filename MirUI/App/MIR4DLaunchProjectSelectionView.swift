import SwiftUI
import AppKit

/// Industrial start centre (NX / SolidWorks / Fusion style): full-screen,
/// horizontal, picture-card layout. Everything fits on one screen without
/// vertical scrolling. Diagnostics move into the header as a compact status
/// indicator; the door is gone — the hub is the page.
struct MIR4DLaunchProjectSelectionView: View {
    @EnvironmentObject private var appState: CADAppState
    @ObservedObject var diagnostic: MIR4DBootCoordinator
    @Binding var isLeaving: Bool

    @State private var showOpenProject = false
    @State private var showNewProject = false
    @State private var newProjectPreset: CADWorkbench?
    @State private var newProjectDefaultName: String?
    @State private var selectedRecentID: UUID?
    @State private var hoveredRecentID: UUID?
    @State private var recentFilter: String = ""
    @State private var viewport: CGSize = .zero
    @FocusState private var recentSearchFocused: Bool
    @State private var showWhatsNew = false
    @State private var appeared = false
    @State private var autoOpenLastProject = MIR4DProjectSession.shared.isAutoOpenLastProjectEnabled
    @State private var recentRefreshToken = UUID()

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

    /// CAD-oriented entry templates shown as picture cards on the start centre.
    private let templates: [CADProjectTemplate] = [
        CADProjectTemplate(title: "Деталь", subtitle: "3D CAD модель", icon: "cube", workbench: .model),
        CADProjectTemplate(title: "Сборка", subtitle: "Структура и связи", icon: "square.stack.3d.up", workbench: .assembly),
        CADProjectTemplate(title: "Чертёж", subtitle: "Конструкторская документация", icon: "doc.text", workbench: .drawing),
        CADProjectTemplate(title: "Листовой металл", subtitle: "Гибка и штамповка", icon: "square.3.layers.xy", workbench: .model),
        CADProjectTemplate(title: "BIM", subtitle: "Здание и инфраструктура", icon: "building.2", workbench: .model),
        CADProjectTemplate(title: "Симуляция", subtitle: "Прочность и физика", icon: "waveform.path.ecg", workbench: .simulation)
    ]

    /// Specialised engineering laboratories available without a CAD document.
    private let laboratories: [CADLaboratory] = [
        CADLaboratory(title: "4D Лаборатория", subtitle: "Симуляции, физика, сценарии во времени", icon: "cube.transparent", workbench: .fourD),
        CADLaboratory(title: "Математическая Вселенная", subtitle: "Инженерные расчёты и модели", icon: "function", workbench: .simulation),
        CADLaboratory(title: "Мир программирования", subtitle: "Визуальное и текстовое программирование", icon: "chevron.left.forwardslash.chevron.right", workbench: .model),
        CADLaboratory(title: "МИР Знаний", subtitle: "Интерактивное обучение", icon: "books.vertical", workbench: .model)
    ]

    /// Picture-card gradients: MIR 4D palette (accent / accentBright / keyframe
    /// / success from MirTheme) over deep industrial blues.
    private let templateGradients: [[Color]] = [
        [Color(red: 0.30, green: 0.55, blue: 1.0), Color(red: 0.05, green: 0.10, blue: 0.24)],
        [Color(red: 0.26, green: 0.85, blue: 1.0), Color(red: 0.04, green: 0.13, blue: 0.21)],
        [Color(red: 0.42, green: 0.48, blue: 0.70), Color(red: 0.06, green: 0.08, blue: 0.15)],
        [Color(red: 1.0, green: 0.70, blue: 0.20), Color(red: 0.22, green: 0.13, blue: 0.04)],
        [Color(red: 0.48, green: 0.34, blue: 0.90), Color(red: 0.09, green: 0.05, blue: 0.21)],
        [Color(red: 0.30, green: 0.90, blue: 0.55), Color(red: 0.05, green: 0.17, blue: 0.10)]
    ]

    /// Picture-card gradients for the four laboratories.
    private let labGradients: [[Color]] = [
        [Color(red: 0.26, green: 0.85, blue: 1.0), Color(red: 0.04, green: 0.14, blue: 0.22)],
        [Color(red: 0.30, green: 0.55, blue: 1.0), Color(red: 0.05, green: 0.09, blue: 0.24)],
        [Color(red: 1.0, green: 0.70, blue: 0.20), Color(red: 0.21, green: 0.13, blue: 0.04)],
        [Color(red: 0.30, green: 0.90, blue: 0.55), Color(red: 0.05, green: 0.16, blue: 0.10)]
    ]

    private var isCompact: Bool {
        viewport.width < 1080 || viewport.height < 720
    }

    /// Header height (brand row + divider).
    private var headerHeight: CGFloat { 62 }

    /// Picture-card height: derived from the available height so that the whole
    /// hub always fits the screen without scrolling.
    private var tileHeight: CGFloat {
        let available = viewport.height - headerHeight - 40
        let perRow = max(88, (available - 130) / 2)
        return min(152, perRow)
    }

    private var sideMargin: CGFloat { max(18, viewport.width * 0.02) }

    private func syncViewport(_ size: CGSize) {
        if size != viewport { viewport = size }
    }

    var body: some View {
        GeometryReader { proxy in
            let _ = syncViewport(proxy.size)
            ZStack {
                Color.black.ignoresSafeArea()
                MIR4DStartupMotionLayer().opacity(0.16)

                hub
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                    .padding(.horizontal, sideMargin)
                    .padding(.top, 10)
                    .padding(.bottom, 18)
                    .onDrop(of: [.fileURL], isTargeted: nil) { providers in
                        guard let provider = providers.first(where: { $0.hasItemConformingToTypeIdentifier("public.file-url") }) else { return false }
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
            .offset(x: isLeaving ? proxy.size.width : 0)
            .opacity(isLeaving ? 0.25 : 1)
            .animation(.timingCurve(0.18, 0.80, 0.22, 1.0, duration: 0.78), value: isLeaving)
        }
        .sheet(isPresented: $showOpenProject) { MIR4DProjectOpenView().environmentObject(appState) }
        .sheet(isPresented: $showNewProject) {
            MIR4DNewProjectView(presetWorkbench: newProjectPreset, defaultName: newProjectDefaultName)
                .environmentObject(appState)
        }
        .sheet(isPresented: $showWhatsNew) { whatsNewSheet }
        .onAppear {
            withAnimation(.timingCurve(0.16, 0.82, 0.22, 1.0, duration: 0.85)) { appeared = true }
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectActivated)) { _ in refreshRecents() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectSaved)) { _ in refreshRecents() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DProjectClosed)) { _ in refreshRecents() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DRequestNewProject)) { _ in presentNewProject() }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DOpenProject)) { _ in presentOpenProject() }
    }

    // MARK: - Full-screen hub

    private var hub: some View {
        VStack(spacing: 0) {
            header
            Divider().overlay(Color.white.opacity(0.08))
            mainArea
        }
        .background(
            RoundedRectangle(cornerRadius: 14).fill(
                LinearGradient(
                    colors: [Color(red: 0.035, green: 0.055, blue: 0.085), Color(red: 0.008, green: 0.012, blue: 0.020)],
                    startPoint: .topLeading,
                    endPoint: .bottomTrailing
                )
            )
        )
        .overlay(RoundedRectangle(cornerRadius: 14).stroke(Color.white.opacity(0.16), lineWidth: 1))
        .overlay(alignment: .leading) {
            Rectangle().fill(LinearGradient(colors: [.clear, Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.75), .clear], startPoint: .top, endPoint: .bottom)).frame(width: 2).blur(radius: 4)
        }
        .shadow(color: Color(red: 0.30, green: 0.55, blue: 1.0).opacity(0.13), radius: 34, y: 6)
        .offset(x: appeared ? 0 : 0)
        .opacity(appeared ? 1 : 0)
        .onKeyPress(phases: .down) { press in
            guard !showNewProject, !showOpenProject else { return .ignored }

            if press.key == .upArrow || press.key == .downArrow {
                moveRecentSelection(down: press.key == .downArrow)
                return .handled
            }

            if press.modifiers.contains(.command), press.characters == "f" || press.characters == "F" {
                recentSearchFocused = true
                return .handled
            }

            switch press.characters {
            case "1"..."6":
                if let value = Int(press.characters), value <= templates.count {
                    let template = templates[value - 1]
                    presentNewProject(preset: template.workbench, defaultName: template.title)
                }
                return .handled

            case "\r", "\n":
                if let id = selectedRecentID,
                   let project = recentProjects.first(where: { $0.id == id }) {
                    commands.open(appState: appState, url: project.url)
                } else if canContinue, let project = continueProject {
                    commands.open(appState: appState, url: project.url)
                } else if let first = recentProjects.first {
                    commands.open(appState: appState, url: first.url)
                }
                return .handled

            default:
                return .ignored
            }
        }
    }

    // MARK: - Header

    private var header: some View {
        HStack(spacing: 16) {
            HStack(spacing: 12) {
                MIR4DLaunchGeometricMark()
                    .scaleEffect(0.55)
                    .frame(width: 44, height: 44)
                VStack(alignment: .leading, spacing: 2) {
                    Text("МИР 4D").font(.system(size: 17, weight: .bold, design: .rounded)).foregroundStyle(.white)
                    Text("Инженерная среда моделирования").font(.system(size: 10)).foregroundStyle(.secondary)
                }
            }

            Spacer()

            HStack(spacing: 9) {
                Circle().fill(diagnostic.state == .failed ? .red : (diagnostic.state == .warning ? .yellow : .green)).frame(width: 7, height: 7)
                Text(diagnostic.state == .ready ? "Готово" : (diagnostic.state == .failed ? "Ошибка запуска" : "Проверка…"))
                    .font(.system(size: 10, weight: .medium))
                    .foregroundStyle(diagnostic.state == .ready ? .white.opacity(0.85) : .secondary)
                ProgressView(value: diagnostic.progress, total: 1)
                    .tint(Color(red: 0.26, green: 0.85, blue: 1.0))
                    .frame(width: 84)
            }
            .help("Самодиагностика: \(Int(diagnostic.progress * 100))% · \(diagnostic.currentTitle)")

            Button {
                showWhatsNew = true
            } label: {
                Image(systemName: "sparkles").font(.system(size: 12))
            }
            .buttonStyle(MIR4DTopBarButtonStyle(prominent: false))
            .help("Что нового в МИР 4D")

            Button("Открыть") { presentOpenProject() }
                .buttonStyle(MIR4DTopBarButtonStyle(prominent: false))
            Button("Новый проект") { presentNewProject() }
                .buttonStyle(MIR4DTopBarButtonStyle(prominent: true))
        }
        .padding(.horizontal, 24)
        .frame(height: headerHeight)
        .foregroundStyle(.white)
    }

    // MARK: - Main horizontal area: Start | Templates | Labs + Recents

    private var mainArea: some View {
        HStack(alignment: .top, spacing: 18) {
            startColumn
                .frame(width: max(200, viewport.width * 0.19))
            Divider().overlay(Color.white.opacity(0.08))
            templateColumn
                .frame(maxWidth: .infinity)
            Divider().overlay(Color.white.opacity(0.08))
            labsAndRecentsColumn
                .frame(width: max(300, viewport.width * 0.34))
        }
        .padding(16)
    }

    // MARK: - Start column

    private var startColumn: some View {
        VStack(alignment: .leading, spacing: 10) {
            sectionTitle("НАЧАТЬ")
            quickTile(
                title: "Продолжить",
                subtitle: continueProject?.name ?? "Последний проект",
                icon: "arrow.forward.circle.fill",
                gradient: [Color(red: 0.30, green: 0.55, blue: 1.0), Color(red: 0.05, green: 0.10, blue: 0.24)],
                height: tileHeight
            ) {
                guard canContinue, let project = continueProject else { return }
                commands.open(appState: appState, url: project.url)
            }
            .disabled(!canContinue)

            quickTile(
                title: "Создать проект",
                subtitle: "Новый документ .mir4d",
                icon: "plus",
                gradient: [Color(red: 0.30, green: 0.90, blue: 0.55), Color(red: 0.05, green: 0.17, blue: 0.10)],
                height: tileHeight
            ) { presentNewProject() }

            quickTile(
                title: "Открыть",
                subtitle: "Проект .mir4d на диске",
                icon: "folder",
                gradient: [Color(red: 1.0, green: 0.70, blue: 0.20), Color(red: 0.22, green: 0.13, blue: 0.04)],
                height: tileHeight
            ) { presentOpenProject() }

            Spacer(minLength: 0)
        }
        .frame(maxHeight: .infinity, alignment: .topLeading)
    }

    // MARK: - Templates column

    private var templateColumn: some View {
        VStack(alignment: .leading, spacing: 10) {
            sectionTitle("ШАБЛОНЫ ПРОЕКТОВ")
            LazyVGrid(columns: [GridItem(.flexible(), spacing: 10), GridItem(.flexible(), spacing: 10), GridItem(.flexible(), spacing: 10)], spacing: 10) {
                ForEach(Array(templates.enumerated()), id: \.element.id) { index, template in
                    pictureTile(
                        icon: template.icon,
                        title: template.title,
                        subtitle: template.subtitle,
                        gradient: templateGradients[index % templateGradients.count],
                        height: tileHeight
                    ) {
                        presentNewProject(preset: template.workbench, defaultName: template.title)
                    }
                }
            }
            Spacer(minLength: 0)
        }
        .frame(maxHeight: .infinity, alignment: .topLeading)
    }

    // MARK: - Laboratories + recents column

    private var labsAndRecentsColumn: some View {
        VStack(alignment: .leading, spacing: 12) {
            VStack(alignment: .leading, spacing: 10) {
                sectionTitle("ЛАБОРАТОРИИ И РЕЖИМЫ")
                LazyVGrid(columns: [GridItem(.flexible(), spacing: 10), GridItem(.flexible(), spacing: 10)], spacing: 10) {
                    ForEach(Array(laboratories.enumerated()), id: \.element.id) { index, lab in
                        pictureTile(
                            icon: lab.icon,
                            title: lab.title,
                            subtitle: lab.subtitle,
                            gradient: labGradients[index % labGradients.count],
                            height: tileHeight
                        ) {
                            enterWorkspace(workbench: lab.workbench, message: "\(lab.title) активирована")
                        }
                    }
                }
            }

            VStack(alignment: .leading, spacing: 10) {
                HStack(alignment: .firstTextBaseline) {
                    sectionTitle("НЕДАВНИЕ ПРОЕКТЫ")
                    Spacer()
                    Button("Все") { presentOpenProject() }
                        .buttonStyle(MIR4DQuietButtonStyle())
                }

                HStack(spacing: 6) {
                    Image(systemName: "magnifyingglass").font(.system(size: 10)).foregroundStyle(.secondary)
                    TextField("Поиск", text: $recentFilter)
                        .textFieldStyle(.plain)
                        .font(.system(size: 10))
                        .foregroundStyle(.white)
                        .focused($recentSearchFocused)
                }
                .padding(.horizontal, 9).padding(.vertical, 6)
                .background(RoundedRectangle(cornerRadius: 8).fill(Color.white.opacity(0.04)))
                .overlay(RoundedRectangle(cornerRadius: 8).stroke(Color.white.opacity(0.07), lineWidth: 1))

                let displayed = (recentFilter.isEmpty ? recentProjects : recentProjects.filter {
                    $0.name.localizedCaseInsensitiveContains(recentFilter) ||
                    $0.path.localizedCaseInsensitiveContains(recentFilter)
                }).prefix(2)

                if recentProjects.isEmpty {
                    HStack(spacing: 8) {
                        Image(systemName: "folder.badge.plus").font(.system(size: 15)).foregroundStyle(.secondary)
                        Text("Проекты появятся здесь").font(.system(size: 9.5)).foregroundStyle(.secondary)
                        Spacer()
                    }
                    .padding(10)
                    .background(RoundedRectangle(cornerRadius: 9).fill(Color.white.opacity(0.03)))
                    .overlay(RoundedRectangle(cornerRadius: 9).stroke(Color.white.opacity(0.06), lineWidth: 1))
                } else if displayed.isEmpty {
                    HStack(spacing: 8) {
                        Image(systemName: "magnifyingglass").font(.system(size: 15)).foregroundStyle(.secondary)
                        Text("Ничего не найдено").font(.system(size: 9.5)).foregroundStyle(.secondary)
                        Spacer()
                    }
                    .padding(10)
                    .background(RoundedRectangle(cornerRadius: 9).fill(Color.white.opacity(0.03)))
                    .overlay(RoundedRectangle(cornerRadius: 9).stroke(Color.white.opacity(0.06), lineWidth: 1))
                } else {
                    VStack(spacing: 8) {
                        ForEach(Array(displayed)) { project in
                            recentTile(project)
                        }
                    }
                }

                Toggle(
                    "Автооткрытие последнего проекта",
                    isOn: Binding(
                        get: { autoOpenLastProject },
                        set: { value in
                            autoOpenLastProject = value
                            MIR4DProjectSession.shared.isAutoOpenLastProjectEnabled = value
                        }
                    )
                )
                .toggleStyle(.switch)
                .font(.system(size: 9))
                .foregroundStyle(.secondary)
            }

            Spacer(minLength: 0)
        }
        .frame(maxHeight: .infinity, alignment: .topLeading)
    }

    // MARK: - Picture tiles

    /// Industrial picture card: a gradient "preview image" with a large glyph,
    /// the title and a short engineering subtitle.
    private func pictureTile(
        icon: String,
        title: String,
        subtitle: String,
        gradient: [Color],
        height: CGFloat,
        action: @escaping () -> Void
    ) -> some View {
        Button(action: action) {
            VStack(alignment: .leading, spacing: 6) {
                ZStack {
                    RoundedRectangle(cornerRadius: 9)
                        .fill(LinearGradient(colors: gradient, startPoint: .topLeading, endPoint: .bottomTrailing))
                    Image(systemName: icon)
                        .font(.system(size: 30, weight: .medium))
                        .foregroundStyle(.white.opacity(0.92))
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .overlay(alignment: .topLeading) {
                    RoundedRectangle(cornerRadius: 9).stroke(Color.white.opacity(0.14), lineWidth: 1)
                }
                Text(title).font(.system(size: 11, weight: .semibold)).foregroundStyle(.white).lineLimit(1)
                Text(subtitle).font(.system(size: 8.5)).foregroundStyle(.white.opacity(0.60)).lineLimit(1)
            }
            .padding(9)
            .frame(maxWidth: .infinity, minHeight: height, alignment: .leading)
            .background(RoundedRectangle(cornerRadius: 12).fill(Color(red: 0.075, green: 0.090, blue: 0.115).opacity(0.40)))
            .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.white.opacity(0.08), lineWidth: 1))
        }
        .buttonStyle(MIR4DStartCardButtonStyle())
        .help(title)
    }

    private func quickTile(
        title: String,
        subtitle: String,
        icon: String,
        gradient: [Color],
        height: CGFloat,
        action: @escaping () -> Void
    ) -> some View {
        pictureTile(icon: icon, title: title, subtitle: subtitle, gradient: gradient, height: height, action: action)
    }

    private func recentTile(_ project: MIR4DRecentProject) -> some View {
        let available = MIR4DProjectSession.shared.availability(of: project) == .available
        let isSelected = (selectedRecentID ?? recentProjects.first?.id) == project.id
        let workbench = (try? MIR4DProjectStore.shared.load(from: project.url))
            .flatMap { CADWorkbench(rawValue: $0.workbench) }
        return Button {
            guard available else { return }
            commands.open(appState: appState, url: project.url)
        } label: {
            HStack(spacing: 10) {
                ZStack {
                    RoundedRectangle(cornerRadius: 7)
                        .fill(
                            LinearGradient(
                                colors: [Color(red: 0.20, green: 0.34, blue: 0.56), Color(red: 0.06, green: 0.11, blue: 0.22)],
                                startPoint: .topLeading,
                                endPoint: .bottomTrailing
                            )
                        )
                    projectThumbnail(project, available: available)
                        .frame(width: 64, height: 42)
                }
                .frame(width: 64, height: 42)
                .overlay(RoundedRectangle(cornerRadius: 7).stroke(Color.white.opacity(0.10), lineWidth: 1))

                VStack(alignment: .leading, spacing: 3) {
                    Text(project.name).font(.system(size: 11, weight: .medium)).foregroundStyle(.white).lineLimit(1)
                    if let workbench {
                        Text(workbench.titleRU.uppercased())
                            .font(.system(size: 6.5, weight: .semibold, design: .monospaced))
                            .padding(.horizontal, 4).padding(.vertical, 1.5)
                            .background(Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.16))
                            .foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0))
                            .cornerRadius(3)
                    }
                    Text(project.lastOpened.formatted(date: .abbreviated, time: .shortened))
                        .font(.system(size: 8.5)).foregroundStyle(.secondary).lineLimit(1)
                }
                Spacer()
                Image(systemName: available ? "arrow.up.right" : "exclamationmark.triangle")
                    .font(.system(size: 9)).foregroundStyle(available ? Color.secondary : Color.orange)
            }
            .padding(.horizontal, 10).padding(.vertical, 8)
            .frame(maxWidth: .infinity, minHeight: max(58, tileHeight * 0.44), alignment: .leading)
            .background((isSelected || hoveredRecentID == project.id) ? Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.14) : Color.white.opacity(0.030))
            .contentShape(Rectangle())
            .overlay(RoundedRectangle(cornerRadius: 9).stroke(Color.white.opacity(0.06), lineWidth: 1))
        }
        .buttonStyle(.plain)
        .onHover { isOver in
            if isOver { hoveredRecentID = project.id }
            else if hoveredRecentID == project.id { hoveredRecentID = nil }
        }
        .disabled(!available)
        .contextMenu {
            Button("Открыть") { commands.open(appState: appState, url: project.url) }
            Divider()
            Button("Показать в Finder") { NSWorkspace.shared.selectFile(project.url.path, inFileViewerRootedAtPath: "") }
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

    private var whatsNewSheet: some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack(spacing: 12) {
                Image(systemName: "sparkles")
                    .font(.system(size: 22))
                    .foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0))
                VStack(alignment: .leading, spacing: 3) {
                    Text("Что нового в МИР 4D")
                        .font(.system(size: 18, weight: .bold, design: .rounded))
                        .foregroundStyle(.white)
                    Text("Версия \(appVersion)")
                        .font(.system(size: 11, design: .monospaced))
                        .foregroundStyle(.secondary)
                }
                Spacer()
            }

            VStack(alignment: .leading, spacing: 10) {
                whatsNewRow(icon: "square.stack.3d.up.fill", text: "Промышленный полноэкранный старт-центр: горизонтальные карточки-картинки без прокрутки.")
                whatsNewRow(icon: "magnifyingglass", text: "Поиск по недавним проектам и открытие перетаскиванием файла .mir4d.")
                whatsNewRow(icon: "checklist", text: "Полная самодиагностика запуска со статусом готовности в шапке.")
                whatsNewRow(icon: "keyboard", text: "Клавиатурная навигация: 1–6 — шаблоны, ↑/↓ — выбор, ⌘F — поиск, Enter — открыть.")
                whatsNewRow(icon: "doc.fill", text: "Реальное сохранение проектов в формате .mir4d с манифестом и превью.")
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
        .frame(width: 520, height: 380)
        .background(Color(NSColor.windowBackgroundColor).opacity(0.96))
    }

    private func whatsNewRow(icon: String, text: String) -> some View {
        HStack(spacing: 12) {
            Image(systemName: icon).font(.system(size: 14)).foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0)).frame(width: 20)
            Text(text).font(.system(size: 12)).foregroundStyle(.white.opacity(0.85)).fixedSize(horizontal: false, vertical: true)
        }
    }

    private func sectionTitle(_ text: String) -> some View {
        Text(text)
            .font(.system(size: 10, weight: .semibold, design: .monospaced))
            .foregroundStyle(Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.85))
            .frame(maxWidth: .infinity, alignment: .leading)
    }

    // MARK: - Actions

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
        // Every route from the start centre must be backed by a real .mir4d
        // package on disk, so the user never loses work in an unsaved in-memory
        // document. If a project is already open we just switch its workbench.
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

    private func moveRecentSelection(down: Bool) {
        let list = recentProjects
        guard !list.isEmpty else { return }
        if let current = selectedRecentID,
           let index = list.firstIndex(where: { $0.id == current }) {
            let next = down
                ? min(list.count - 1, index + 1)
                : max(0, index - 1)
            selectedRecentID = list[next].id
        } else {
            selectedRecentID = list.first?.id
        }
    }

    /// Default location for projects started directly from the start centre
    /// (laboratories / modes) without an explicit "New project" folder pick.
    private var defaultProjectsDirectory: URL {
        let documents =
            FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first
            ?? URL(fileURLWithPath: NSTemporaryDirectory(), isDirectory: true)
        let directory = documents.appendingPathComponent("MIR4D", isDirectory: true)
        try? FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        return directory
    }

    /// Avoids colliding with an existing package when auto-naming a project.
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

    /// Loads the package preview written by MIR4DProjectStore, falling back to a
    /// generic icon when the project is unavailable or has no preview yet.
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

// MARK: - CAD project template

private struct CADProjectTemplate: Identifiable {
    let id = UUID()
    let title: String
    let subtitle: String
    let icon: String
    let workbench: CADWorkbench
}

// MARK: - CAD laboratory

private struct CADLaboratory: Identifiable {
    let id = UUID()
    let title: String
    let subtitle: String
    let icon: String
    let workbench: CADWorkbench
}

// MARK: - Shared button styles

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
                            colors: [Color(red: 0.30, green: 0.55, blue: 1.0), Color(red: 0.26, green: 0.85, blue: 1.0)],
                            startPoint: .leading,
                            endPoint: .trailing
                        )
                    )
                } else {
                    RoundedRectangle(cornerRadius: 8).fill(Color.white.opacity(0.055))
                }
            }
            .overlay(RoundedRectangle(cornerRadius: 8).stroke(Color.white.opacity(prominent ? 0.22 : 0.08), lineWidth: 1))
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
            RoundedRectangle(cornerRadius: 5).stroke(Color(red: 0.26, green: 0.85, blue: 1.0).opacity(glow ? 0.9 : 0.55), lineWidth: 2).frame(width: 43, height: 43).rotationEffect(.degrees(45))
            RoundedRectangle(cornerRadius: 4).stroke(Color.white.opacity(0.75), lineWidth: 1.2).frame(width: 30, height: 30).rotationEffect(.degrees(45))
            Rectangle().fill(Color(red: 0.26, green: 0.85, blue: 1.0).opacity(0.8)).frame(width: 1.5, height: 25).rotationEffect(.degrees(45)).offset(x: 11, y: 11)
        }
        .frame(width: 76, height: 76)
        .shadow(color: Color(red: 0.26, green: 0.85, blue: 1.0).opacity(glow ? 0.45 : 0.16), radius: 15)
        .onAppear {
            withAnimation(.easeInOut(duration: 1.8).repeatForever(autoreverses: true)) { glow = true }
        }
    }
}