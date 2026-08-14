import CoreGraphics

struct SketchLineHit {
    let geometryID: UInt32
    let distance: CGFloat
}

struct SketchGeometryHitTester {
    var tolerance: CGFloat = 8

    func nearestLine(
        to point: CGPoint,
        lines: [(id: UInt32, start: CGPoint, end: CGPoint)]
    ) -> SketchLineHit? {
        var best: SketchLineHit?

        for line in lines {
            let distance = pointToSegmentDistance(point, line.start, line.end)
            guard distance <= tolerance else { continue }

            if best == nil || distance < best!.distance {
                best = SketchLineHit(geometryID: line.id, distance: distance)
            }
        }

        return best
    }

    private func pointToSegmentDistance(
        _ point: CGPoint,
        _ a: CGPoint,
        _ b: CGPoint
    ) -> CGFloat {
        let ab = CGPoint(x: b.x - a.x, y: b.y - a.y)
        let ap = CGPoint(x: point.x - a.x, y: point.y - a.y)
        let lengthSquared = ab.x * ab.x + ab.y * ab.y

        if lengthSquared <= 0.000001 {
            return hypot(point.x - a.x, point.y - a.y)
        }

        let t = max(0, min(1, (ap.x * ab.x + ap.y * ab.y) / lengthSquared))
        let closest = CGPoint(
            x: a.x + ab.x * t,
            y: a.y + ab.y * t
        )

        return hypot(point.x - closest.x, point.y - closest.y)
    }
}
