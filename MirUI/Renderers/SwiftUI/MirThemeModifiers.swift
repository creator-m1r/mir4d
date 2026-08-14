import SwiftUI

struct MirPanelModifier: ViewModifier {
    let raised: Bool

    func body(content: Content) -> some View {
        content
            .background(raised ? MirTheme.Colors.panelRaised : MirTheme.Colors.panel)
            .overlay {
                RoundedRectangle(cornerRadius: MirTheme.Radius.panel)
                    .stroke(MirTheme.Colors.border, lineWidth: 1)
            }
            .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.panel))
    }
}

struct MirFloatingModifier: ViewModifier {
    func body(content: Content) -> some View {
        content
            .background(.ultraThinMaterial)
            .background(MirTheme.Colors.surface.opacity(0.86))
            .clipShape(RoundedRectangle(cornerRadius: MirTheme.Radius.floating))
            .overlay {
                RoundedRectangle(cornerRadius: MirTheme.Radius.floating)
                    .stroke(MirTheme.Colors.border, lineWidth: 1)
            }
            .shadow(radius: 20, y: 8)
    }
}

extension View {
    func mirPanel(raised: Bool = false) -> some View {
        modifier(MirPanelModifier(raised: raised))
    }

    func mirFloating() -> some View {
        modifier(MirFloatingModifier())
    }
}
