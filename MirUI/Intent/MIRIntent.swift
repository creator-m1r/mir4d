import Foundation

/// A device-independent expression of an engineer's intention.
/// Input devices produce intents; the CAD layer decides whether and how they become actions.
struct MIRIntent: Equatable, Identifiable {
    enum Source: String, Equatable, Sendable {
        case voice
        case touch
        case trackpad
        case mouse
        case keyboard
        case gaze
        case spatial
        case system
    }

    enum Phase: String, Equatable, Sendable {
        case attention
        case preview
        case selection
        case confirmation
        case execution
        case cancel
    }

    let id: UUID
    let source: Source
    let phase: Phase
    let action: String?
    let targetID: String?
    let directionRadians: Double?
    let value: Double?
    let confidence: Double
    let timestamp: Date

    init(
        id: UUID = UUID(),
        source: Source,
        phase: Phase,
        action: String? = nil,
        targetID: String? = nil,
        directionRadians: Double? = nil,
        value: Double? = nil,
        confidence: Double = 1,
        timestamp: Date = Date()
    ) {
        self.id = id
        self.source = source
        self.phase = phase
        self.action = action
        self.targetID = targetID
        self.directionRadians = directionRadians
        self.value = value
        self.confidence = min(max(confidence, 0), 1)
        self.timestamp = timestamp
    }
}
