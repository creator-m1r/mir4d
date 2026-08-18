import Foundation
import Combine
import CoreGraphics
import simd
import MirUIHandGesture

/// Bridges the rich `MIR4DSculptIntent` stream into a real, in-place mesh
/// deformation of the selected object through MirEngine.
///
/// The hand module works in a normalised interaction volume; the CAD object
/// lives in world space. This bridge projects the brush centre / radius from the
/// interaction volume onto the selected object's bounding box so the stroke lands
/// on the object proportionally, then calls `MirEngineDeformSelected`.
///
/// Hand sculpt strokes are grouped into one undoable operation.
@MainActor
final class MIR4DSculptCommandBridge {
    static let shared = MIR4DSculptCommandBridge()

    private var cancellable: AnyCancellable?
    private let runtime = MIR4DModelRuntime.shared

    private let volumeW = 2.0
    private let volumeH = 1.25
    private let volumeD = 1.6
    private let depthCenter = 0.0

    private let strokeIdleSeconds: TimeInterval = 0.25
    private var strokeActive = false
    private var endTimer: Timer?

    private init() {
        cancellable = MIR4DSculptIntentPublisher.shared.stream
            .sink { [weak self] intent in
                self?.apply(intent)
            }
    }

    private func apply(_ intent: MIR4DSculptIntent) {
        if !strokeActive {
            runtime.beginDeformStroke()
            strokeActive = true
        }

        endTimer?.invalidate()
        endTimer = Timer.scheduledTimer(withTimeInterval: strokeIdleSeconds, repeats: false) { [weak self] _ in
            MainActor.assumeIsolated { self?.finishStroke() }
        }

        guard let bounds = selectedBounds() else {
            MIR4DBrushIndicator.shared.hide()
            return
        }

        let bmin = bounds.min
        let bmax = bounds.max
        let center = (bmin + bmax) * 0.5
        let half = (bmax - bmin) * 0.5
        let maxHalf = max(half.x, max(half.y, half.z))

        let nx = Double(intent.position.x) / (volumeW * 0.5)
        let ny = Double(intent.position.y) / (volumeH * 0.5)

        let (wx, wy, wz): (Double, Double, Double)
        if let hit = runtime.pickWorldPoint(nx: nx, ny: ny) {
            wx = hit.point.x
            wy = hit.point.y
            wz = hit.point.z
        } else {
            let nz = (Double(intent.depth) - depthCenter) / (volumeD * 0.5)
            wx = center.x + nx * half.x
            wy = center.y + ny * half.y
            wz = center.z + nz * half.z
        }

        let worldRadius = max(Double(intent.radius), 0.05) * maxHalf * MIR4DSculptSettings.shared.radiusScale
        let worldStrength = Double(intent.strength) * maxHalf * MIR4DSculptSettings.shared.strengthScale

        MIR4DBrushIndicator.shared.show(
            x: nx,
            y: ny,
            radius: CGFloat(max(Double(intent.radius), 0.05) * MIR4DSculptSettings.shared.radiusScale),
            mode: intent.mode
        )

        _ = runtime.deformSelected(
            x: wx,
            y: wy,
            z: wz,
            radius: worldRadius,
            strength: worldStrength,
            mode: modeInt(intent.mode)
        )
    }

    private func finishStroke() {
        guard strokeActive else { return }
        endTimer?.invalidate()
        endTimer = nil
        runtime.endDeformStroke()
        strokeActive = false
        MIR4DBrushIndicator.shared.hide()
    }

    private func modeInt(_ mode: MIR4DSculptIntent.Mode) -> Int {
        switch mode {
        case .push: 0
        case .pull: 1
        case .smooth: 2
        case .inflate: 3
        case .grab: 4
        case .pinch: 5
        case .cut: 6
        case .paint: 7
        }
    }

    private func selectedBounds() -> (min: SIMD3<Double>, max: SIMD3<Double>)? {
        guard let viewport = runtime.viewport else { return nil }
        var buffer = [CChar](repeating: 0, count: 1024)
        guard MirEngineGetSelectedObjectMetrics(viewport, &buffer, buffer.count) else { return nil }

        let json = String(decoding: buffer, as: UTF8.self)
        guard let data = json.data(using: .utf8),
              let dict = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
              let bmin = dict["boundsMin"] as? [String: Double],
              let bmax = dict["boundsMax"] as? [String: Double],
              let x0 = bmin["x"], let y0 = bmin["y"], let z0 = bmin["z"],
              let x1 = bmax["x"], let y1 = bmax["y"], let z1 = bmax["z"]
        else { return nil }

        return (SIMD3(x0, y0, z0), SIMD3(x1, y1, z1))
    }
}
