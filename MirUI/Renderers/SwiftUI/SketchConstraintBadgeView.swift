import SwiftUI

/// Compact visual badges for automatically detected sketch constraints.
struct SketchConstraintBadgeView: View {
    let constraint: SketchConstraintState.DetectedConstraint

    var body: some View {
        HStack(spacing: 6) {
            Text(constraint.symbol)
                .font(.system(size: 12, weight: .bold, design: .rounded))
            Text(constraint.title)
                .font(.system(size: 11, weight: .medium))
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 5)
        .background(.thinMaterial, in: Capsule())
    }
}
