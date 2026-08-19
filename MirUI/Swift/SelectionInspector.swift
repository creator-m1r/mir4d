import Foundation
import Combine

public struct SelectionInspectorProperty: Identifiable, Equatable {
    public let id: String
    public let name: String
    public let value: String

    public init(id: String, name: String, value: String) {
        self.id = id
        self.name = name
        self.value = value
    }
}

public final class SelectionInspector: ObservableObject {
    @Published public private(set) var properties: [SelectionInspectorProperty] = []

    private var handle: OpaquePointer?

    public init() {
        handle = mirui_selection_inspector_create()
        reload()
    }

    deinit {
        if let handle {
            mirui_selection_inspector_destroy(handle)
        }
    }

    public func clear() {
        guard let handle else {
            properties = []
            return
        }

        mirui_selection_inspector_clear(handle)
        reload()
    }

    public func updateFace(
        id: UInt64,
        triangleCount: UInt64,
        area: Double,
        centerX: Double,
        centerY: Double,
        centerZ: Double,
        normalX: Double,
        normalY: Double,
        normalZ: Double
    ) {
        guard let handle else { return }

        mirui_selection_inspector_update_face(
            handle,
            id,
            triangleCount,
            area,
            centerX,
            centerY,
            centerZ,
            normalX,
            normalY,
            normalZ
        )

        reload()
    }

    private func reload() {
        guard let handle else {
            properties = []
            return
        }

        let count = mirui_selection_inspector_property_count(handle)
        var result: [SelectionInspectorProperty] = []
        result.reserveCapacity(count)

        for index in 0..<count {
            guard
                let namePointer = mirui_selection_inspector_property_name(handle, index),
                let valuePointer = mirui_selection_inspector_property_value(handle, index)
            else {
                continue
            }

            let name = mirCString(namePointer) ?? ""
            let value = mirCString(valuePointer) ?? ""

            result.append(
                SelectionInspectorProperty(
                    id: "selection.\(index)",
                    name: name,
                    value: value
                )
            )
        }

        properties = result
    }
}
