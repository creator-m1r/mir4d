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

    @Published private(set) var pendingIntent: LaunchIntent?
    @Published private(set) var bootFinished = false

    private init() {}

    /// Called by SwiftUI when macOS gives the application a document URL.
    /// We only remember it until boot has completed; startup owns execution.
    func handleOpenURL(_ url: URL) {
        let normalized = normalize(url)
        pendingIntent = .externalProject(normalized)
    }

    /// Marks diagnostics complete. An external URL always wins, even if it
    /// arrived while the boot sequence was still running.
    func markBootFinished() {
        bootFinished = true
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
