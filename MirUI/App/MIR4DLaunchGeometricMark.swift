import SwiftUI

struct MIR4DLaunchGeometricMark: View {
    var body: some View {
        HStack(spacing: 0) {
            diamond(offset: -7)
                .foregroundStyle(Color(red: 0.35, green: 0.62, blue: 0.98))
            diamond(offset: 0)
                .foregroundStyle(Color(red: 0.45, green: 0.82, blue: 0.66))
            diamond(offset: 7)
                .foregroundStyle(Color(red: 0.96, green: 0.63, blue: 0.28))
            diamond(offset: 14)
                .foregroundStyle(Color(red: 0.82, green: 0.46, blue: 0.94))
        }
        .frame(width: 120, height: 40)
    }

    private func diamond(offset: CGFloat) -> some View {
        RoundedRectangle(cornerRadius: 3)
            .rotationEffect(.degrees(45))
            .frame(width: 22, height: 22)
            .offset(y: offset)
            .opacity(0.92)
    }
}