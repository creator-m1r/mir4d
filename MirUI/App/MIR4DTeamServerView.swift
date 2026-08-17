import SwiftUI
import AppKit
import MirServer

/// Модель-мост между SwiftUI и подсистемой MirServer.
///
/// Изолирована на главном потоке (`@MainActor`): подписывается на Event Bus
/// `MirServer` и зеркалирует состояние в `@Published` поля для представления,
/// а пользовательские действия перенаправляет в `MirServerManager` / `MirTeamChat`.
@MainActor
final class MIR4DTeamServerModel: ObservableObject {
    @Published var status: MirServerConnectionStatus = .disconnected
    @Published var team: [MirTeamMember] = []
    @Published var messages: [MirTeamMessage] = []
    @Published var lastError: String?

    // Редактируемая конфигурация.
    @Published var baseURLString: String
    @Published var webAppURLString: String
    @Published var apiToken: String
    @Published var teamID: String
    @Published var userID: String
    @Published var userName: String

    // Ввод чата.
    @Published var draft: String = ""
    @Published var lastExportURL: String?

    private let manager = MirServerManager.shared
    private let chat = MirTeamChat.shared
    private var observers: [NSObjectProtocol] = []

    init() {
        let cfg = manager.configuration
        self.baseURLString = cfg.baseURL.absoluteString
        self.webAppURLString = cfg.webAppURL.absoluteString
        self.apiToken = cfg.apiToken ?? ""
        self.teamID = cfg.teamID
        self.userID = cfg.currentUserID
        self.userName = cfg.currentUserName
        self.status = manager.status
        self.team = manager.team
        self.messages = chat.messages

        let center = NotificationCenter.default
        observers.append(center.addObserver(forName: .mir4DServerStatusChanged, object: nil, queue: .main) { [weak self] note in
            guard let s = note.object as? MirServerConnectionStatus else { return }
            MainActor.assumeIsolated { self?.status = s }
        })
        observers.append(center.addObserver(forName: .mir4DTeamUpdated, object: nil, queue: .main) { [weak self] note in
            guard let t = note.object as? [MirTeamMember] else { return }
            MainActor.assumeIsolated { self?.team = t }
        })
        observers.append(center.addObserver(forName: .mir4DTeamMessageReceived, object: nil, queue: .main) { [weak self] note in
            guard let m = note.object as? MirTeamMessage else { return }
            MainActor.assumeIsolated { self?.messages.append(m) }
        })
        observers.append(center.addObserver(forName: .mir4DServerError, object: nil, queue: .main) { [weak self] note in
            guard let e = note.object as? MirServerError else { return }
            MainActor.assumeIsolated { self?.lastError = String(describing: e) }
        })
        observers.append(center.addObserver(forName: .mir4DProjectExported, object: nil, queue: .main) { [weak self] note in
            guard let r = note.object as? MirProjectExportResult else { return }
            MainActor.assumeIsolated { self?.lastExportURL = r.remoteURL }
        })
    }

    deinit {
        MainActor.assumeIsolated {
            observers.forEach { NotificationCenter.default.removeObserver($0) }
        }
    }

    // MARK: - Действия

    func applyConfiguration() {
        guard let base = URL(string: baseURLString), let web = URL(string: webAppURLString) else {
            lastError = "Некорректный URL"
            return
        }
        let cfg = MirServerConfiguration(
            baseURL: base,
            webAppURL: web,
            apiToken: apiToken.isEmpty ? nil : apiToken,
            teamID: teamID,
            currentUserID: userID,
            currentUserName: userName
        )
        manager.configure(cfg)
        lastError = nil
    }

    func connect() {
        applyConfiguration()
        Task { await manager.connect() }
    }

    /// Автоматически подключиться при открытии окна, если заданы учётные данные.
    func autoConnectIfConfigured() {
        guard status == .disconnected else { return }
        guard let token = manager.configuration.apiToken, !token.isEmpty,
              !manager.configuration.teamID.isEmpty else { return }
        connect()
    }

    func disconnect() {
        manager.disconnect()
    }

    func send() {
        let text = draft.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !text.isEmpty else { return }
        draft = ""
        Task {
            let ok = await chat.send(text, projectID: MIR4DProjectSession.shared.projectUUID?.uuidString)
            if !ok {
                await MainActor.run { self.lastError = "Не удалось отправить: нет соединения" }
            }
        }
    }

    func exportCurrentProject() {
        guard let url = MIR4DProjectSession.shared.projectURL else {
            lastError = "Сначала откройте проект MIR 4D"
            return
        }
        Task {
            do {
                let payload = try MirProjectExporter().archiveProject(at: url)
                let id = MIR4DProjectSession.shared.projectUUID?.uuidString ?? url.lastPathComponent
                let name = MIR4DProjectSession.shared.projectName
                let result = await manager.exportProject(
                    projectID: id,
                    projectName: name,
                    format: "mir4d",
                    payload: payload
                )
                if result == nil {
                    await MainActor.run { self.lastError = "Экспорт не выполнен (нет соединения или ошибка сервера)" }
                }
            } catch {
                await MainActor.run { self.lastError = String(describing: error) }
            }
        }
    }

