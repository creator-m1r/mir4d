import Foundation
import simd

/// Structured debug snapshot for development overlays.
/// Never surfaced in the production interface.
struct MIRHandGestureDebugInfo: Sendable {
    struct HandEntry: Sendable {
        let handedness: Handedness
        let gesture: MIRHandGestureType
        let confidence: Double
        let pinch: Double
        let speed: Double
        let direction: SIMD3<Double>
        let position: SIMD3<Double>
        /// Normalised (0...1) landmark positions for skeleton rendering.
        let landmarkPositions: [SIMD3<Double>]
    }

    struct TwoHandEntry: Sendable {
        let center: SIMD3<Double>
        let distance: Double
        let gesture: MIRHandGestureType
    }

    let hands: [HandEntry]
    let twoHand: TwoHandEntry?
    let spatialContext: MIRHandSpatialContext

    init(hands: [HandEntry] = [], twoHand: TwoHandEntry? = nil, spatialContext: MIRHandSpatialContext) {
        self.hands = hands
        self.twoHand = twoHand
        self.spatialContext = spatialContext
    }
}

#if DEBUG
import SwiftUI

/// Development-only overlay. It renders the hand skeleton and live recognition
/// telemetry. It must never appear in the production interface.
@MainActor
struct MIRHandGestureDebugOverlay: View {
    let info: MIRHandGestureDebugInfo

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            ForEach(Array(info.hands.enumerated()), id: \.offset) { _, hand in
                Text("\(hand.handedness.rawValue) \(hand.gesture.rawValue)")
                    .font(.system(size: 12, weight: .bold, design: .monospaced))
                Text("confidence: \(hand.confidence, specifier: "%.2f")")
                Text("pinch: \(hand.pinch, specifier: "%.2f")")
                Text("speed: \(hand.speed, specifier: "%.2f")")
                let p = hand.position
                Text("x: \(p.x, specifier: "%.2f") y: \(p.y, specifier: "%.2f") z: \(p.z, specifier: "%.2f")")
            }
            if let two = info.twoHand {
                Text("TWO-HAND \(two.gesture.rawValue) d: \(two.distance, specifier: "%.2f")")
            }
        }
        .padding(8)
        .background(.black.opacity(0.55))
        .foregroundStyle(.green)
        .cornerRadius(8)
    }
}
#endif

