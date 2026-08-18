import Foundation
import simd

/// Converts sculpting intents into MirEngine deformation commands.
///
/// The bridge deliberately uses the existing `MIR4DModelRuntime` as the
/// application runtime and resolves the hand position through the viewport
/// picking API. This keeps the sculpt intent independent from the C ABI
/// and avoids duplicating a second runtime/viewport ownership model.
@MainActor
final class MIR4DSculptCommandBridge {
    static let shared = MIR4DSculptCommandBridge()

    enum BridgeError: Error {
        case viewportUnavailable
        case selectedObjectUnavailable
        case invalidBounds
        case surfaceNotFound
    }

    private let runtime: MIR4DModelRuntime

    init(runtime: MIR4DModelRuntime = .shared) {
        self.runtime = runtime
    }

    /// Applies one sculpt intent to the selected surface.
    /// `MIR4DSculptIntent.position` is expected to be normalized screen space.
    @discardableResult
    func apply(_ intent: MIR4DSculptIntent) -> Bool {
        guard runtime.viewport != nil else { return false }

        let nx = Double(intent.position.x) * 2.0 - 1.0
        let ny = 1.0 - Double(intent.position.y) * 2.0

        guard let hit = runtime.pickWorldPoint(nx: nx, ny: ny) else {
            return false
        }

        let radius = max(0.001, Double(intent.radius))
        let strength = Double(intent.strength)
        let mode = modeInt(intent.mode)

        return runtime.deformSelected(
            x: hit.point.x,
            y: hit.point.y,
            z: hit.point.z,
            radius: radius,
            strength: strength,
            mode: mode
        )
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
        guard MirEngineGetSelectedObjectMetrics(viewport, &buffer, buffer.count) else {
            return nil
        }

        let bytes: [UInt8] = buffer.map { UInt8(bitPattern: $0) }
        let utf8Bytes = bytes.prefix { $0 != 0 }
        let json = String(decoding: utf8Bytes, as: UTF8.self)

        guard let data = json.data(using: String.Encoding.utf8),
              let dict = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
              let bmin = dict["boundsMin"] as? [String: Double],
              let bmax = dict["boundsMax"] as? [String: Double],
              let x0 = bmin["x"], let y0 = bmin["y"], let z0 = bmin["z"],
              let x1 = bmax["x"], let y1 = bmax["y"], let z1 = bmax["z"]
        else {
            return nil
        }

        return (
            SIMD3<Double>(x0, y0, z0),
            SIMD3<Double>(x1, y1, z1)
        )
    }
}
