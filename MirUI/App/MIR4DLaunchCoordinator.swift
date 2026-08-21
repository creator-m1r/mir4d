import Foundation
import SwiftUI

@MainActor
final class MIR4DLaunchCoordinator: ObservableObject {
    static let shared = MIR4DLaunchCoordinator()

    enum LaunchIntent: Equatable {
        case externalProject(URL)
        case restoreLast
        case startMenu
    }

    enum LaunchUIPhase: Equatable {
        case diagnostics
        case projectHub
        case workspace
    }

    @Published private(set) var pendingIntent: LaunchIntent?
    @Published private(set) var bootFinished = false
    @Published private(set) var diagnosticsCompleted = false
    @Published private(set) var phase: LaunchUIPhase = .diagnostics
    @Published private(set) var launchResolved = false

    private init() {}

    func handleOpenURL(_ url: URL) {
        let normalized = normalize(url)

        guard bootFinished else {
            pendingIntent = .externalProject(normalized)
            return
        }

        NotificationCenter.default.post(
            name: .mir4DExternalProjectURL,
            object: normalized
        )
    }

    func markBootFinished() {
        bootFinished = true
    }

    func markDiagnosticsDone() {
        diagnosticsCompleted = true
        if phase == .diagnostics {
            phase = .projectHub
        }
    }

    func showProjectHub() {
        phase = .projectHub
    }

    func revealWorkspace() {
        phase = .workspace
    }

    func markLaunchResolved() {
        launchResolved = true
    }

    func resolveAfterBoot(autoOpenLastProject: Bool) -> LaunchIntent {
        if let pendingIntent {
            self.pendingIntent = nil
            return pendingIntent
        }

        return autoOpenLastProject ? .restoreLast : .startMenu
    }

    func reset() {
        pendingIntent = nil
        bootFinished = false
        diagnosticsCompleted = false
        phase = .diagnostics
        launchResolved = false
    }

    private func normalize(_ url: URL) -> URL {

        var candidate = url.standardizedFileURL

        if candidate.pathExtension.lowercased() == MIR4DProjectStore.packageExtension {
            return candidate
        }

        var current = candidate.deletingLastPathComponent()
        while current.path != candidate.path {
            if current.pathExtension.lowercased() == MIR4DProjectStore.packageExtension {
                candidate = current
                break
            }
            let parent = current.deletingLastPathComponent()
            if parent.path == current.path { break }
            current = parent
        }

        return candidate
    }
}
