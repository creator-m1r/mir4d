import SwiftUI

struct MIR4DSculptSettingsHUD: View {
    @ObservedObject private var settings = MIR4DSculptSettings.shared
    @ObservedObject private var indicator = MIR4DBrushIndicator.shared
    private var russian: Bool {
        (Locale.preferredLanguages.first ?? "").hasPrefix("ru")
    }

    var body: some View {
        if indicator.active {
            VStack(alignment: .leading, spacing: 8) {
                Text(russian ? "Кисть" : "Brush")
                    .font(.system(size: 11, weight: .semibold))
                    .foregroundStyle(.white.opacity(0.8))

                HStack(spacing: 6) {
                    Text(russian ? "Радиус" : "Radius").font(.system(size: 10)).foregroundStyle(.white.opacity(0.7))
                    Slider(value: $settings.radiusScale, in: 0.05...1.0)
                        .frame(width: 130)
                }

                HStack(spacing: 6) {
                    Text(russian ? "Сила" : "Strength").font(.system(size: 10)).foregroundStyle(.white.opacity(0.7))
                    Slider(value: $settings.strengthScale, in: 0.005...0.2)
                        .frame(width: 130)
                }
            }
            .padding(10)
            .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 10))
            .overlay(RoundedRectangle(cornerRadius: 10).stroke(.white.opacity(0.12), lineWidth: 1))
            .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topTrailing)
            .padding([.top, .trailing], 14)
            .allowsHitTesting(true)
        }
    }
}
