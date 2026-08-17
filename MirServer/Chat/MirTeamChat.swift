import Foundation

/// Командный чат инженеров поверх MirServer.
///
/// Удобная надстройка над `MirServerManager`: хранит ленту сообщений,
/// автоматически подписывается на входящие сообщения Event Bus и
/// предоставляет потокобезопасный `@MainActor`-интерфейс для UI.
@MainActor
public final class MirTeamChat {
    public static let shared = MirTeamChat()

    private(set) public var messages: [MirTeamMessage] = []

    private let manager = MirServerManager.shared
    private var messageObserver: NSObjectProtocol?

    private init() {
        messageObserver = NotificationCenter.default.addObserver(
            forName: .mir4DTeamMessageReceived,
            object: nil,
            queue: .main
        ) { [weak self] note in
            guard let message = note.object as? MirTeamMessage else { return }
            MainActor.assumeIsolated {
                self?.append(message)
            }
        }
    }

    deinit {
        MainActor.assumeIsolated {
            if let observer = messageObserver {
                NotificationCenter.default.removeObserver(observer)
            }
        }
    }

    /// Отправить текстовое сообщение команде.
    @discardableResult
    public func send(_ text: String, projectID: String? = nil) async -> Bool {
        await manager.sendMessage(text, projectID: projectID)
    }

    /// История сообщений, относящихся к конкретному проекту.
    public func messages(forProject projectID: String) -> [MirTeamMessage] {
        messages.filter { $0.projectID == projectID }
    }

    /// Очистить локальную ленту (не влияет на сервер).
    public func clearLocalHistory() {
        messages.removeAll()
    }

    private func append(_ message: MirTeamMessage) {
        messages.append(message)
        if messages.count > 1000 {
            messages.removeFirst(messages.count - 1000)
        }
    }
}
