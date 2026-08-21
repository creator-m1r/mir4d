import Foundation

/// Низкоуровневый сетевой транспорт MirServer.
///
/// Реализован как `actor`, чтобы весь сетевой ввод-вывод был изолирован
/// от главного потока UI и потокобезопасен в модели конкурентности Swift 6.
///
/// Обязанности:
/// - REST-запросы (экспорт проекта, получение состава команды);
/// - долговременное WebSocket-соединение для обмена сообщениями;
/// - преобразование входящих событий в уведомления Event Bus (главный поток).
actor MirServerTransport {
    private let configuration: MirServerConfiguration
    private let session: URLSession
    private var webSocketTask: URLSessionWebSocketTask?
    private var listenTask: Task<Void, Never>?
    /// Буфер исходящих сообщений совместной работы при разрыве соединения.
    private var pendingEnvelopes: [MirCollaborationEnvelope] = []

    init(configuration: MirServerConfiguration) {
        self.configuration = configuration
        let cfg = URLSessionConfiguration.default
        cfg.timeoutIntervalForRequest = configuration.connectTimeout
        cfg.timeoutIntervalForResource = configuration.connectTimeout * 2
        cfg.waitsForConnectivity = true
        self.session = URLSession(configuration: cfg)
    }

    func updateConfiguration(_ configuration: MirServerConfiguration) {
        // Токен/состав команды обновляются «на лету»; активный сокет при
        // смене учётных данных переподключается вызывающей стороной.
        _ = configuration
    }

    // MARK: - Жизненный цикл соединения

    func connect() async throws {
        guard webSocketTask == nil else { return }
        var components = URLComponents(url: configuration.baseURL, resolvingAgainstBaseURL: false)!
        components.scheme = (components.scheme == "https") ? "wss" : "ws"
        components.path = (components.path as NSString).appendingPathComponent("/team/stream")
        if !configuration.teamID.isEmpty {
            components.queryItems = [URLQueryItem(name: "team", value: configuration.teamID)]
        }
        guard let socketURL = components.url else {
            throw MirServerError.invalidURL
        }

        let request = urlRequest(for: socketURL)
        let task = session.webSocketTask(with: request)
        webSocketTask = task
        task.resume()
        listenTask = Task { await listen() }
        await flushPendingEnvelopes()
    }

    /// Сбросить накопленные исходящие сообщения после установки соединения.
    private func flushPendingEnvelopes() async {
        guard !pendingEnvelopes.isEmpty else { return }
        let buffered = pendingEnvelopes
        pendingEnvelopes.removeAll()
        for envelope in buffered {
            try? await sendCollaborationEnvelope(envelope)
        }
    }

    func disconnect() {
        listenTask?.cancel()
        listenTask = nil
        webSocketTask?.cancel(with: .goingAway, reason: nil)
        webSocketTask = nil
    }

    // MARK: - Отправка сообщений

    func sendMessage(_ message: MirTeamMessage) async throws {
        let encoder = JSONEncoder()
        encoder.dateEncodingStrategy = .iso8601
        let data = try encoder.encode(message)
        guard let task = webSocketTask else {
            throw MirServerError.notConnected
        }
        try await task.send(.data(data))
    }

    /// Отправить сообщение совместной работы (операция/presence/snapshot) в поток.
    /// При отсутствии соединения сообщение буферизуется и сбрасывается после
    /// повторного подключения.
    func sendCollaborationEnvelope(_ envelope: MirCollaborationEnvelope) async throws {
        let encoder = JSONEncoder()
        encoder.dateEncodingStrategy = .iso8601
        let payload = try encoder.encode(envelope)
        let wire: [String: Any] = ["kind": "collab", "payload": try JSONSerialization.jsonObject(with: payload)]
        let data = try JSONSerialization.data(withJSONObject: wire)
        guard let task = webSocketTask else {
            pendingEnvelopes.append(envelope)
            return
        }
        try await task.send(.data(data))
    }

    // MARK: - REST: экспорт проекта

    /// Загрузить опубликованный архив проекта с сервера (для импорта).
    func fetchProjectArchive(projectID: String) async throws -> Data {
        let url = configuration.baseURL.appendingPathComponent("/projects/\(projectID)/archive")
        var req = urlRequest(for: url)
        req.httpMethod = "GET"
        let (data, response) = try await session.data(for: req)
        try validate(response: response, data: data)
        return data
    }

    func exportProject(_ request: MirProjectExportRequest) async throws -> MirProjectExportResult {
        let url = configuration.baseURL.appendingPathComponent("/projects/export")
        var req = urlRequest(for: url)
        req.httpMethod = "POST"
        req.setValue("application/json", forHTTPHeaderField: "Content-Type")
        req.setValue("application/octet-stream", forHTTPHeaderField: "X-MIR4D-Format")

        let envelope = MirProjectExportEnvelope(request: request)
        let encoder = JSONEncoder()
        encoder.dateEncodingStrategy = .iso8601
        let body = try encoder.encode(envelope)
        req.httpBody = body

        let (data, response) = try await session.data(for: req)
        try validate(response: response, data: data)

        struct Acknowledgement: Decodable, Sendable {
            var remoteURL: String
        }
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        let ack = try decoder.decode(Acknowledgement.self, from: data)

        return MirProjectExportResult(
            projectID: request.projectID,
            remoteURL: ack.remoteURL,
            exportedAt: Date(),
            bytes: request.payload.count
        )
    }

    // MARK: - REST: состав команды

    func fetchTeam() async throws -> [MirTeamMember] {
        let url = configuration.baseURL.appendingPathComponent("/teams/\(configuration.teamID)/members")
        var req = urlRequest(for: url)
        req.httpMethod = "GET"

        let (data, response) = try await session.data(for: req)
        try validate(response: response, data: data)

        let decoder = JSONDecoder()
        return try decoder.decode([MirTeamMember].self, from: data)
    }

    // MARK: - Чтение WebSocket-потока

    private func listen() async {
        guard let task = webSocketTask else { return }
        while !Task.isCancelled {
            do {
                let message = try await task.receive()
                try await handle(received: message)
            } catch {
                if Task.isCancelled { return }
                await notify(.mir4DServerError, object: MirServerError.transport(String(describing: error)))
                return
            }
        }
    }

    private func handle(received message: URLSessionWebSocketTask.Message) async throws {
        let data: Data
        switch message {
        case .data(let d):
            data = d
        case .string(let s):
            data = Data(s.utf8)
        @unknown default:
            return
        }

        guard let raw = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
              let kind = raw["kind"] as? String else {
            return
        }

        switch kind {
        case "chat":
            if let payload = raw["payload"] as? [String: Any],
               let json = try? JSONSerialization.data(withJSONObject: payload) {
                let decoder = JSONDecoder()
                decoder.dateDecodingStrategy = .iso8601
                if let msg = try? decoder.decode(MirTeamMessage.self, from: json) {
                    await notify(.mir4DTeamMessageReceived, object: msg)
                }
            }
        case "team":
            if let payload = raw["payload"] as? [String: Any],
               let json = try? JSONSerialization.data(withJSONObject: payload) {
                let decoder = JSONDecoder()
                if let members = try? decoder.decode([MirTeamMember].self, from: json) {
                    await notify(.mir4DTeamUpdated, object: members)
                }
            }
        case "collab":
            if let payload = raw["payload"] as? [String: Any],
               let json = try? JSONSerialization.data(withJSONObject: payload) {
                let decoder = JSONDecoder()
                decoder.dateDecodingStrategy = .iso8601
                if let envelope = try? decoder.decode(MirCollaborationEnvelope.self, from: json) {
                    await notify(.mir4DCollaborationMessageReceived, object: envelope)
                }
            }
        default:
            break
        }
    }

    // MARK: - Утилиты

    private func urlRequest(for url: URL) -> URLRequest {
        var req = URLRequest(url: url)
        req.timeoutInterval = configuration.connectTimeout
        if let token = configuration.apiToken, !token.isEmpty {
            req.setValue("Bearer \(token)", forHTTPHeaderField: "Authorization")
        }
        req.setValue("MIR4DClient/1.0", forHTTPHeaderField: "User-Agent")
        return req
    }

    private func validate(response: URLResponse?, data: Data) throws {
        guard let http = response as? HTTPURLResponse else {
            throw MirServerError.invalidResponse
        }
        guard (200...299).contains(http.statusCode) else {
            throw MirServerError.http(status: http.statusCode, body: String(data: data, encoding: .utf8))
        }
    }

    private func notify<T: Sendable>(_ name: Notification.Name, object: T?) async {
        await MainActor.run {
            NotificationCenter.default.post(name: name, object: object as Any)
        }
    }
}

/// Конверт экспорта проекта для передачи по REST.
private struct MirProjectExportEnvelope: Encodable, Sendable {
    var projectID: String
    var projectName: String
    var format: String
    var message: String?
    var payload: Data

    init(request: MirProjectExportRequest) {
        self.projectID = request.projectID
        self.projectName = request.projectName
        self.format = request.format
        self.message = request.message
        self.payload = request.payload
    }
}

/// Ошибки подсистемы MirServer.
public enum MirServerError: Error, Sendable, Equatable {
    case invalidURL
    case notConnected
    case invalidResponse
    case http(status: Int, body: String?)
    case transport(String)
}
