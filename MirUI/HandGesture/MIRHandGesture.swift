import Foundation
import simd

/// Recognised hand gestures. The set is open for extension; consumers should
/// treat unknown values gracefully via the raw `rawValue`.
public enum MIRHandGestureType: String, Sendable, CaseIterable {
    // Single-hand
    case openPalm = "OPEN_PALM"
    case point = "POINT"
    case pinch = "PINCH"
    case grab = "GRAB"
    case fist = "FIST"
    case twoFinger = "TWO_FINGER"
    case threeFinger = "THREE_FINGER"
    case vSign = "V_SIGN"
    case thumbsUp = "THUMBS_UP"
    case rest = "REST"

    // Two-hand
    case twoHandTranslate = "TWO_HAND_TRANSLATE"
    case twoHandScale = "TWO_HAND_SCALE"
    case twoHandRotate = "TWO_HAND_ROTATE"
    case twoHandPinch = "TWO_HAND_PINCH"
    case twoHandGrab = "TWO_HAND_GRAB"

    var isTwoHanded: Bool {
        rawValue.hasPrefix("TWO_HAND_")
    }
}

/// Gesture lifecycle phase. Required for continuous interactions such as grab,
/// sculpt, drag and draw.
public enum MIRHandGesturePhase: String, Sendable {
    case began
    case changed
    case ended
    case cancelled
}

/// A single sampled point of a continuous motion (e.g. a drawn trajectory).
struct MIRHandMotionSample: Sendable {
    let position: SIMD3<Double>
    let direction: SIMD3<Double>
    let velocity: SIMD3<Double>
    let timestamp: Date
}

/// A recognised gesture with its spatial context at the moment of recognition.
public struct MIRHandGesture: Sendable {
    public let type: MIRHandGestureType
    public let confidence: Double
    public let position: SIMD3<Double>
    public let direction: SIMD3<Double>
    public let velocity: SIMD3<Double>
    public let strength: Double
    public let timestamp: Date

    init(
        type: MIRHandGestureType,
        confidence: Double,
        position: SIMD3<Double>,
        direction: SIMD3<Double> = .zero,
        velocity: SIMD3<Double> = .zero,
        strength: Double = 0,
        timestamp: Date = Date()
    ) {
        self.type = type
        self.confidence = min(max(confidence, 0), 1)
        self.position = position
        self.direction = direction
        self.velocity = velocity
        self.strength = strength
        self.timestamp = timestamp
    }
}

/// Both hands observed simultaneously, with derived relational properties.
struct MIRHandPair: Sendable {
    let left: MIRHandPose?
    let right: MIRHandPose?

    let center: SIMD3<Double>
    let distance: Double
    let direction: SIMD3<Double>
    let relativeVelocity: SIMD3<Double>
    let rotation: Double

    init(left: MIRHandPose?, right: MIRHandPose?, timestamp: Date = Date()) {
        self.left = left
        self.right = right

        let a = left?.palmPosition
        let b = right?.palmPosition
        if let a, let b {
            let delta = b - a
            self.center = (a + b) * 0.5
            self.distance = simd_length(delta)
            self.direction = simd_normalize(delta)
            self.relativeVelocity = (right?.velocityHint ?? .zero) - (left?.velocityHint ?? .zero)
            self.rotation = atan2(delta.y, delta.x)
        } else if let a {
            self.center = a
            self.distance = 0
            self.direction = .zero
            self.relativeVelocity = .zero
            self.rotation = 0
        } else if let b {
            self.center = b
            self.distance = 0
            self.direction = .zero
            self.relativeVelocity = .zero
            self.rotation = 0
        } else {
            self.center = .zero
            self.distance = 0
            self.direction = .zero
            self.relativeVelocity = .zero
            self.rotation = 0
        }
    }
}

private extension MIRHandPose {
    /// Best-effort instantaneous velocity used for two-hand relational math.
    /// The authoritative velocity lives in `MIRHandMotion`; this is only a hint.
    var velocityHint: SIMD3<Double>? { nil }
}
