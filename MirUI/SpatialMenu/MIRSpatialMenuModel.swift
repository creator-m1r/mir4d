import Foundation
import CoreGraphics

enum MIRSpatialMenuAnchor {
    static func center(in size: CGSize) -> CGPoint {
        CGPoint(x: size.width / 2, y: size.height / 2)
    }
}

enum MIRSpatialMenuLevel: Int, Equatable, Sendable {
    case intent = 0
    case category = 1
    case tool = 2

    var radius: Int { rawValue }
}

struct MIRSpatialMenuItem: Identifiable, Equatable, Sendable {
    let id: String
    let titleRU: String
    let titleEN: String
    let icon: String
    let command: String
    let children: [MIRSpatialMenuItem]
    var enabled: Bool

    init(
        id: String,
        titleRU: String,
        titleEN: String,
        icon: String,
        command: String,
        children: [MIRSpatialMenuItem] = [],
        enabled: Bool = true
    ) {
        self.id = id
        self.titleRU = titleRU
        self.titleEN = titleEN
        self.icon = icon
        self.command = command
        self.children = children
        self.enabled = enabled
    }

    func localizedTitle(_ language: String) -> String {
        language == "ru" ? titleRU : titleEN
    }
}

struct MIRSpatialMenuState: Equatable, Sendable {
    var level: MIRSpatialMenuLevel = .intent
    var intentIndex: Int?
    var categoryIndex: Int?
    var toolIndex: Int?
    var dx: Double = 0
    var dy: Double = 0

    var vector: CGVector {
        CGVector(dx: dx, dy: dy)
    }

    var angleRadians: Double {
        atan2(dy, dx)
    }

    var distance: Double {
        hypot(dx, dy)
    }

    var isCentred: Bool {
        level == .intent && intentIndex == nil
    }

    static let initial = MIRSpatialMenuState()
}

enum MIRSpatialMenuHighlightState: Equatable, Sendable {
    case idle
    case hover
    case selected
    case active
    case disabled
}

struct MIRSpatialMenuSettings: Codable, Equatable, Sendable {
    var enabled = true
    var keyboardTriggerEnabled = true
    var middleMouseTriggerEnabled = true
    var trackpadTriggerEnabled = true
    var hapticEnabled = true
    var showLabels = true
    var voiceEnabled = true
    var handEnabled = true

    var deadZone: Double = 26

    var intentRadius: Double = 96

    var categoryOffset: Double = 186

    var toolOffset: Double = 292

    var activationDelay: Double = 0.18

    var magneticStrength: Double = 0.72
    var magneticHysteresis: Double = 0.08

    var maxLevel1Segments: Int = 8

    static let `default` = MIRSpatialMenuSettings()
}

@MainActor
final class MIRSpatialMenuSettingsStore: ObservableObject {
    static let shared = MIRSpatialMenuSettingsStore()

    @Published var settings: MIRSpatialMenuSettings {
        didSet { save() }
    }

    private let key = "MIR4D.SpatialMenu.Settings"

    private init() {
        if let data = UserDefaults.standard.data(forKey: key),
           let value = try? JSONDecoder().decode(MIRSpatialMenuSettings.self, from: data) {
            settings = value
        } else {
            settings = .default
        }
    }

    func reset() {
        settings = .default
    }

    private func save() {
        guard let data = try? JSONEncoder().encode(settings) else { return }
        UserDefaults.standard.set(data, forKey: key)
    }
}