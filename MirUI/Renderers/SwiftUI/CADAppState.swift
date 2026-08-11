import SwiftUI
import Combine

enum NotificationType { case info, success, warning, error }

struct CADNotification: Identifiable {
    let id = UUID()
    let message: String
    let type: NotificationType
}

class CADAppState: ObservableObject {
    @Published var selectedTool: String = "select"
    @Published var selectedTreeItem: String = "Система жизнеобеспечения"
    @Published var activePropTab: Int = 0
    @Published var notifications: [CADNotification] = []
    @Published var isPlaying: Bool = false
    
    // Дерево проекта (пример)
    let treeData: [TreeNodeData] = [
        TreeNodeData(name: "Проект", icon: "cube", children: [
            TreeNodeData(name: "Космический шлем", icon: "square.stack.3d.up", status: .inProgress, children: [
                TreeNodeData(name: "Корпус", icon: "square", status: .approved),
                TreeNodeData(name: "Визор", icon: "square", status: .approved),
                TreeNodeData(name: "Система жизнеобеспечения", icon: "square", status: .inProgress)
            ]),
            TreeNodeData(name: "Сборочные единицы", icon: "rectangle.split.3x1", status: .approved),
            TreeNodeData(name: "Электроника", icon: "cpu", status: .issue),
            TreeNodeData(name: "Конструкторская документация", icon: "doc.text")
        ])
    ]
    
    func showNotification(_ msg: String, type: NotificationType = .info) {
        let n = CADNotification(message: msg, type: type)
        notifications.append(n)
        DispatchQueue.main.asyncAfter(deadline: .now() + 3.5) {
            self.notifications.removeAll { $0.id == n.id }
        }
    }
    
    func loadModel(url: URL) {
        // Здесь будет вызов AssimpImporter через C-API
        showNotification("Загрузка модели: \(url.lastPathComponent)", type: .info)
        // TODO: MirEngineLoadModel(url.path)
        showNotification("Модель успешно добавлена в сцену", type: .success)
    }
}

struct TreeNodeData: Identifiable {
    let id = UUID()
    let name: String
    let icon: String
    var status: Status = .none
    var children: [TreeNodeData] = []
    
    enum Status { case none, approved, inProgress, issue }
}