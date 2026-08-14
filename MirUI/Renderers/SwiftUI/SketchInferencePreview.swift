import SwiftUI

struct SketchInferencePreview: Identifiable, Equatable {
    enum Kind: Equatable {
        case coincident
        case horizontal
        case vertical
        case perpendicular
        case midpoint
    }

    let id = UUID()
    let kind: Kind
    let position: CGPoint
    let confidence: Double

    var symbol: String {
        switch kind {
        case .coincident: return "●"
        case .horizontal: return "H"
        case .vertical: return "V"
        case .perpendicular: return "⊥"
        case .midpoint: return "M"
        }
    }

    var title: String {
        switch kind {
        case .coincident: return "Coincident"
        case .horizontal: return "Horizontal"
        case .vertical: return "Vertical"
        case .perpendicular: return "Perpendicular"
        case .midpoint: return "Midpoint"
        }
    }
}

struct SketchInferencePreviewView: View {
    let preview: SketchInferencePreview

    var body: some View {
        HStack(spacing: 6) {
            Text(preview.symbol)
                .font(.system(size: 13, weight: .bold, design: .monospaced))
            Text(preview.title)
                .font(.system(size: 12, weight: .medium))
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 5)
        .background(.ultraThinMaterial, in: Capsule())
        .overlay(Capsule().stroke(.white.opacity(0.25), lineWidth: 1))
        .position(preview.position)
        .opacity(max(0.25, min(1.0, preview.confidence)))
        .allowsHitTesting(false)
    }
}

struct SketchInferencePreviewLayer: View {
    let previews: [SketchInferencePreview]

    var body: some View {
        ZStack {
            ForEach(previews) { preview in
                SketchInferencePreviewView(preview: preview)
            }
        }
        .allowsHitTesting(false)
    }
}
