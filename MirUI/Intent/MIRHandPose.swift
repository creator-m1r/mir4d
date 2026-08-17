import Foundation
import CoreGraphics

/// Camera-independent hand pose consumed by the air-interaction layer.
struct MIRHandPoint: Equatable, Sendable {
    var x: CGFloat
    var y: CGFloat
    var confidence: Double
}

struct MIRHandPose: Equatable, Sendable {
    var wrist: MIRHandPoint
    var indexTip: MIRHandPoint
    var thumbTip: MIRHandPoint
    var middleTip: MIRHandPoint
    var pinchDistance: CGFloat
    var openness: CGFloat
    var depth: CGFloat
    var timestamp: Date

    init(
        wrist: MIRHandPoint,
        indexTip: MIRHandPoint,
        thumbTip: MIRHandPoint,
        middleTip: MIRHandPoint,
        pinchDistance: CGFloat,
        openness: CGFloat,
        depth: CGFloat = 0,
        timestamp: Date = Date()
    ) {
        self.wrist = wrist
        self.indexTip = indexTip
        self.thumbTip = thumbTip
        self.middleTip = middleTip
        self.pinchDistance = pinchDistance
        self.openness = openness
        self.depth = depth
        self.timestamp = timestamp
    }
}
