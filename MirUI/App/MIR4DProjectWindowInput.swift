import SwiftUI

/// Device-independent input contract for MIR 4D floating windows.
/// Mouse, touch, trackpad and future hand tracking can feed this same model.
struct MIR4DProjectWindowInput: Equatable {
    enum Phase: Equatable {
        case began
        case changed
        case ended
        case cancelled
    }

    enum Action: Equatable {
        case focus
        case move
        case scale
        case rotate
        case close
    }

    let action: Action
    let phase: Phase
    let position: CGPoint
    let translation: CGSize
    let scale: CGFloat
    let rotation: Angle
    let confidence: Double

    static func move(
        phase: Phase,
        position: CGPoint,
        translation: CGSize = .zero,
        confidence: Double = 1
    ) -> Self {
        Self(action: .move, phase: phase, position: position, translation: translation, scale: 1, rotation: .zero, confidence: confidence)
    }

    static func pinch(
        phase: Phase,
        position: CGPoint,
        scale: CGFloat,
        confidence: Double = 1
    ) -> Self {
        Self(action: .scale, phase: phase, position: position, translation: .zero, scale: scale, rotation: .zero, confidence: confidence)
    }

    static func rotate(
        phase: Phase,
        position: CGPoint,
        rotation: Angle,
        confidence: Double = 1
    ) -> Self {
        Self(action: .rotate, phase: phase, position: position, translation: .zero, scale: 1, rotation: rotation, confidence: confidence)
    }
}

@MainActor
final class MIR4DProjectWindowInputRouter: ObservableObject {
    @Published private(set) var lastInput: MIR4DProjectWindowInput?
    @Published private(set) var focusedWindowID: UUID?

    func send(_ input: MIR4DProjectWindowInput, windowID: UUID) {
        guard input.confidence >= 0.5 else { return }
        lastInput = input
        if input.action == .focus || input.phase == .began {
            focusedWindowID = windowID
        }
    }

    func focus(_ windowID: UUID) {
        focusedWindowID = windowID
        lastInput = MIR4DProjectWindowInput(
            action: .focus,
            phase: .began,
            position: .zero,
            translation: .zero,
            scale: 1,
            rotation: .zero,
            confidence: 1
        )
    }

    func clearFocus() {
        focusedWindowID = nil
    }
}
