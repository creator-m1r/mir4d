import SwiftUI

enum MIR4DTouchInteractionPolicy {
    enum Platform {
        case iPad
        case macTrackpad
        case macMouse
    }

    struct Metrics: Equatable {
        let minimumTouchTarget: CGFloat
        let radialDeadZone: CGFloat
        let radialFirstOrbit: CGFloat
        let radialSecondOrbit: CGFloat
        let radialMaximum: CGFloat
        let sceneGestureMinimumDistance: CGFloat
        let sceneNavigationScale: CGFloat
        let blurRadius: CGFloat
    }

    static func metrics(for platform: Platform, shortestSide: CGFloat) -> Metrics {
        switch platform {
        case .iPad:
            let scale = max(1, min(shortestSide / 768, 1.35))
            return Metrics(
                minimumTouchTarget: 52 * scale,
                radialDeadZone: 34 * scale,
                radialFirstOrbit: 118 * scale,
                radialSecondOrbit: 188 * scale,
                radialMaximum: 270 * scale,
                sceneGestureMinimumDistance: 8 * scale,
                sceneNavigationScale: 1.0,
                blurRadius: 8
            )
        case .macTrackpad:
            return Metrics(
                minimumTouchTarget: 40,
                radialDeadZone: 28,
                radialFirstOrbit: 96,
                radialSecondOrbit: 158,
                radialMaximum: 230,
                sceneGestureMinimumDistance: 4,
                sceneNavigationScale: 1.0,
                blurRadius: 7
            )
        case .macMouse:
            return Metrics(
                minimumTouchTarget: 36,
                radialDeadZone: 24,
                radialFirstOrbit: 88,
                radialSecondOrbit: 148,
                radialMaximum: 220,
                sceneGestureMinimumDistance: 2,
                sceneNavigationScale: 1.0,
                blurRadius: 6
            )
        }
    }

    static func platform(isMac: Bool, pointerIsMouse: Bool) -> Platform {
        #if os(iOS)
        return .iPad
        #else
        return pointerIsMouse ? .macMouse : .macTrackpad
        #endif
    }

    static func radialCenter(in size: CGSize, topInset: CGFloat = 0, bottomInset: CGFloat = 0) -> CGPoint {
        let usableTop = max(0, topInset)
        let usableBottom = max(0, bottomInset)
        let height = max(0, size.height - usableTop - usableBottom)
        return CGPoint(x: size.width * 0.5, y: usableTop + height * 0.5)
    }

    static func radialOrbit(for distance: CGFloat, metrics: Metrics) -> Int {
        if distance < metrics.radialFirstOrbit { return 0 }
        if distance < metrics.radialSecondOrbit { return 1 }
        return 2
    }
}
