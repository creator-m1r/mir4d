import Foundation
import SwiftUI

struct MIR4DUIRootConfig: Codable {
    struct Window: Codable {
        var minWidth: Double = 1400
        var minHeight: Double = 900
        var title: String = "МИР 4D"
    }

    struct Editor: Codable {
        var enabled: Bool = false
        var showGrid: Bool = true
        var snap: Double = 8
        var allowMove: Bool = true
        var allowResize: Bool = true
        var allowScale: Bool = true
        var saveOnChange: Bool = true
    }

    var version: Int = 1
    var window = Window()
    var editor = Editor()
}

struct MIR4DPanelConfig: Codable, Identifiable {
    let id: String
    var title: String
    var visible: Bool
    var x: Double
    var y: Double
    var width: Double
    var height: Double
    var minWidth: Double
    var minHeight: Double
    var maxWidth: Double?
    var maxHeight: Double?
    var scale: Double
    var opacity: Double
    var padding: Double
    var spacing: Double
    var cornerRadius: Double
    var background: String
    var foreground: String
    var accent: String
    var dock: String
    var resizable: Bool
    var movable: Bool
    var scalable: Bool
}

@MainActor
final class MIR4DUIConfigStore: ObservableObject {
    @Published private(set) var root: MIR4DUIRootConfig
    @Published private(set) var panels: [String: MIR4DPanelConfig] = [:]

    init() {
        root = Self.loadRoot()
        panels = Self.loadPanels()
    }

    func panel(_ id: String) -> MIR4DPanelConfig? {
        panels[id]
    }

    func setEditorEnabled(_ enabled: Bool) {
        root.editor.enabled = enabled
    }

    func updatePanel(_ config: MIR4DPanelConfig) {
        panels[config.id] = config
    }

    private static func loadRoot() -> MIR4DUIRootConfig {
        load("MIR4DUIConfig", subdirectory: "UIConfig") ?? MIR4DUIRootConfig()
    }

    private static func loadPanels() -> [String: MIR4DPanelConfig] {
        let names = ["TopBar", "ProjectTree", "Viewport", "Inspector", "TimePanel"]
        var result: [String: MIR4DPanelConfig] = [:]
        for name in names {
            if let config: MIR4DPanelConfig = load(name, subdirectory: "UIConfig") {
                result[config.id] = config
            }
        }
        return result
    }

    private static func load<T: Decodable>(_ name: String, subdirectory: String) -> T? {
        guard let url = Bundle.module.url(forResource: name, withExtension: "json", subdirectory: subdirectory),
              let data = try? Data(contentsOf: url),
              let value = try? JSONDecoder().decode(T.self, from: data) else {
            return nil
        }
        return value
    }
}

extension Color {
    init(mirHex: String) {
        let value = mirHex.trimmingCharacters(in: CharacterSet.alphanumerics.inverted)
        var number: UInt64 = 0
        Scanner(string: value).scanHexInt64(&number)
        let red = Double((number >> 16) & 0xFF) / 255
        let green = Double((number >> 8) & 0xFF) / 255
        let blue = Double(number & 0xFF) / 255
        self.init(red: red, green: green, blue: blue)
    }
}

struct MIR4DPanelFrameModifier: ViewModifier {
    let config: MIR4DPanelConfig

    func body(content: Content) -> some View {
        content
            .opacity(config.visible ? config.opacity : 0)
            .scaleEffect(config.scale)
            .padding(config.padding)
    }
}

extension View {
    func mirPanel(_ config: MIR4DPanelConfig) -> some View {
        modifier(MIR4DPanelFrameModifier(config: config))
    }
}
