import SwiftUI

struct SketchInfiniteGridView: View {
    let zoom: CGFloat
    let pan: CGSize

    private var minorStep: CGFloat {
        let base: CGFloat = 24
        let candidates: [CGFloat] = [base, base * 2, base * 5, base * 10]
        return candidates.first { $0 * zoom >= 12 } ?? base * 10
    }

    var body: some View {
        Canvas { context, size in
            let step = minorStep * zoom
            guard step > 2 else { return }

            let offsetX = pan.width.truncatingRemainder(dividingBy: step)
            let offsetY = pan.height.truncatingRemainder(dividingBy: step)

            stride(from: offsetX, through: size.width, by: step).forEach { x in
                var path = Path()
                path.move(to: CGPoint(x: x, y: 0))
                path.addLine(to: CGPoint(x: x, y: size.height))
                context.stroke(path, with: .color(.gray.opacity(0.10)), lineWidth: 1)
            }

            stride(from: offsetY, through: size.height, by: step).forEach { y in
                var path = Path()
                path.move(to: CGPoint(x: 0, y: y))
                path.addLine(to: CGPoint(x: size.width, y: y))
                context.stroke(path, with: .color(.gray.opacity(0.10)), lineWidth: 1)
            }

            let majorStep = step * 5
            let majorX = pan.width.truncatingRemainder(dividingBy: majorStep)
            let majorY = pan.height.truncatingRemainder(dividingBy: majorStep)

            stride(from: majorX, through: size.width, by: majorStep).forEach { x in
                var path = Path()
                path.move(to: CGPoint(x: x, y: 0))
                path.addLine(to: CGPoint(x: x, y: size.height))
                context.stroke(path, with: .color(.gray.opacity(0.20)), lineWidth: 1)
            }

            stride(from: majorY, through: size.height, by: majorStep).forEach { y in
                var path = Path()
                path.move(to: CGPoint(x: 0, y: y))
                path.addLine(to: CGPoint(x: size.width, y: y))
                context.stroke(path, with: .color(.gray.opacity(0.20)), lineWidth: 1)
            }
        }
        .allowsHitTesting(false)
    }
}
