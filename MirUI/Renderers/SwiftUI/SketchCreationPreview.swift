import CoreGraphics

struct SketchCreationPreview {
    enum Shape {
        case line(start: CGPoint, end: CGPoint)
        case circle(center: CGPoint, radius: CGFloat)
        case arc(center: CGPoint, start: CGPoint, end: CGPoint)
    }

    let shape: Shape
    let snap: SketchSnapPreview?
    let inferences: [SketchInferencePreview]
}

struct SketchSnapPreview {
    let point: CGPoint
    let kind: String
    let geometryID: UInt32
}