    func openInBrowser() {
        guard let id = MIR4DProjectSession.shared.projectUUID?.uuidString else {
            lastError = "Проект не открыт"
            return
        }
        manager.openProjectInBrowser(projectID: id)
    }

    var statusLabel: String {
        switch status {
        case .disconnected: return "Не подключено"
        case .connecting: return "Подключение…"
        case .connected: return "Подключено"
        case .failed(let reason): return "Ошибка: \(reason)"
        }
    }
}

/// Панель «Сервер MIR 4D»: подключение, состав команды, чат, экспорт проекта.
struct MIR4DTeamServerView: View {
    @StateObject private var model = MIR4DTeamServerModel()

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            header
            configSection
            Divider()
            teamSection
            Divider()
            chatSection
            Divider()
            exportSection
            if let error = model.lastError {
                Text("Ошибка: \(error)")
                    .font(.caption)
                    .foregroundStyle(.red)
                    .lineLimit(3)
            }
        }
        .padding(16)
        .frame(minWidth: 520, minHeight: 640)
        .task {
            model.autoConnectIfConfigured()
        }
    }

    private var header: some View {
        HStack {
            Circle()
                .fill(model.status == .connected ? Color.green : Color.secondary)
                .frame(width: 10, height: 10)
            Text(model.statusLabel)
                .font(.headline)
            Spacer()
            if model.status == .connected {
                Button("Отключиться") { model.disconnect() }
            } else {
                Button("Подключиться") { model.connect() }
            }
        }
    }

    private var configSection: some View {
        DisclosureGroup("Параметры подключения") {
            Grid(alignment: .leadingFirstTextBaseline, horizontalSpacing: 8, verticalSpacing: 6) {
                configRow("API URL", $model.baseURLString)
                configRow("Сайт", $model.webAppURLString)
                configRow("Токен", $model.apiToken)
                configRow("Команда ID", $model.teamID)
                configRow("Пользователь ID", $model.userID)
                configRow("Имя", $model.userName)
            }
            .textFieldStyle(.roundedBorder)
            .frame(maxWidth: .infinity)
        }
    }

    private func configRow(_ title: String, _ binding: Binding<String>) -> some View {
        GridRow {
            Text(title).gridColumnAlignment(.trailing)
            TextField(title, text: binding)
        }
    }

    private var teamSection: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Команда").font(.subheadline.bold())
            if model.team.isEmpty {
                Text("Нет данных. Подключитесь к серверу.").font(.caption).foregroundStyle(.secondary)
            } else {
                ScrollView(.vertical, showsIndicators: true) {
                    VStack(alignment: .leading, spacing: 4) {
                        ForEach(model.team) { member in
                            HStack {
                                Circle()
                                    .fill(member.online ? Color.green : Color.gray)
                                    .frame(width: 8, height: 8)
                                Text(member.displayName).font(.callout)
                                Spacer()
                                Text(member.role).font(.caption).foregroundStyle(.secondary)
                            }
                        }
                    }
                }
                .frame(maxHeight: 120)
            }
        }
    }

    private var chatSection: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Обмен сообщениями").font(.subheadline.bold())
            ScrollViewReader { proxy in
                ScrollView(.vertical, showsIndicators: true) {
                    VStack(alignment: .leading, spacing: 6) {
                        ForEach(model.messages) { message in
                            VStack(alignment: .leading, spacing: 2) {
                                HStack {
                                    Text(message.authorName).font(.caption.bold())
                                    Spacer()
                                    Text(message.timestamp, style: .time).font(.caption2).foregroundStyle(.secondary)
                                }
                                Text(message.text).textSelection(.enabled)
                            }
                            .padding(6)
                            .background(.quaternary)
                            .cornerRadius(6)
                            .id(message.id)
                        }
                    }
                }
                .frame(maxHeight: 220)
                .onChange(of: model.messages.count) { _, _ in
                    if let last = model.messages.last {
                        withAnimation { proxy.scrollTo(last.id, anchor: .bottom) }
                    }
                }
            }
            HStack {
                TextField("Сообщение команде…", text: $model.draft, onCommit: { model.send() })
                    .textFieldStyle(.roundedBorder)
                Button("Отправить") { model.send() }
                    .disabled(model.status != .connected)
            }
        }
    }

    private var exportSection: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Экспорт проекта").font(.subheadline.bold())
            HStack {
                Button("Экспортировать текущий проект") { model.exportCurrentProject() }
                    .disabled(model.status != .connected)
                Button("Открыть на сайте") { model.openInBrowser() }
            }
            if let url = model.lastExportURL {
                Text("Опубликовано: \(url)")
                    .font(.caption)
                    .foregroundStyle(.green)
                    .textSelection(.enabled)
            }
        }
    }
}

/// Команда меню для открытия окон «Сервер MIR 4D» и «Совместная работа».
struct MIR4DServerCommands: Commands {
    @Environment(\.openWindow) private var openWindow

    var body: some Commands {
        CommandGroup(after: .windowSize) {
            Divider()
            Button("Сервер MIR 4D") {
                openWindow(id: "mir4d-server")
            }
            .keyboardShortcut("u", modifiers: [.command, .shift])
            Button("Совместная работа") {
                openWindow(id: "mir4d-collab")
            }
            .keyboardShortcut("i", modifiers: [.command, .shift])
        }
    }
}
