import SwiftUI

struct MIR4DSpatialWindow<Content: View>: View {
    let title: String
    let subtitle: String?
    let isFocused: Bool
    let onClose: () -> Void
    @ViewBuilder let content: () -> Content

    var body: some View {
        VStack(spacing: 0) {
            HStack(spacing: 10) {
                Circle()
                    .fill(isFocused ? Color.cyan : Color.white.opacity(0.25))
                    .frame(width: 7, height: 7)
                    .shadow(color: isFocused ? .cyan.opacity(0.65) : .clear, radius: 5)

                VStack(alignment: .leading, spacing: 2) {
                    Text(title)
                        .font(.system(size: 13, weight: .semibold, design: .rounded))
                        .foregroundStyle(.white)
                    if let subtitle {
                        Text(subtitle)
                            .font(.system(size: 9, weight: .medium))
                            .foregroundStyle(.white.opacity(0.42))
                    }
                }

                Spacer()

                Button(action: onClose) {
                    Image(systemName: "xmark")
                        .font(.system(size: 9, weight: .bold))
                        .foregroundStyle(.white.opacity(0.62))
                        .frame(width: 28, height: 28)
                        .background(Color.white.opacity(0.06), in: Circle())
                }
                .buttonStyle(.plain)
            }
            .padding(.horizontal, 15)
            .padding(.vertical, 11)

            Divider().overlay(Color.white.opacity(0.07))
            content()
        }
        .background(.ultraThinMaterial)
        .background(Color(red: 0.015, green: 0.024, blue: 0.038).opacity(0.92))
        .clipShape(RoundedRectangle(cornerRadius: 17, style: .continuous))
        .overlay {
            RoundedRectangle(cornerRadius: 17, style: .continuous)
                .stroke(isFocused ? Color.cyan.opacity(0.25) : Color.white.opacity(0.09), lineWidth: 1)
        }
        .shadow(color: .black.opacity(isFocused ? 0.48 : 0.32), radius: isFocused ? 32 : 22, y: isFocused ? 16 : 10)
        .scaleEffect(isFocused ? 1.0 : 0.985)
        .animation(.spring(response: 0.25, dampingFraction: 0.85), value: isFocused)
    }
}

struct MIR4DSpatialWindowBackground: View {
    var body: some View {
        ZStack {
            Color.black
            RadialGradient(
                colors: [Color.cyan.opacity(0.055), .clear],
                center: .topLeading,
                startRadius: 20,
                endRadius: 520
            )
            RadialGradient(
                colors: [Color.blue.opacity(0.04), .clear],
                center: .bottomTrailing,
                startRadius: 30,
                endRadius: 560
            )
        }
        .ignoresSafeArea()
    }
}
