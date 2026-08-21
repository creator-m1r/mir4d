import SwiftUI
import Combine

@MainActor
final class MIR4DSculptSettings: ObservableObject {
    static let shared = MIR4DSculptSettings()

    @Published var radiusScale: Double = 0.25

    @Published var strengthScale: Double = 0.04
}
