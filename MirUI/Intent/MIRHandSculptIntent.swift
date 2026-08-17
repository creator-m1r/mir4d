import Foundation
import CoreGraphics

struct MIRSculptSample: Equatable, Sendable {
    let position: CGPoint
    let pressure: CGFloat
    let radius: CGFloat
    let timestamp: Date
}

enum MIRHandGesture: String, Equatable, Sendable {
    case point
    case pinch
    case grab
    case openPalm
    case unknown
}

struct MIRHandSculptInterpreter {
    var pinchThreshold: CGFloat = 0.075
    var grabThreshold: CGFloat = 0.19

    func gesture(for pose: MIRHandPose) -> MIRHandGesture {
        if pose.pinchDistance <= pinchThreshold { return .pinch }
        if pose.openness <= grabThreshold { return .grab }
        if pose.openness > grabThreshold * 1.7 { return .openPalm }
        return .point
    }

    func sample(for pose: MIRHandPose) -> MIRSculptSample {
        let gesture = gesture(for: pose)
        let pressure: CGFloat
        switch gesture {
        case .pinch: pressure = 1.0
        case .grab: pressure = 0.72
        case .openPalm: pressure = 0.12
        case .point: pressure = 0.45
        case .unknown: pressure = 0
        }
        let radius = 0.012 + pressure * 0.035
        return MIRSculptSample(position: pose.indexTip.location, pressure: pressure, radius: radius, timestamp: pose.timestamp)
    }
}

private extension MIRHandPoint {
    var location: CGPoint { CGPoint(x: x, y: y) }
}
