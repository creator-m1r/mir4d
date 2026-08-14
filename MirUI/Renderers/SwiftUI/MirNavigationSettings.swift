import Foundation
import Combine

/// Trackpad navigation scheme, Blender-style:
/// - two fingers drag -> orbit (default, like Blender's trackpad rotate)
/// - Shift + two fingers -> pan
/// - Control + two fingers -> zoom
/// - Option + left mouse drag -> orbit (macOS middle-button emulation)
/// Wheel and pinch keep their zoom behavior.
enum MirTrackpadGesture: String, Codable, CaseIterable, Identifiable {
    case orbit
    case pan
    case zoom

    var id: String { rawValue }

    var titleRU: String {
        switch self {
        case .orbit: return "Поворот"
        case .pan: return "Панорама"
        case .zoom: return "Масштаб"
        }
    }

    var titleEN: String {
        switch self {
        case .orbit: return "Orbit"
        case .pan: return "Pan"
        case .zoom: return "Zoom"
        }
    }
}

struct MirNavigationSettings: Codable, Equatable {
    var trackpadGesture: MirTrackpadGesture = .orbit
    var orbitSensitivity: Double = 0.0045
    var zoomSensitivity: Double = 0.012
    var invertOrbitX: Bool = false
    var invertOrbitY: Bool = false
    var invertZoom: Bool = false
}

@MainActor
final class MirNavigationSettingsStore: ObservableObject {
    static let shared = MirNavigationSettingsStore()

    @Published var settings: MirNavigationSettings {
        didSet { save() }
    }

    private let key = "MIR4D.Navigation.Settings"

    private init() {
        if let data = UserDefaults.standard.data(forKey: key),
           let value = try? JSONDecoder().decode(MirNavigationSettings.self, from: data) {
            settings = value
        } else {
            settings = MirNavigationSettings()
        }
    }

    func reset() {
        settings = MirNavigationSettings()
    }

    private func save() {
        guard let data = try? JSONEncoder().encode(settings) else { return }
        UserDefaults.standard.set(data, forKey: key)
    }
}