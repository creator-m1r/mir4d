import Foundation

/// What a hand interaction is performed over. Pure, scene-independent enum so
/// the resolver never needs to touch the CAD model.
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

/// Semantic action derived from a raw gesture within an interaction context.
///
/// This is the device-independent "what the user means", distinct from the raw
/// `MIRHandGestureType` and from any concrete CAD command. Downstream consumers
/// (Spatial Menu, CAD) decide what to actually execute.
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

/// Resolves a raw hand gesture together with the current scene context into a
/// semantic action. Pure and `Sendable`: it performs no CAD queries and can run
/// on any actor, keeping recognition off the main thread.
///
/// Canonical mappings (see hand-gesture architecture):
/// - `PINCH + OBJECT` → `moveObject`
/// - `PINCH + FACE`   → `selectFace`
/// - `PINCH + EDGE`   → `selectEdge`
/// - `PINCH + SCULPT` → `sculpt`
/// - `PINCH + EMPTY`  → `cameraControl`
public struct MIR4DInteractionContext: Sendable {
    public let target: MIR4DInteractionTarget

    public init(target: MIR4DInteractionTarget) {
        self.target = target
    }

    public func resolve(gesture: MIRHandGestureType, phase: MIRHandIntentPhase) -> MIR4DInteractionAction {
        let isRelease = phase == .ended || phase == .cancelled

        switch (target, gesture) {
        // Empty space: a pinch orbits the camera, an open palm / point opens the menu.
        case (.empty, .pinch):
            return .cameraControl
        case (.empty, .point), (.empty, .openPalm):
            return .openMenu

        // Geometry: pinch grabs and manipulates; releasing confirms.
        case (.object, .pinch):
            return isRelease ? .confirm : .moveObject
        case (.face, .pinch):
            return isRelease ? .confirm : .selectFace
        case (.edge, .pinch):
            return isRelease ? .confirm : .selectEdge

        // Sculpting surface: pinch / grab deforms the matter.
        case (.sculpt, .pinch), (.sculpt, .grab):
            return isRelease ? .confirm : .sculpt

        // Sketch: pinch paints / draws.
        case (.sketch, .pinch):
            return isRelease ? .confirm : .paint

        // Parameter editing: pinch tweaks the value.
        case (.parameter, .pinch):
            return isRelease ? .confirm : .editParameter

        // Navigation zones and chrome: pinch navigates / opens.
        case (.navigation, .pinch):
            return .navigate
        case (.window, .pinch), (.timeline, .pinch):
            return .openMenu

        // Two-hand relational gestures drive navigation/transform.
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
