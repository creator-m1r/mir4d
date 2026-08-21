// MirUI/Renderers/SwiftUI/MirUISwiftBridge.swift
// Swift-мост C++ → SwiftUI для плоского списка UI-узлов.

import SwiftUI
import Foundation

struct ViewNode: Sendable {
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

@MainActor
final class MirUIBridge: ObservableObject {
    static let shared = MirUIBridge()

    @Published var nodes: [ViewNode] = []
    @Published var rootIndex: Int = 0

    private init() {}

    func updateNodes(_ newNodes: [ViewNode], root: Int) {
        nodes = newNodes
        rootIndex = root
    }
}

func convertToViewNodes(ptr: UnsafeRawPointer, count: Int) -> [ViewNode] {
    guard count > 0 else { return [] }
    let buffer = ptr.bindMemory(to: ViewNode.self, capacity: count)
    return Array(UnsafeBufferPointer(start: buffer, count: count))
}

@_cdecl("MirUI_SwiftUI_UpdateViewNodes")
public func MirUI_SwiftUI_UpdateViewNodes_C(_ nodes: UnsafeRawPointer, _ count: Int32, _ rootIndex: Int32) {
    let viewNodes = convertToViewNodes(ptr: nodes, count: Int(count))
    Task { @MainActor in
        MirUIBridge.shared.updateNodes(viewNodes, root: Int(rootIndex))
    }
}

struct ViewFactory {
    @ViewBuilder
    static func makeView(for node: ViewNode, allNodes: [ViewNode]) -> some View {
        Group {
            switch node.type {
            case "Button":
                Button(action: {
                    guard !node.commandId.isEmpty else { return }
                    node.commandId.withCString { cStr in
                        MirUI_ExecuteCommand(cStr, node.widgetId)
                    }
                }) {
                    Text(node.text)
                        .frame(width: node.width, height: node.height)
                        .background(Color.blue)
                        .foregroundColor(.white)
                        .cornerRadius(8)
                }
                .position(x: node.x + node.width / 2, y: node.y + node.height / 2)
                .opacity(node.visible ? 1 : 0)

            case "Label":
                Text(node.text)
                    .frame(width: node.width, height: node.height, alignment: .leading)
                    .position(x: node.x + node.width / 2, y: node.y + node.height / 2)
                    .opacity(node.visible ? 1 : 0)

            case "Container", "Toolbar":
                Rectangle()
                    .fill(Color.gray.opacity(0.1))
                    .frame(width: node.width, height: node.height)
                    .position(x: node.x + node.width / 2, y: node.y + node.height / 2)
                    .opacity(node.visible ? 1 : 0)

            default:
                EmptyView()
            }
        }
    }
}

func buildHierarchy(nodeIndex: Int, allNodes: [ViewNode]) -> AnyView {
    guard allNodes.indices.contains(nodeIndex) else {
        return AnyView(EmptyView())
    }

    let node = allNodes[nodeIndex]
    let children = allNodes.enumerated()
        .filter { $0.element.parentIndex == Int32(nodeIndex) }
        .map { buildHierarchy(nodeIndex: $0.offset, allNodes: allNodes) }

    if children.isEmpty {
        return AnyView(ViewFactory.makeView(for: node, allNodes: allNodes))
    }

    return AnyView(
        ZStack {
            ViewFactory.makeView(for: node, allNodes: allNodes)
            ForEach(Array(children.enumerated()), id: \.offset) { _, child in
                child
            }
        }
    )
}

@MainActor
struct MirUIRootView: View {
    @ObservedObject private var bridge = MirUIBridge.shared

    var body: some View {
        ZStack {
            if bridge.nodes.indices.contains(bridge.rootIndex) {
                buildHierarchy(nodeIndex: bridge.rootIndex, allNodes: bridge.nodes)
            } else {
                EmptyView()
            }
        }
    }
}
