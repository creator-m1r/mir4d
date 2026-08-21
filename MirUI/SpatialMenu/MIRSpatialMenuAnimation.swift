import SwiftUI

/// Animation palette and highlight styles for the spatial fan.
/// Transitions are smooth and continuous; there is no harsh colour switching.
enum MIRSpatialMenuAnimation {
    static let appear = Animation.easeOut(duration: 0.22)
    static let disappear = Animation.easeIn(duration: 0.16)
    static let ringReveal = Animation.interactiveSpring(response: 0.26, dampingFraction: 0.80)
    static let highlight = Animation.interactiveSpring(response: 0.18, dampingFraction: 0.72)
    static let haloBreath = Animation.easeInOut(duration: 1.6).repeatForever(autoreverses: true)
    static let blurTransition = Animation.easeInOut(duration: 0.24)

    /// Scale of a segment by highlight state: hover grows, selected grows further,
    /// disabled stays quiet.
    static func scale(for state: MIRSpatialMenuHighlightState) -> CGFloat {
        switch state {
        case .idle: return 1.0
        case .hover: return 1.06
        case .selected: return 1.14
        case .active: return 1.20
        case .disabled: return 0.96
        }
    }

    /// Stroke width of a segment by highlight state.
    static func strokeWidth(for state: MIRSpatialMenuHighlightState) -> CGFloat {
        switch state {
        case .idle: return 1.0
        case .hover: return 2.0
        case .selected: return 2.6
        case .active: return 3.0
        case .disabled: return 0.75
        }
    }

    /// Opacity of a segment by highlight state.
    static func opacity(for state: MIRSpatialMenuHighlightState) -> Double {
        switch state {
        case .idle: return 0.55
        case .hover: return 0.82
        case .selected: return 1.0
        case .active: return 1.0
        case .disabled: return 0.24
        }
    }

    /// Glow (shadow blur radius) of a selected segment.
    static func glow(for state: MIRSpatialMenuHighlightState) -> CGFloat {
        switch state {
        case .selected: return 18
        case .active: return 22
        default: return 0
        }
    }
}

/// Soft halo behind the selected segment: a quiet breathing glow.
struct MIRSpatialMenuHalo: View {
    let color: Color
    let active: Bool

    var body: some View {
        Circle()
            .fill(color.opacity(active ? 0.20 : 0.07))
            .frame(width: 54, height: 54)
            .blur(radius: 6)
            .scaleEffect(active ? 1.0 : 0.86)
            .animation(MIRSpatialMenuAnimation.highlight, value: active)
    }
}

/// Short transition when the selection moves to another segment: the glow
/// travels instead of popping. Applied as a ViewModifier on the fan segment.
struct MIRSpatialMenuSegmentTransition: ViewModifier {
    let isHighlighted: Bool

    func body(content: Content) -> some View {
        content
            .scaleEffect(isHighlighted ? 1.1 : 1.0)
            .shadow(
                color: MirTheme.Colors.accentBright.opacity(isHighlighted ? 0.55 : 0),
                radius: isHighlighted ? 12 : 0
            )
            .animation(MIRSpatialMenuAnimation.highlight, value: isHighlighted)
    }
}