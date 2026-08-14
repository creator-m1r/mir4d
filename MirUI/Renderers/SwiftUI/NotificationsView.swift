import SwiftUI

struct NotificationsView: View {
    @ObservedObject var appState: CADAppState
    
    var body: some View {
        VStack(alignment: .trailing, spacing: 6) {
            ForEach(appState.notifications) { notif in
                HStack(spacing: 8) {
                    Image(systemName: icon(for: notif.type))
                    Text(notif.message)
                        .font(.system(size: 12))
                }
                .padding(.horizontal, 14)
                .padding(.vertical, 10)
                .background(.ultraThinMaterial)
                .clipShape(RoundedRectangle(cornerRadius: 10))
                .overlay(
                    RoundedRectangle(cornerRadius: 10)
                        .stroke(Color.white.opacity(0.06), lineWidth: 1)
                )
                .overlay(
                    Rectangle()
                        .fill(borderColor(for: notif.type))
                        .frame(width: 3),
                    alignment: .leading
                )
                .shadow(color: .black.opacity(0.35), radius: 12, y: 4)
                .transition(.move(edge: .trailing).combined(with: .opacity))
            }
        }
        .animation(.spring(response: 0.4, dampingFraction: 0.8), value: appState.notifications.count)
    }
    
    private func icon(for type: NotificationType) -> String {
        switch type {
        case .success: return "checkmark.circle.fill"
        case .warning: return "exclamationmark.triangle.fill"
        case .error:   return "xmark.octagon.fill"
        case .info:    return "info.circle.fill"
        }
    }
    
    private func borderColor(for type: NotificationType) -> Color {
        switch type {
        case .success: return Color(hex: "36D98C")
        case .warning: return Color(hex: "FFB84D")
        case .error:   return Color(hex: "FF4D4D")
        case .info:    return Color(hex: "4D8DFF")
        }
    }
}