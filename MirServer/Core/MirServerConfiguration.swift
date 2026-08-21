import Foundation

public struct MirServerConfiguration: Sendable, Codable {

    public var baseURL: URL

    public var webAppURL: URL

    public var apiToken: String?

    public var teamID: String

    public var currentUserID: String

    public var currentUserName: String

    public var connectTimeout: TimeInterval

    public init(
        baseURL: URL = URL(string: "https://mir4d.cloud/api")!,
        webAppURL: URL = URL(string: "https://mir4d.cloud")!,
        apiToken: String? = nil,
        teamID: String = "",
        currentUserID: String = "",
        currentUserName: String = "",
        connectTimeout: TimeInterval = 30
    ) {
        self.baseURL = baseURL
        self.webAppURL = webAppURL
        self.apiToken = apiToken
        self.teamID = teamID
        self.currentUserID = currentUserID
        self.currentUserName = currentUserName
        self.connectTimeout = connectTimeout
    }
}
