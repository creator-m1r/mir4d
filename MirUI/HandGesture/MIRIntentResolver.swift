import Foundation
import Combine

/// Combines short-lived input signals into a single engineering intention.
/// No CAD command is executed here; the resolver only produces a coherent intent.
@MainActor
final class MIRIntentResolver: ObservableObject {
    static let shared = MIRIntentResolver()

    struct Configuration: Equatable, Sendable {
        var maxSignalAge: TimeInterval = 1.25
        var minimumConfidence: Double = 0.45
        var confirmationConfidence: Double = 0.78
    }

    @Published private(set) var resolvedIntent: MIRIntent?

    private(set) var configuration = Configuration()
    private var recentSignals: [MIRIntent] = []
    private var cancellable: AnyCancellable?

    init(router: MIRIntentRouter = .shared) {
        cancellable = router.publisher
            .receive(on: DispatchQueue.main)
            .sink { [weak self] intent in
                self?.ingest(intent)
            }
    }

    func ingest(_ signal: MIRIntent) {
        pruneExpiredSignals(now: signal.timestamp)
        recentSignals.append(signal)
        if recentSignals.count > 16 { recentSignals.removeFirst(recentSignals.count - 16) }
        resolvedIntent = resolve(now: signal.timestamp)
    }

    func update(configuration: Configuration) {
        self.configuration = configuration
    }

    func clear() {
        recentSignals.removeAll()
        resolvedIntent = nil
    }

    private func resolve(now: Date) -> MIRIntent? {
        guard !recentSignals.isEmpty else { return nil }

        let latest = recentSignals.last!
        let sameTarget = recentSignals.last(where: { $0.targetID != nil })?.targetID
        let action = recentSignals.last(where: { $0.action != nil })?.action
        let direction = recentSignals.last(where: { $0.directionRadians != nil })?.directionRadians
        let value = recentSignals.last(where: { $0.value != nil })?.value

        let confidence = combinedConfidence(relevant: recentSignals, now: now)
        guard confidence >= configuration.minimumConfidence else { return latest }

        let phase: MIRIntent.Phase
        if recentSignals.contains(where: { $0.phase == .execution }) {
            phase = .execution
        } else if recentSignals.contains(where: { $0.phase == .confirmation }) && confidence >= configuration.confirmationConfidence {
            phase = .confirmation
        } else if action != nil || sameTarget != nil {
            phase = .selection
        } else {
            phase = latest.phase
        }

        return MIRIntent(
            source: .system,
            phase: phase,
            action: action,
            targetID: sameTarget,
            directionRadians: direction,
            value: value,
            confidence: confidence,
            timestamp: now
        )
    }

    private func combinedConfidence(relevant signals: [MIRIntent], now: Date) -> Double {
        let recent = signals.filter { now.timeIntervalSince($0.timestamp) <= configuration.maxSignalAge }
        guard !recent.isEmpty else { return 0 }

        let sources = Set(recent.map(\.source))
        let independentSourceBonus = min(Double(max(0, sources.count - 1)) * 0.12, 0.24)
        let base = recent.map(\.confidence).max() ?? 0
        return min(base + independentSourceBonus, 1)
    }

    private func pruneExpiredSignals(now: Date) {
        recentSignals.removeAll { now.timeIntervalSince($0.timestamp) > configuration.maxSignalAge }
    }
}
