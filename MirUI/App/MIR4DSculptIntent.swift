import CoreGraphics
import MirUIHandGesture

struct MIR4DSculptIntent: Equatable {

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

    var depth: CGFloat

    var direction: CGPoint

    var pressure: CGFloat

    var radius: CGFloat

    var strength: CGFloat

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

extension MIR4DSculptIntent.Mode {

    init?(handPose: MIRHandGestureType) {
        switch handPose {
        case .pinch: self = .push
        case .fist: self = .pull
        case .point: self = .smooth
        case .openPalm: self = .inflate
        case .grab: self = .grab
        case .thumbsUp: self = .pinch
        case .vSign: self = .cut
        case .twoFinger: self = .paint
        default: return nil
        }
    }

    var shortTitle: String {
        switch self {
        case .push: return "Вдавить"
        case .pull: return "Вытянуть"
        case .smooth: return "Сгладить"
        case .inflate: return "Надуть"
        case .grab: return "Тянуть"
        case .pinch: return "Щипок"
        case .cut: return "Разрез"
        case .paint: return "Краска"
        }
    }
}
