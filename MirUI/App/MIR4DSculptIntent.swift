import CoreGraphics
import MirUIHandGesture

/// Rich, typed payload for a sculpting stroke.
///
/// Decouples hand / air-sculpt recognition from the CAD deformation command:
/// producers fill the geometry, consumers (MirEngine deformation) interpret `mode`.
///
/// The canonical intent bus for the whole application is `MIRIntentRouter`
/// (module `MirUIHandGesture`); a sculpt stroke is published on it as
/// `MIRIntent(source: .spatial, action: "sculpt", ...)` and the rich payload
/// below is built from the raw `MIRHandIntent` at the App layer.
struct MIR4DSculptIntent: Equatable {
    /// Deformation mode requested by the user.
    enum Mode: String, Equatable, Sendable, CaseIterable {
        case push
        case pull
        case smooth
        case inflate
        case grab
        case pinch
        case cut
        case paint
    }

    var position: CGPoint
    /// Scene-space depth of the stroke, in the hand interaction volume.
    /// Carries the real relative depth from the camera (Vision `z`, projected
    /// by `MIRHandSpatialMapper`) — not an estimate. 0 at the depth centre.
    var depth: CGFloat
    /// Surface-normal direction in screen space (unit-ish).
    var direction: CGPoint
    /// Applied force, normalised to 0...1.
    var pressure: CGFloat
    /// Brush radius in scene units.
    var radius: CGFloat
    /// Deformation magnitude.
    var strength: CGFloat
    /// Stroke speed.
    var velocity: CGFloat
    var mode: Mode

    init(
        position: CGPoint,
        depth: CGFloat = 0,
        direction: CGPoint = .zero,
        pressure: CGFloat = 0,
        radius: CGFloat = 1,
        strength: CGFloat = 0,
        velocity: CGFloat = 0,
        mode: Mode = .push
    ) {
        self.position = position
        self.depth = depth
        self.direction = direction
        self.pressure = pressure
        self.radius = radius
        self.strength = strength
        self.velocity = velocity
        self.mode = mode
    }

    /// Build from the canonical raw hand intent. `grab` maps to grab mode,
    /// everything else to a push stroke; geometry fields default sensibly.
    init(from intent: MIRHandIntent) {
        let mode: Mode = intent.gesture.type == .grab ? .grab : .push
        let pressure = min(max(intent.strength, 0), 1)
        self.init(
            position: CGPoint(x: intent.position.x, y: intent.position.y),
            depth: CGFloat(intent.position.z),
            direction: CGPoint(x: intent.direction.x, y: intent.direction.y),
            pressure: pressure,
            strength: pressure,
            mode: mode
        )
    }
}
