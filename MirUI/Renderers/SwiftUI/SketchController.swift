import Foundation

/// Контроллер эскиза: единственная точка вызова универсального решателя
/// ограничений MirEngine из SwiftUI. Зеркален WorkPlaneController.
///
/// ТЗ Этап 2: профессиональный эскиз. Эскиз строится как набор примитивов
/// плюс ограничения и решается ядром MirEngine (SketchDocumentSolver),
/// без дублирования геометрии в Swift.
@MainActor
final class SketchController {
    static let shared = SketchController()

    private init() {}

    /// Строит прямоугольник заданного размера через универсальный решатель
    /// ограничений и возвращает 4 угла в локальных координатах плоскости
    /// (порядок: левый-низ, правый-низ, правый-верх, левый-верх).
    func createSolvedRectangle(width: Float, height: Float) -> [CGPoint]? {
        guard let doc = MirEngineSketchCreateDocument() else { return nil }
        defer { MirEngineSketchDestroyDocument(doc) }

        let w = width
        let h = height

        let l1 = MirEngineSketchAddLine(doc, 0, 0, w, 0) // низ
        let l2 = MirEngineSketchAddLine(doc, w, 0, w, h) // право
        let l3 = MirEngineSketchAddLine(doc, w, h, 0, h) // верх
        let l4 = MirEngineSketchAddLine(doc, 0, h, 0, 0) // лево

        let c = MirEngineSketchConstraint.self
        MirEngineSketchAddConstraint(doc, Int32(c.horizontal.rawValue), l1, 0, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.vertical.rawValue), l2, 0, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.horizontal.rawValue), l3, 0, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.vertical.rawValue), l4, 0, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.equal.rawValue), l1, l2, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.equal.rawValue), l1, l3, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.equal.rawValue), l1, l4, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.perpendicular.rawValue), l1, l2, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.coincident.rawValue), l1, l2, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.coincident.rawValue), l2, l3, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.coincident.rawValue), l3, l4, 0)
        MirEngineSketchAddConstraint(doc, Int32(c.coincident.rawValue), l4, l1, 0)

        guard MirEngineSketchSolve(doc) else { return nil }

        func readLine(_ id: UInt32) -> (CGPoint, CGPoint)? {
            var x1: Float = 0, y1: Float = 0, x2: Float = 0, y2: Float = 0
            guard MirEngineSketchGetLine(doc, id, &x1, &y1, &x2, &y2) else { return nil }
            return (CGPoint(x: CGFloat(x1), y: CGFloat(y1)),
                    CGPoint(x: CGFloat(x2), y: CGFloat(y2)))
        }

        guard let a = readLine(l1), let b = readLine(l2),
              let cc = readLine(l3), let d = readLine(l4) else { return nil }

        // Углы: BL, BR, TR, TL
        return [a.0, a.1, b.1, cc.1]
    }
}
