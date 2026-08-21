import Foundation
import AppKit

@MainActor
public final class MirServerManager {
    public static let shared = MirServerManager()

    private(set) public var configuration: MirServerConfiguration
    private(set) public var status: MirServerConnectionStatus = .disconnected
    private(set) public var team: [MirTeamMember] = []

    private let transport: MirServerTransport
    private var statusObservation: NSObjectProtocol?
    private var reconnectAttempts = 0

    private let configKey = "MIR4D.MirServer.configuration"

    private init() {
        let configuration = Self.loadConfiguration(key: configKey)
        self.configuration = configuration
        self.transport = MirServerTransport(configuration: configuration)
    }

    public func configure(_ configuration: MirServerConfiguration) {
        self.configuration = configuration
        Self.saveConfiguration(configuration, key: configKey)
        Task { await transport.updateConfiguration(configuration) }
    }

    private static func loadConfiguration(key: String) -> MirServerConfiguration {
        guard let data = UserDefaults.standard.data(forKey: key),
              let cfg = try? JSONDecoder().decode(MirServerConfiguration.self, from: data) else {
            return MirServerConfiguration()
        }
        return cfg
    }

    private static func saveConfiguration(_ configuration: MirServerConfiguration, key: String) {
        if let data = try? JSONEncoder().encode(configuration) {
            UserDefaults.standard.set(data, forKey: key)
        }
    }

    private let maxReconnectAttempts = 5

    public func connect() async {
        guard status != .connected, status != .connecting else { return }
        setStatus(.connecting)
        do {
            try await transport.connect()
            setStatus(.connected)
            reconnectAttempts = 0
            await refreshTeam()
        } catch {
            setStatus(.failed(String(describing: error)))
            await scheduleReconnect()
        }
    }

    public func disconnect() {
        reconnectAttempts = maxReconnectAttempts
        Task { await transport.disconnect() }
        setStatus(.disconnected)
    }

    private func scheduleReconnect() async {
        guard reconnectAttempts < maxReconnectAttempts else { return }
        reconnectAttempts &+= 1
        let delay = min(pow(2.0, Double(reconnectAttempts)), 30.0)
        try? await Task.sleep(nanoseconds: UInt64(delay * 1_000_000_000))
        guard status != .disconnected else { return }
        await connect()
    }

    public func openProjectInBrowser(projectID: String) {
        var components = URLComponents(url: configuration.webAppURL, resolvingAgainstBaseURL: false)!
        components.path = (components.path as NSString).appendingPathComponent("/projects/\(projectID)")
        if let url = components.url {
            NSWorkspace.shared.open(url)
        }
    }

    @discardableResult
    public func sendMessage(_ text: String, projectID: String? = nil) async -> Bool {
        guard status == .connected else { return false }
        let message = MirTeamMessage(
            authorID: configuration.currentUserID,
            authorName: configuration.currentUserName,
            text: text,
            projectID: projectID
        )
        do {
            try await transport.sendMessage(message)
            return true
        } catch {
            NotificationCenter.default.post(name: .mir4DServerError, object: MirServerError.transport(String(describing: error)))
            return false
        }
    }

    public func broadcastCollaboration(_ envelope: MirCollaborationEnvelope) async throws {
        try await transport.sendCollaborationEnvelope(envelope)
    }

    public func fetchProjectArchive(projectID: String) async throws -> Data {
        try await transport.fetchProjectArchive(projectID: projectID)
    }

    public func refreshTeam() async {
        guard !configuration.teamID.isEmpty else { return }
        do {
            let members = try await transport.fetchTeam()
            self.team = members
            NotificationCenter.default.post(name: .mir4DTeamUpdated, object: members)
        } catch {
            NotificationCenter.default.post(name: .mir4DServerError, object: MirServerError.transport(String(describing: error)))
        }
    }

    @discardableResult
    public func exportProject(
        projectID: String,
        projectName: String,
        format: String,
        payload: Data,
        message: String? = nil
    ) async -> MirProjectExportResult? {
        let request = MirProjectExportRequest(
            projectID: projectID,
            projectName: projectName,
            format: format,
            payload: payload,
            message: message
        )
        do {
            let result = try await transport.exportProject(request)
            NotificationCenter.default.post(name: .mir4DProjectExported, object: result)
            return result
        } catch {
            NotificationCenter.default.post(name: .mir4DServerError, object: MirServerError.transport(String(describing: error)))
            return nil
        }
    }

    private func setStatus(_ status: MirServerConnectionStatus) {
        self.status = status
        NotificationCenter.default.post(name: .mir4DServerStatusChanged, object: status)
    }
}
