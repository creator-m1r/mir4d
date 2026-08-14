import SwiftUI
import AppKit
import UniformTypeIdentifiers

private let mir4DImportTypes: [UTType] = [
    UTType(filenameExtension: "stl", conformingTo: .data),
    UTType(filenameExtension: "obj", conformingTo: .data),
    UTType(filenameExtension: "ply", conformingTo: .data),
    UTType(filenameExtension: "gltf", conformingTo: .data),
    UTType(filenameExtension: "glb", conformingTo: .data),
    UTType(filenameExtension: "fbx", conformingTo: .data)
].compactMap { $0 }

// CADMainView layout refinements are intentionally UI-only.
// MirEngine remains the owner of geometry, camera state and engineering data.
// The existing implementation below is preserved by the application; this
// small shell establishes the viewport chrome without covering the model.

struct CADViewportChrome: View {
    @ObservedObject var appState: CADAppState
    @Binding var cameraTheta: Double
    @Binding var cameraPhi: Double
    @Binding var cameraDistance: Double
    @Binding var showEmptyState: Bool

    var body: some View {
        ZStack {
            viewportBackground
            viewportControls
            viewportReadout
        }
        .background(MirTheme.Colors.viewport)
        .clipped()
    }

    private var viewportBackground: some View {
        Rectangle()
            .fill(MirTheme.Colors.viewport)
            .overlay(alignment: .topLeading) {
                HStack(spacing: 7) {
                    Image(systemName: "cube.transparent")
                        .font(.system(size: 11, weight: .semibold))
                        .foregroundStyle(MirTheme.Colors.accentBright)
                    Text(appState.ui.language == .russian ? "3D ВИД" : "3D VIEW")
                        .font(.system(size: 10, weight: .semibold))
                        .tracking(0.6)
                        .foregroundStyle(MirTheme.Colors.textSecondary)
                }
                .padding(.horizontal, 10)
                .padding(.vertical, 7)
                .background(MirTheme.Colors.surfaceRaised.opacity(0.88), in: Capsule())
                .overlay(Capsule().stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 1))
                .padding(10)
            }
    }

    private var viewportControls: some View {
        VStack {
            HStack {
                Spacer()
                VStack(spacing: 6) {
                    viewportButton("arrow.up.left.and.arrow.down.right", "Подогнать модель") {
                        NotificationCenter.default.post(name: .mir4DFitViewport, object: nil)
                    }
                    viewportButton("viewfinder", "Центрировать") {
                        NotificationCenter.default.post(name: .mir4DFitViewport, object: nil)
                    }
                }
                .padding(8)
                .background(MirTheme.Colors.surfaceRaised.opacity(0.88), in: RoundedRectangle(cornerRadius: MirTheme.Radius.medium))
                .overlay(RoundedRectangle(cornerRadius: MirTheme.Radius.medium).stroke(MirTheme.Colors.panelBorder, lineWidth: 1))
                .padding(10)
            }
            Spacer()
        }
    }

    private func viewportButton(_ image: String, _ help: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: image)
                .font(.system(size: 11, weight: .medium))
                .frame(width: 26, height: 26)
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .background(MirTheme.Colors.surface.opacity(0.65), in: RoundedRectangle(cornerRadius: MirTheme.Radius.small))
        }
        .buttonStyle(.plain)
        .help(help)
    }

    private var viewportReadout: some View {
        VStack {
            Spacer()
            HStack(spacing: 10) {
                Label("X\(format(cameraTheta))", systemImage: "arrow.left.and.right")
                Label("Y\(format(cameraPhi))", systemImage: "arrow.up.and.down")
                Label("D\(format(cameraDistance))", systemImage: "ruler")
                Spacer()
                Text(appState.ui.language == .russian ? "Колесо — масштаб · ПКМ — панорама" : "Wheel — zoom · RMB — pan")
                    .font(.system(size: 9))
                    .foregroundStyle(MirTheme.Colors.textTertiary)
            }
            .font(.system(size: 9, weight: .medium, design: .monospaced))
            .foregroundStyle(MirTheme.Colors.textSecondary)
            .padding(.horizontal, 10)
            .padding(.vertical, 6)
            .background(MirTheme.Colors.surfaceRaised.opacity(0.88), in: Capsule())
            .overlay(Capsule().stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 1))
            .padding(10)
        }
    }

    private func format(_ value: Double) -> String { String(format: "%.1f", value) }
}
