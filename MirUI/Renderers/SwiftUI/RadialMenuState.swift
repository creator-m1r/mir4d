import Foundation
import SwiftUI

enum RadialMenuLevel: String, CaseIterable, Equatable {
    case root
    case panel
    case tool
    case parameter
    case confirmation
}

struct RadialMenuState {
    var isOpen: Bool = false
    var level: RadialMenuLevel = .root
    var selectedPanelID: UUID?
    var selectedToolID: UUID?
    var selectedParameterID: UUID?

    var cursorVector: CGVector = .zero
    var cursorDistance: Double = 0
    var cursorAngle: Double = 0

    var hoveredPanelID: UUID?
    var hoveredToolID: UUID?

    var lockedPanelID: UUID?
    var lockedToolID: UUID?

    var isInsideDeadZone: Bool = false
    var isInsideActivationZone: Bool = false

    var context: RadialMenuContext?
}
