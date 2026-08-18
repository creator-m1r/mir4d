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
@MainActor
final class MIR4DSculptCommandBridge {
    static let shared = MIR4DSculptCommandBridge()

    private var cancellable: AnyCancellable?
    private let runtime = MIR4DModelRuntime.shared

    // Must match `MIRHandSpatialMapper.Volume` defaults.
    private let volumeW = 2.0
    private let volumeH = 1.25
    private let volumeD = 1.6
    private let depthCenter = 0.0
    /// Fraction of the object half-extent used as the brush radius so a default
    /// stroke stays local rather than engulfing the whole body.
    private let brushScale: Double = 0.25

    /// A hand sculpt stroke spans many deform frames; they collapse into exactly
    /// one undo entry via begin/end of a stroke. The stroke ends after a short
    /// idle (gap between pinch frames) so releasing the gesture commits the
    /// undoable `DeformObjectCommand`.
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
        // Any incoming frame keeps the stroke alive; commit only after a gap.
        endTimer?.invalidate()
        endTimer = Timer.scheduledTimer(withTimeInterval: strokeIdleSeconds, repeats: false) { _ in
            MainActor.assumeIsolated { self?.finishStroke() }
        }

        guard let bounds = selectedBounds() else { return }
        let bmin = bounds.min
        let bmax = bounds.max
        let center = (bmin + bmax) * 0.5
        let half = (bmax - bmin) * 0.5
        let maxHalf = max(half.x, max(half.y, half.z))

        // Interaction volume (−1…1) → object extents.
        let nx = Double(intent.position.x) / (volumeW * 0.5)
        let ny = Double(intent.position.y) / (volumeH * 0.5)
        let nz = (Double(intent.depth) - depthCenter) / (volumeD * 0.5)
        let wx = center.x + nx * half.x
        let wy = center.y + ny * half.y
        let wz = center.z + nz * half.z

        let worldRadius = max(Double(intent.radius), 0.05) * maxHalf * brushScale
        // Displacement scales with object size and pinch strength (0…1).
        let worldStrength = Double(intent.strength) * maxHalf * 0.04

        _ = runtime.deformSelected(
            x: wx, y: wy, z: wz,
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
    }

    /// Maps `MIR4DSculptIntent.Mode` to the int profile expected by MirEngine.
    private func modeInt(_ mode: MIR4DSculptIntent.Mode) -> Int {
        switch mode {
        case .push:   0
        case .pull:   1
        case .smooth: 2
        case .inflate: 3
        case .grab:   4
        case .pinch:  5
        case .cut:    6
        case .paint:  7
        }
    }

    private func selectedBounds() -> (min: SIMD3<Double>, max: SIMD3<Double>)? {
        guard let viewport = runtime.viewport else { return nil }
        var buffer = [CChar](repeating: 0, count: 1024)
        guard MirEngineGetSelectedObjectMetrics(viewport, &buffer, buffer.count) else { return nil }
        guard let data = String(cString: buffer).data(using: .utf8),
              let dict = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
              let bmin = dict["boundsMin"] as? [String: Double],
              let bmax = dict["boundsMax"] as? [String: Double],
              let x0 = bmin["x"], let y0 = bmin["y"], let z0 = bmin["z"],
              let x1 = bmax["x"], let y1 = bmax["y"], let z1 = bmax["z"]
        else { return nil }
        return (SIMD3(x0, y0, z0), SIMD3(x1, y1, z1))
    }
}
