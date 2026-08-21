import Foundation
import SwiftUI

/// Single source of truth for the application's launch decision.
/// Priority is intentionally fixed:
/// 1. external project URL supplied by macOS
/// 2. restore the last project (when enabled)
/// 3. show the start menu
@MainActor
final class MIR4DLaunchCoordinator: ObservableObject {
    static let shared = MIR4DLaunchCoordinator()

    enum LaunchIntent: Equatable {
        case externalProject(URL)
        case restoreLast
        case startMenu
    }

    /// Persistent UI phase for the launch experience. Lives above
    /// `MIR4DLaunchExperienceView` so it survives view recreation (create
    /// project / transition to workspace) instead of resetting to diagnostics.
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

    /// Called by SwiftUI when macOS gives the application a document URL.
    /// Before boot completes the URL remains pending and participates in the
    /// normal external > restore > menu launch decision. After boot completes
    /// it is delivered immediately so a second double-click/Open With action is
    /// never stranded on the already-visible Project Hub.
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

    /// Marks diagnostics complete. An external URL always wins, even if it
    /// arrived while the boot sequence was still running.
    func markBootFinished() {
        bootFinished = true
    }

    /// Marks diagnostics complete and advances to the project hub when the
    /// launch experience is still sitting on the diagnostics card.
    func markDiagnosticsDone() {
        diagnosticsCompleted = true
        if phase == .diagnostics {
            phase = .projectHub
        }
    }

    /// Returns the user to the project hub (e.g. after closing a project).
    func showProjectHub() {
        phase = .projectHub
    }

    /// Advances directly to the immersive workspace. Intended for the create /
    /// open project transition; phase is switched immediately so the view is
    /// never rebuilt through a diagnostics reset.
    func revealWorkspace() {
        phase = .workspace
    }

    /// Marks the post-boot launch decision (external / restore / menu) as
    /// applied. Survives view recreation so the hub is not resolved twice.
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
        // A .mir4d project is a package directory. If macOS supplies a URL
        // pointing at a child item, walk up to the package root when possible.
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
