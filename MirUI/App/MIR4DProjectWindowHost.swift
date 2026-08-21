import SwiftUI

struct MIR4DProjectWindowHost<Content: View>: View {
    let title: String
    let subtitle: String?
    let isPresented: Bool
    let onDismiss: () -> Void
    @ViewBuilder let content: () -> Content

    @Environment(\.horizontalSizeClass) private var horizontalSizeClass
    @Environment(\.verticalSizeClass) private var verticalSizeClass

    private var compact: Bool {
        horizontalSizeClass == .compact || verticalSizeClass == .compact
    }

    var body: some View {
        ZStack {
            if isPresented {
                Color.black.opacity(compact ? 0.48 : 0.38)
                    .ignoresSafeArea()
                    .transition(.opacity)

                VStack(spacing: 0) {
                    header
                    Divider().overlay(Color.white.opacity(0.08))
                    content()
                }
                .frame(
                    maxWidth: compact ? .infinity : 780,
                    maxHeight: compact ? .infinity : 650
                )
                .background(.ultraThinMaterial)
                .background(Color(red: 0.018, green: 0.028, blue: 0.045).opacity(0.94))
                .clipShape(RoundedRectangle(cornerRadius: compact ? 0 : 22, style: .continuous))
                .overlay {
                    if !compact {
                        RoundedRectangle(cornerRadius: 22, style: .continuous)
                            .stroke(Color.white.opacity(0.12), lineWidth: 1)
                    }
                }
                .shadow(color: .black.opacity(0.55), radius: compact ? 0 : 45, y: compact ? 0 : 22)
                .padding(compact ? 0 : 24)
                .transition(.scale(scale: compact ? 1 : 0.97).combined(with: .opacity))
            }
        }
        .animation(.spring(response: 0.32, dampingFraction: 0.86), value: isPresented)
    }

    private var header: some View {
        HStack(spacing: 12) {
            VStack(alignment: .leading, spacing: 3) {
                Text(title)
                    .font(.system(size: compact ? 19 : 17, weight: .semibold, design: .rounded))
                    .foregroundStyle(.white)
                if let subtitle {
                    Text(subtitle)
                        .font(.system(size: 10, weight: .medium))
                        .foregroundStyle(.white.opacity(0.45))
                }
            }

            Spacer()

            Button(action: onDismiss) {
                Image(systemName: "xmark")
                    .font(.system(size: 11, weight: .bold))
                    .foregroundStyle(.white.opacity(0.72))
                    .frame(width: compact ? 38 : 30, height: compact ? 38 : 30)
                    .background(Color.white.opacity(0.07), in: Circle())
                    .contentShape(Circle())
            }
            .buttonStyle(.plain)
            .accessibilityLabel("Закрыть")
        }
        .padding(.horizontal, compact ? 20 : 22)
        .padding(.vertical, compact ? 18 : 15)
    }
}
