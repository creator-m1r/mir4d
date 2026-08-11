// MirUI/Renderers/SwiftUI/MirUISwiftBridge.swift
// 🍏 Swift-мост: превращает C++-описание интерфейса в живые SwiftUI Views.
//
// Этот файл — главное связующее звено между миром C++ и миром SwiftUI.
// Когда C++-рендерер (SwiftUIRenderer) завершает обход дерева виджетов,
// он вызывает C-функцию MirUI_SwiftUI_UpdateViewNodes, передавая плоский список
// узлов (SwiftUIViewNode). Здесь, на стороне Swift, этот список превращается
// в иерархию настоящих View: кнопок, надписей, контейнеров.
//
// Как это работает (по шагам):
//
//   1. C++ вызывает MirUI_SwiftUI_UpdateViewNodes(nodes, count, rootIndex).
//      Эта функция (написанная на C, но доступная в Swift через bridging header)
//      сохраняет узлы во внутреннее хранилище и запускает перестроение UI.
//
//   2. MirUIBridge (синглтон) хранит текущие узлы и публикует их через
//      @Published свойство, за которым наблюдает SwiftUI.
//
//   3. Головная View (MirUIRootView) читает узлы из MirUIBridge и для каждого
//      создаёт соответствующий SwiftUI View с помощью фабрики ViewFactory.
//      Фабрика по строке типа ("Button", "Label", "Container"...) создаёт
//      конкретный View и настраивает его свойства (текст, размер, позиция).
//
//   4. Позиционирование: поскольку C++-дерево использует абсолютные координаты,
//      мы временно применяем .position(x:y:) и .frame(width:height:).
//      В будущем можно заменить на Auto Layout через HStack/VStack.
//
//   5. Команды: когда кнопка нажимается, она вызывает C-функцию
//      MirUI_ExecuteCommand(commandId, widgetId), которая возвращает управление
//      в C++-ядро, где CommandBus выполняет нужную команду, изменяет дерево,
//      и цикл повторяется.
//
// ВАЖНО: чтобы Swift увидел C-функции, они должны быть объявлены в
// bridging header (например, MirUI-Bridging-Header.h) с прототипами:
//   void MirUI_SwiftUI_UpdateViewNodes(const void* nodes, int count, int rootIndex);
//   void MirUI_ExecuteCommand(const char* commandId, int64_t widgetId);
//
// Для простоты в этом файле мы используем условную компиляцию и заглушки,
// которые можно заменить реальными вызовами после настройки проекта.

import SwiftUI
import Foundation

// ─── C-функции, приходящие из C++ ─────────────────────────────
// Объявляем сигнатуры, чтобы можно было вызывать из Swift.
// В реальном проекте они подтянутся через bridging header,
// здесь мы просто объявляем их как внешние.
@_silgen_name("MirUI_SwiftUI_UpdateViewNodes")
func MirUI_SwiftUI_UpdateViewNodes(_ nodes: UnsafeRawPointer, _ count: Int32, _ rootIndex: Int32)

@_silgen_name("MirUI_ExecuteCommand")
func MirUI_ExecuteCommand(_ commandId: UnsafePointer<CChar>, _ widgetId: Int64)


// ─── Структура, идентичная C++ SwiftUIViewNode ────────────────
// Должна полностью соответствовать полям из C++ (порядок, типы).
// Используем её для получения данных от C++.
struct ViewNode {
    var type: String = ""
    var widgetId: Int64 = 0
    var text: String = ""
    var iconName: String = ""
    var commandId: String = ""
    var x: Double = 0
    var y: Double = 0
    var width: Double = 0
    var height: Double = 0
    var visible: Bool = true
    var parentIndex: Int32 = -1
}

// ─── Синглтон-хранилище узлов ────────────────────────────────
// MirUIBridge хранит текущее состояние всех узлов и уведомляет
// SwiftUI об изменениях через @Published.
class MirUIBridge: ObservableObject {
    static let shared = MirUIBridge()
    
    @Published var nodes: [ViewNode] = []
    @Published var rootIndex: Int = 0
    
    private init() {}
    
    // Вызывается из C-функции MirUI_SwiftUI_UpdateViewNodes
    func updateNodes(_ newNodes: [ViewNode], root: Int) {
        nodes = newNodes
        rootIndex = root
    }
}

// ─── Преобразование сырых данных C в массив ViewNode ──────────
// Эта функция вызывается из C-обёртки, когда приходят новые узлы.
// Она интерпретирует переданный буфер как массив ViewNode.
func convertToViewNodes(ptr: UnsafeRawPointer, count: Int) -> [ViewNode] {
    let buffer = ptr.bindMemory(to: ViewNode.self, capacity: count)
    return Array(UnsafeBufferPointer(start: buffer, count: count))
}

// ─── C-обёртка, которая будет вызвана из C++ ──────────────────
// Настоящее определение для C-функции, экспортируемое в Swift.
// Оно вызывает синглтон MirUIBridge для обновления состояния.
@_cdecl("MirUI_SwiftUI_UpdateViewNodes")
public func MirUI_SwiftUI_UpdateViewNodes_C(_ nodes: UnsafeRawPointer, _ count: Int32, _ rootIndex: Int32) {
    let viewNodes = convertToViewNodes(ptr: nodes, count: Int(count))
    DispatchQueue.main.async {
        MirUIBridge.shared.updateNodes(viewNodes, root: Int(rootIndex))
    }
}

