import SwiftUI
import Combine

@MainActor
final class MirUIAppearanceStore: ObservableObject {
    enum Language: String, CaseIterable, Identifiable {
        case russian = "ru"
        case english = "en"

        var id: String { rawValue }

        var locale: Locale {
            Locale(identifier: rawValue)
        }

        var displayName: String {
            switch self {
            case .russian: return "Русский"
            case .english: return "English"
            }
        }

        var shortName: String {
            rawValue.uppercased()
        }
    }

    enum Theme: String, CaseIterable, Identifiable {
        case dark = "mir.dark"
        case midnight = "mir.midnight"
        case graphite = "mir.graphite"
        case light = "mir.light"
        case highContrast = "mir.highContrast"

        var id: String { rawValue }

        var titleRU: String {
            switch self {
            case .dark: return "Тёмная"
            case .midnight: return "Полночь"
            case .graphite: return "Графит"
            case .light: return "Светлая"
            case .highContrast: return "Высокий контраст"
            }
        }

        var titleEN: String {
            switch self {
            case .dark: return "Dark"
            case .midnight: return "Midnight"
            case .graphite: return "Graphite"
            case .light: return "Light"
            case .highContrast: return "High Contrast"
            }
        }

        var title: String {
            UserDefaults.standard.string(forKey: Self.languageKey) == Language.english.rawValue
                ? titleEN
                : titleRU
        }

        var colorScheme: ColorScheme? {
            switch self {
            case .light:
                return .light
            case .dark, .midnight, .graphite, .highContrast:
                return .dark
            }
        }

        var accent: Color {
            switch self {
            case .dark: return Color(red: 0.30, green: 0.55, blue: 1.0)
            case .midnight: return Color(red: 0.36, green: 0.62, blue: 1.0)
            case .graphite: return Color(red: 0.65, green: 0.72, blue: 0.82)
            case .light: return Color(red: 0.12, green: 0.36, blue: 0.78)
            case .highContrast: return Color(red: 0.35, green: 0.90, blue: 1.0)
            }
        }

        var windowBackground: Color {
            switch self {
            case .dark: return Color(red: 0.035, green: 0.043, blue: 0.055)
            case .midnight: return Color(red: 0.018, green: 0.025, blue: 0.055)
            case .graphite: return Color(red: 0.055, green: 0.060, blue: 0.068)
            case .light: return Color(red: 0.93, green: 0.94, blue: 0.96)
            case .highContrast: return Color.black
            }
        }

        private static let languageKey = "mir4d.ui.language"
    }

    static let shared = MirUIAppearanceStore()

    @Published private(set) var language: Language
    @Published private(set) var theme: Theme

    private static let languageKey = "mir4d.ui.language"
    private static let themeKey = "mir4d.ui.theme"

    private init() {
        let storedLanguage = UserDefaults.standard.string(forKey: Self.languageKey)
        let storedTheme = UserDefaults.standard.string(forKey: Self.themeKey)

        language = Language(rawValue: storedLanguage ?? "") ?? .russian
        theme = Theme(rawValue: storedTheme ?? "") ?? .dark
    }

    func setLanguage(_ value: Language) {
        guard language != value else { return }
        language = value
        UserDefaults.standard.set(value.rawValue, forKey: Self.languageKey)
        objectWillChange.send()
    }

    func toggleLanguage() {
        setLanguage(language == .russian ? .english : .russian)
    }

    func setTheme(_ value: Theme) {
        guard theme != value else { return }
        theme = value
        UserDefaults.standard.set(value.rawValue, forKey: Self.themeKey)
        objectWillChange.send()
    }

    func reset() {
        setLanguage(.russian)
        setTheme(.dark)
    }
}

struct MirUIAppearancePanel: View {
    @ObservedObject var appearance: MirUIAppearanceStore

