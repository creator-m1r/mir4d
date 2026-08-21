import SwiftUI
import Foundation

@MainActor
final class MIR4DTouchInteractionCoordinator: ObservableObject {
    enum GestureMode: Equatable {
        case idle
        case scenePan
        case sceneOrbit
        case sceneZoom
        case radial
        case radialSubmenu
    }

    @Published private(set) var mode: GestureMode = .idle
    @Published private(set) var touchCount: Int = 0
    @Published private(set) var translation: CGSize = .zero
    @Published private(set) var scale: CGFloat = 1
    @Published private(set) var rotation: Angle = .zero
    @Published private(set) var radialVector: CGVector = .zero
    @Published private(set) var radialProgress: CGFloat = 0

    var radialActivationDistance: CGFloat = 38
    var radialSubmenuDistance: CGFloat = 150
    var maximumRadialDistance: CGFloat = 280

    func beginTouches(count: Int) {
        touchCount = count
        translation = .zero
        scale = 1
        rotation = .zero
        radialProgress = 0
        mode = count >= 2 ? .sceneOrbit : .idle
    }

    func updateOneFinger(translation: CGSize, longPressActive: Bool) {
        guard touchCount <= 1 else { return }
        self.translation = translation
        mode = longPressActive ? .radial : .scenePan
    }

    func updateTwoFinger(translation: CGSize, scale: CGFloat, rotation: Angle) {
        touchCount = 2
        self.translation = translation
        self.scale = scale
        self.rotation = rotation
        mode = .sceneOrbit
    }

    func updateRadial(vector: CGVector) {
        radialVector = vector
        let distance = hypot(vector.dx, vector.dy)
        radialProgress = min(max(distance / maximumRadialDistance, 0), 1)
        mode = distance >= radialSubmenuDistance ? .radialSubmenu : .radial
    }

    func endGesture() {
        mode = .idle
        touchCount = 0
        translation = .zero
        scale = 1
        rotation = .zero
        radialVector = .zero
        radialProgress = 0
    }

    func cancel() { endGesture() }

    var isRadial: Bool { mode == .radial || mode == .radialSubmenu }
    var isSubmenu: Bool { mode == .radialSubmenu }
}

enum MIR4DTouchGeometry {
    static func radialVector(from start: CGPoint, to current: CGPoint) -> CGVector {
        CGVector(dx: current.x - start.x, dy: current.y - start.y)
    }

    static func clamped(_ vector: CGVector, maximum: CGFloat) -> CGVector {
        let length = hypot(vector.dx, vector.dy)
        guard length > maximum, length > 0 else { return vector }
        let scale = maximum / length
        return CGVector(dx: vector.dx * scale, dy: vector.dy * scale)
    }

    static func isInsideRadialDeadZone(_ vector: CGVector, radius: CGFloat) -> Bool {
        hypot(vector.dx, vector.dy) < radius
    }
}