// ─── Фабрика Views ────────────────────────────────────────────
// Получает один ViewNode и возвращает соответствующий SwiftUI View.
// Пока поддерживаем основные типы: Button, Label, Container, Toolbar.
struct ViewFactory {
    @ViewBuilder
    static func makeView(for node: ViewNode, allNodes: [ViewNode]) -> some View {
        Group {
            switch node.type {
            case "Button":
                Button(action: {
                    // Выполняем команду, связанную с кнопкой
                    if !node.commandId.isEmpty {
                        node.commandId.withCString { cStr in
                            MirUI_ExecuteCommand(cStr, node.widgetId)
                        }
                    }
                }) {
                    Text(node.text)
                        .frame(width: node.width, height: node.height)
                        .background(Color.blue)
                        .foregroundColor(.white)
                        .cornerRadius(8)
                }
                .position(x: node.x + node.width/2, y: node.y + node.height/2)
                .opacity(node.visible ? 1 : 0)
                
            case "Label":
                Text(node.text)
                    .frame(width: node.width, height: node.height, alignment: .leading)
                    .position(x: node.x + node.width/2, y: node.y + node.height/2)
                    .opacity(node.visible ? 1 : 0)
                
            case "Container", "Toolbar":
                // Контейнер не рисует себя, но содержит детей.
                // Дети будут отрисованы отдельно через иерархию.
                // Здесь можно добавить фон, если нужно.
                Rectangle()
                    .fill(Color.gray.opacity(0.1))
                    .frame(width: node.width, height: node.height)
                    .position(x: node.x + node.width/2, y: node.y + node.height/2)
                    .opacity(node.visible ? 1 : 0)
                
            default:
                EmptyView()
            }
        }
    }
}

// ─── Рекурсивное построение иерархии ─────────────────────────
// Получает индекс узла и весь список, отрисовывает узел и его детей.
func buildHierarchy(nodeIndex: Int, allNodes: [ViewNode]) -> AnyView {
    let node = allNodes[nodeIndex]
    var childrenViews: [AnyView] = []
    
    // Ищем всех детей (узлы, у которых parentIndex == nodeIndex)
    for (index, other) in allNodes.enumerated() where other.parentIndex == nodeIndex {
        childrenViews.append(buildHierarchy(nodeIndex: index, allNodes: allNodes))
    }
    
    // Если есть дети, оборачиваем в ZStack с позиционированием.
    if !childrenViews.isEmpty {
        return AnyView(
            ZStack {
                ViewFactory.makeView(for: node, allNodes: allNodes)
                ForEach(0..<childrenViews.count, id: \.self) { idx in
                    childrenViews[idx]
                }
            }
        )
    } else {
        return AnyView(ViewFactory.makeView(for: node, allNodes: allNodes))
    }
}

// ─── Корневая View, отображающая весь MirUI интерфейс ─────────
// Подписывается на изменения в MirUIBridge и перестраивает всё дерево.
struct MirUIRootView: View {
    @ObservedObject var bridge = MirUIBridge.shared
    
    var body: some View {
        ZStack {
            if bridge.nodes.isEmpty {
                Text("Интерфейс пуст")
            } else {
                // Строим иерархию начиная с корневого индекса
                buildHierarchy(nodeIndex: bridge.rootIndex, allNodes: bridge.nodes)
            }
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }
}

// ─── Вспомогательная обёртка для C-функции выполнения команды ──
// В реальном проекте вызов будет идти через C++ CommandBus.
// Здесь просто печатаем в лог.
func MirUI_ExecuteCommand_C(_ commandId: UnsafePointer<CChar>, _ widgetId: Int64) {
    let cmd = String(cString: commandId)
    print("[Swift] Выполнить команду '\(cmd)' для виджета \(widgetId)")
    // Далее нужно вызвать C++-функцию CommandBus::execute.
    // Пока заглушка.
}

// ─── Экспорт обёртки для C (если нужно) ──────────────────────
@_cdecl("MirUI_ExecuteCommand")
public func MirUI_ExecuteCommand_CWrapper(_ commandId: UnsafePointer<CChar>, _ widgetId: Int64) {
    MirUI_ExecuteCommand_C(commandId, widgetId)
}

// ─── Примечания по интеграции ─────────────────────────────────
// 1. Добавить этот файл в Swift-пакет или проект Xcode.
// 2. Убедиться, что в bridging header объявлены:
//      void MirUI_SwiftUI_UpdateViewNodes(const void* nodes, int count, int rootIndex);
//      void MirUI_ExecuteCommand(const char* commandId, int64_t widgetId);
// 3. В C++ коде (SwiftUIRenderer.mm) раскомментировать вызов
//    MirUI_SwiftUI_UpdateViewNodes и передавать реальный массив ViewNode.
// 4. В App.swift использовать MirUIRootView как основное содержимое окна.