    var body: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.lg) {
            header
            languageSection
            themeSection
            Spacer(minLength: 0)
        }
        .padding(MirTheme.Spacing.xl)
        .frame(minWidth: 360, minHeight: 360)
        .background(appearance.theme.windowBackground)
        .foregroundStyle(MirTheme.Colors.textPrimary)
        .preferredColorScheme(appearance.theme.colorScheme)
    }

    private var header: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.xs) {
            Text("MirUI")
                .font(.system(size: 20, weight: .semibold))
            Text(appearance.language == .russian ? "Интерфейс и внешний вид" : "Interface and appearance")
                .font(MirTheme.Typography.body)
                .foregroundStyle(MirTheme.Colors.textSecondary)
        }
    }

    private var languageSection: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.sm) {
            Text(appearance.language == .russian ? "Язык интерфейса" : "Interface language")
                .font(MirTheme.Typography.section)
                .foregroundStyle(MirTheme.Colors.textSecondary)

            Picker(
                appearance.language == .russian ? "Язык" : "Language",
                selection: Binding(
                    get: { appearance.language },
                    set: { appearance.setLanguage($0) }
                )
            ) {
                ForEach(MirUIAppearanceStore.Language.allCases) { language in
                    Text(language.displayName).tag(language)
                }
            }
            .pickerStyle(.segmented)
        }
    }

    private var themeSection: some View {
        VStack(alignment: .leading, spacing: MirTheme.Spacing.sm) {
            Text(appearance.language == .russian ? "Тема" : "Theme")
                .font(MirTheme.Typography.section)
                .foregroundStyle(MirTheme.Colors.textSecondary)

            LazyVGrid(columns: [GridItem(.flexible()), GridItem(.flexible())], spacing: MirTheme.Spacing.sm) {
                ForEach(MirUIAppearanceStore.Theme.allCases) { theme in
                    themeButton(theme)
                }
            }
        }
    }

    private func themeButton(_ theme: MirUIAppearanceStore.Theme) -> some View {
        Button {
            appearance.setTheme(theme)
        } label: {
            HStack(spacing: MirTheme.Spacing.sm) {
                Circle()
                    .fill(theme.accent)
                    .frame(width: 10, height: 10)
                Text(appearance.language == .russian ? theme.titleRU : theme.titleEN)
                    .font(MirTheme.Typography.body)
                    .lineLimit(1)
                Spacer(minLength: 0)
                if appearance.theme == theme {
                    Image(systemName: "checkmark")
                        .font(.caption.weight(.bold))
                }
            }
            .padding(.horizontal, MirTheme.Spacing.md)
            .padding(.vertical, MirTheme.Spacing.sm)
            .background(theme.windowBackground.opacity(0.92), in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
            .overlay {
                RoundedRectangle(cornerRadius: MirTheme.Radius.medium)
                    .stroke(
                        appearance.theme == theme ? theme.accent.opacity(0.65) : Color.white.opacity(0.08),
                        lineWidth: appearance.theme == theme ? 1 : 0.5
                    )
            }
        }
        .buttonStyle(.plain)
    }
}

struct MirUIAppearanceToolbar: View {
    @ObservedObject var appearance: MirUIAppearanceStore
    @State private var showingPanel = false

    var body: some View {
        HStack(spacing: 6) {
            Menu {
                ForEach(MirUIAppearanceStore.Theme.allCases) { theme in
                    Button {
                        appearance.setTheme(theme)
                    } label: {
                        HStack {
                            Text(appearance.language == .russian ? theme.titleRU : theme.titleEN)
                            if appearance.theme == theme {
                                Image(systemName: "checkmark")
                            }
                        }
                    }
                }
            } label: {
                Label(
                    appearance.language == .russian ? "Тема" : "Theme",
                    systemImage: "paintbrush"
                )
            }
            .menuStyle(.borderlessButton)

            Button {
                appearance.toggleLanguage()
            } label: {
                Text(appearance.language.shortName)
                    .font(.system(size: 10, weight: .bold, design: .monospaced))
                    .frame(minWidth: 24)
            }
            .buttonStyle(.bordered)
            .help(appearance.language == .russian ? "Сменить язык" : "Change language")

            Button {
                showingPanel = true
            } label: {
                Image(systemName: "slider.horizontal.3")
            }
            .buttonStyle(.borderless)
            .help(appearance.language == .russian ? "Настройки интерфейса" : "Interface settings")
            .sheet(isPresented: $showingPanel) {
                MirUIAppearancePanel(appearance: appearance)
            }
        }
        .foregroundStyle(MirTheme.Colors.textPrimary)
    }
}

extension View {

    func mirUIAppearance(_ appearance: MirUIAppearanceStore) -> some View {
        self
            .environment(\.locale, appearance.language.locale)
            .preferredColorScheme(appearance.theme.colorScheme)
            .tint(appearance.theme.accent)
    }
}
