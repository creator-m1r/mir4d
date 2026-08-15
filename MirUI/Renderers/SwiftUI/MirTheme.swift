import SwiftUI

/// Single visual language for MIR 4D.
///
/// The theme deliberately uses opaque application surfaces. CAD viewports and
/// engineering panels must remain visually stable while overlays are animated.
enum MirTheme {
    enum Colors {
        static let topBar = Color(red: 0.028, green: 0.034, blue: 0.044)
        static let background = Color(red: 0.035, green: 0.043, blue: 0.055)
        static let viewport = Color(red: 0.020, green: 0.027, blue: 0.035)
        static let surface = Color(red: 0.055, green: 0.067, blue: 0.086)
        static let surfaceRaised = Color(red: 0.075, green: 0.090, blue: 0.115)
        static let panel = Color(red: 0.065, green: 0.078, blue: 0.098)
        static let panelRaised = Color(red: 0.085, green: 0.102, blue: 0.128)
        static let border = Color.white.opacity(0.075)
        static let borderStrong = Color.white.opacity(0.13)
        static let panelBorder = borderStrong
        static let textPrimary = Color.white.opacity(0.94)
        static let textSecondary = Color.white.opacity(0.65)
        static let textTertiary = Color.white.opacity(0.38)
        static let textDisabled = Color.white.opacity(0.22)
        static let accent = Color(red: 0.30, green: 0.55, blue: 1.0)
        static let accentBright = Color(red: 0.26, green: 0.85, blue: 1.0)
        static let accentSoft = accent.opacity(0.18)
        static let selection = Color(red: 0.30, green: 0.55, blue: 1.0)
        static let selectionSoft = selection.opacity(0.14)
        static let selectionGlow = selection.opacity(0.32)
        static let geometry = Color.white.opacity(0.88)
        static let geometrySecondary = Color.white.opacity(0.55)
        static let construction = Color.white.opacity(0.30)
        static let sketch = Color(red: 0.25, green: 0.85, blue: 1.0)
        static let constraint = Color(red: 0.95, green: 0.65, blue: 0.25)
        static let constraintSatisfied = Color(red: 0.30, green: 0.90, blue: 0.55)
        static let constraintWarning = Color(red: 1.0, green: 0.72, blue: 0.20)
        static let constraintError = Color(red: 1.0, green: 0.28, blue: 0.30)
        static let simulation = Color(red: 0.70, green: 0.35, blue: 1.0)
        static let simulationStructural = Color(red: 0.25, green: 0.55, blue: 1.0)
        static let simulationThermal = Color(red: 1.0, green: 0.30, blue: 0.20)
        static let simulationFluid = Color(red: 0.20, green: 0.80, blue: 0.95)
        static let time = Color(red: 0.25, green: 0.90, blue: 0.75)
        static let timeline = Color(red: 0.30, green: 0.55, blue: 1.0)
        static let keyframe = Color(red: 1.0, green: 0.70, blue: 0.20)
        static let event = Color(red: 0.95, green: 0.40, blue: 0.75)
        static let success = Color(red: 0.30, green: 0.90, blue: 0.55)
        static let warning = Color(red: 1.0, green: 0.72, blue: 0.20)
        static let error = Color(red: 1.0, green: 0.28, blue: 0.30)
        static let info = Color(red: 0.30, green: 0.65, blue: 1.0)
    }

    enum Spacing {
        static let xxs: CGFloat = 2
        static let xs: CGFloat = 4
        static let sm: CGFloat = 8
        static let md: CGFloat = 12
        static let lg: CGFloat = 16
        static let xl: CGFloat = 24
        static let xxl: CGFloat = 32
    }

    enum Radius {
        static let small: CGFloat = 4
        static let medium: CGFloat = 7
        static let large: CGFloat = 10
        static let panel: CGFloat = 12
        static let floating: CGFloat = 14
    }

    enum Typography {
        static let status = Font.system(size: 10, weight: .regular)
        static let caption = Font.system(size: 11, weight: .regular)
        static let body = Font.system(size: 12, weight: .regular)
        static let bodyMedium = Font.system(size: 12, weight: .medium)
        static let bodySemibold = Font.system(size: 12, weight: .semibold)
        static let title = Font.system(size: 14, weight: .semibold)
        static let section = Font.system(size: 11, weight: .semibold)
        static let numeric = Font.system(size: 12, weight: .medium, design: .monospaced)
        static let coordinate = Font.system(size: 11, weight: .medium, design: .monospaced)
    }

    enum Animation {
        static let fast = SwiftUI.Animation.easeOut(duration: 0.12)
        static let normal = SwiftUI.Animation.easeInOut(duration: 0.18)
        static let slow = SwiftUI.Animation.easeInOut(duration: 0.28)
        static let spring = SwiftUI.Animation.spring(response: 0.22, dampingFraction: 0.82)
    }
}
