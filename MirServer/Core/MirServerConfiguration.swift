import Foundation

/// Конфигурация подключения к серверу MIR 4D (сайт + API).
///
/// Тип хранит только значения (`Sendable`), поэтому его можно безопасно
/// передавать в сетевой `actor` транспорта.
public struct MirServerConfiguration: Sendable, Codable {
    /// Базовый URL REST/JSON API сервера, например `https://mir4d.cloud/api`.
    public var baseURL: URL

    /// URL веб-приложения для открытия проекта в браузере.
    public var webAppURL: URL

    /// Токен авторизации API (Bearer). Пуст — если пользователь не вошёл.
    public var apiToken: String?

    /// Идентификатор команды инженеров.
    public var teamID: String

    /// Идентификатор текущего пользователя.
    public var currentUserID: String

    /// Отображаемое имя текущего пользователя.
    public var currentUserName: String

    /// Таймаут сетевых операций, сек.
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
