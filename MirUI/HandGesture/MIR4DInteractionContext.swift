import Foundation

public enum MIR4DInteractionTarget: String, Sendable, Hashable, CaseIterable {
    case empty
    case object
    case face
    case edge
    case sketch
    case window
    case timeline
    case sculpt
    case parameter
    case navigation
}

public enum MIR4DInteractionAction: String, Sendable, Hashable {
    case openMenu
    case moveObject
    case selectFace
    case selectEdge
    case cameraControl
    case sculpt
    case paint
    case editParameter
    case navigate
    case confirm
    case cancel
}

public struct MIR4DInteractionContext: Sendable {
    public let target: MIR4DInteractionTarget

    public init(target: MIR4DInteractionTarget) {
        self.target = target
    }

    public func resolve(gesture: MIRHandGestureType, phase: MIRHandIntentPhase) -> MIR4DInteractionAction {
        let isRelease = phase == .ended || phase == .cancelled

        switch (target, gesture) {

        case (.empty, .pinch):
            return .cameraControl
        case (.empty, .point), (.empty, .openPalm):
            return .openMenu

        case (.object, .pinch):
            return isRelease ? .confirm : .moveObject
        case (.face, .pinch):
            return isRelease ? .confirm : .selectFace
        case (.edge, .pinch):
            return isRelease ? .confirm : .selectEdge

        case (.sculpt, .pinch), (.sculpt, .grab):
            return isRelease ? .confirm : .sculpt

        case (.sketch, .pinch):
            return isRelease ? .confirm : .paint

        case (.parameter, .pinch):
            return isRelease ? .confirm : .editParameter

        case (.navigation, .pinch):
            return .navigate
        case (.window, .pinch), (.timeline, .pinch):
            return .openMenu

        case (_, .twoHandScale):
            return .navigate
        case (_, .twoHandRotate):
            return .navigate
        case (_, .twoHandTranslate):
            return .navigate
        case (_, .twoHandPinch):
            return isRelease ? .confirm : .navigate

        default:
            return .openMenu
        }
    }
}
