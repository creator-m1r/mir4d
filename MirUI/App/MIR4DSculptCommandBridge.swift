import Foundation
import simd

/// Converts sculpting intents into MirEngine sculpt commands.
@MainActor
final class MIR4DSculptCommandBridge {
    enum BridgeError: Error {
        case viewportUnavailable
        case selectedObjectUnavailable
        case invalidBounds
    }

    private let runtime: MIR4DAppRuntime

    init(runtime: MIR4DAppRuntime) {
        self.runtime = runtime
    }

    func apply(_ intent: MIR4DSculptIntent) {
        guard let viewport = runtime.viewport else { return }

        let point = intent.point
        let normal = intent.normal

        MirEngineApplySculpt(
            viewport,
            modeInt(intent.mode),
            point.x,
            point.y,
            point.z,
            normal.x,
            normal.y,
            normal.z,
            intent.radius,
            intent.strength
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

        // CChar is platform-dependent (Int8 on Apple platforms), while
        // String(decoding:as:) expects UTF-8 code units. Convert explicitly
        // and stop at the first C null terminator.
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
