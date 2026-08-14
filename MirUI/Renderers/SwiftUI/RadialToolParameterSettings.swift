import SwiftUI

struct RadialToolParameterSettings: Codable {
    var enabled = true
    var pointerSensitivity = 0.12
    var scrollSensitivity = 1.0
    var showReversedToggle = true
    var showSymmetricToggle = true
    var panelWidth = 350.0
    var panelHeight = 122.0
    var pointerOffset = 18.0
    var escapeCancels = true
    var returnCommits = true
}

@MainActor
final class RadialToolParameterSettingsStore: ObservableObject {
    static let shared = RadialToolParameterSettingsStore()

    @Published var settings: RadialToolParameterSettings {
        didSet { save() }
    }

    private let key = "MIR4D.RadialMenu.ParameterSettings"

    private init() {
        if let data = UserDefaults.standard.data(forKey: key),
           let decoded = try? JSONDecoder().decode(RadialToolParameterSettings.self, from: data) {
            settings = decoded
        } else {
            settings = RadialToolParameterSettings()
        }
    }

    private func save() {
        guard let data = try? JSONEncoder().encode(settings) else { return }
        UserDefaults.standard.set(data, forKey: key)
    }

    func reset() {
        settings = RadialToolParameterSettings()
    }
}
