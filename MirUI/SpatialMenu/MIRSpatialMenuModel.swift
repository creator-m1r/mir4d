import Foundation
import CoreGraphics

/// The spatial menu is anchored to the centre of the display, never to the cursor.
/// `MIRSpatialMenuAnchor` is fixed relative to the viewport.
enum MIRSpatialMenuAnchor {
    static func center(in size: CGSize) -> CGPoint {
        CGPoint(x: size.width / 2, y: size.height / 2)
    }
}

/// Depth of the fan: first radius expresses intentions, deeper radii express tools.
enum MIRSpatialMenuLevel: Int, Equatable, Sendable {
    case intent = 0
    case category = 1
    case tool = 2

    var radius: Int { rawValue }
}

/// One node of the spatial intent tree.
/// Levels: intent → category → tool. Commands are existing MIR 4D command ids.
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

/// The live selection state of the spatial gesture.
/// All fields are value types so the state is Sendable and testable.
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

/// Idle / hover / selected / active / disabled — the full highlight palette of a fan segment.
enum MIRSpatialMenuHighlightState: Equatable, Sendable {
    case idle
    case hover
    case selected
    case active
    case disabled
}

/// Spatial menu configuration. Codable so the engineer can persist it locally.
struct MIRSpatialMenuSettings: Codable, Equatable, Sendable {
    var enabled = true
    var keyboardTriggerEnabled = true
    var middleMouseTriggerEnabled = true
    var trackpadTriggerEnabled = true
    var hapticEnabled = true
    var showLabels = true
    var voiceEnabled = true
    var handEnabled = true

    /// Dead zone around the centre: the fan stays idle until the finger leaves it.
    var deadZone: Double = 26
    /// Radius of the first (intention) ring.
    var intentRadius: Double = 96
    /// Offset of the second ring along the movement direction.
    var categoryOffset: Double = 186
    /// Offset of the third ring along the movement direction.
    var toolOffset: Double = 292

    /// Hold time before activation for touch input. Recommended 0.35...0.60 sec on iPad.
    /// Keyboard and middle-mouse activation stay immediate.
    var activationDelay: Double = 0.18

    var magneticStrength: Double = 0.72
    var magneticHysteresis: Double = 0.08

    /// The fan never shows more than 8 segments on the first radius.
    var maxLevel1Segments: Int = 8

    static let `default` = MIRSpatialMenuSettings()
}

/// Persistent store for the spatial menu configuration.
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