import SwiftUI

struct MIR4DProjectWindowShell<Content: View>: View {
    let title: String
    let subtitle: String?
    let onClose: () -> Void
    @ViewBuilder let content: () -> Content

    var body: some View {
        ZStack {
            Color.black.opacity(0.42)
                .ignoresSafeArea()
                .transition(.opacity)

            VStack(spacing: 0) {
                HStack(spacing: 12) {
                    VStack(alignment: .leading, spacing: 3) {
                        Text(title)
                            .font(.system(size: 17, weight: .semibold, design: .rounded))
                            .foregroundStyle(.white)
                        if let subtitle {
                            Text(subtitle)
                                .font(.system(size: 10, weight: .medium))
                                .foregroundStyle(.white.opacity(0.45))
                        }
                    }

                    Spacer()

                    Button(action: onClose) {
                        Image(systemName: "xmark")
                            .font(.system(size: 11, weight: .bold))
                            .foregroundStyle(.white.opacity(0.7))
                            .frame(width: 30, height: 30)
                            .background(Color.white.opacity(0.06), in: Circle())
                    }
                    .buttonStyle(.plain)
                }
                .padding(.horizontal, 20)
                .padding(.vertical, 15)

                Divider().overlay(Color.white.opacity(0.08))

                content()
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
            .frame(minWidth: 520, maxWidth: 760, minHeight: 360, maxHeight: 620)
            .background(.ultraThinMaterial)
            .background(Color(red: 0.018, green: 0.028, blue: 0.045).opacity(0.92))
            .clipShape(RoundedRectangle(cornerRadius: 20, style: .continuous))
            .overlay(
                RoundedRectangle(cornerRadius: 20, style: .continuous)
                    .stroke(Color.white.opacity(0.12), lineWidth: 1)
            )
            .shadow(color: .black.opacity(0.55), radius: 45, y: 22)
            .transition(.scale(scale: 0.97).combined(with: .opacity))
        }
    }
}
