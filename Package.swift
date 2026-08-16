// swift-tools-version: 6.0

import PackageDescription

let package = Package(
    name: "MIR4DApp",
    defaultLocalization: "en",
    platforms: [
        .macOS(.v14)
    ],
    products: [
        .executable(name: "MIR4DApp", targets: ["MIR4DApp"])
    ],
    targets: [
        .executableTarget(
            name: "MIR4DApp",
            // SwiftPM must see this target as Swift-only. The SwiftUI directory
            // also contains legacy C++/Objective-C++ bridge files used by Xcode
            // and the CMake UI target, so they must be explicitly excluded.
            path: "MirUI",
            exclude: [
                "Renderers/SwiftUI/SwiftUIRenderer.mm",
                "Renderers/SwiftUI/MirUICppBridge.mm",
                "Renderers/SwiftUI/SwiftUIEventBridge.hpp",
                "Renderers/SwiftUI/MirUI-Bridging-Header.h",
                "Renderers/SwiftUI/MirUI-CAPI.h",
                "Renderers/SwiftUI/README_BUILD.md",
                "Renderers/SwiftUI/MirEngineApp.swift",
                "App/UIConfig/Inspector.ui.json",
                "App/UIConfig/TimePanel.ui.json",
                "App/UIConfig/Viewport.ui.json",
                "App/UIConfig/ProjectTree.ui.json",
                "App/UIConfig/TopBar.ui.json",
                "App/UIConfig/MIR4DUIConfig.json",
                "Renderers/SwiftUI/Resources/ru.lproj/Localizable.strings",
                "Renderers/SwiftUI/Resources/en.lproj/Localizable.strings",
                "Renderers/SwiftUI/ViewportSelectionBridge.swift",
                "Renderers/SwiftUI/ViewportSelectionState.swift",
                "Renderers/SwiftUI/ViewportSelectionEventAdapter.swift",
                "Renderers/SwiftUI/CADViewportInteractionOverlay.swift",
                "Renderers/SwiftUI/SelectionFilterBar.swift",
                "Renderers/SwiftUI/CADViewportContextBar.swift",
                "Renderers/SwiftUI/ViewportHoverState.swift"
            ],
            sources: [
                "App",
                "Renderers/SwiftUI"
            ],
            resources: [
                .process("App/UIConfig/Inspector.ui.json"),
                .process("App/UIConfig/TimePanel.ui.json"),
                .process("App/UIConfig/Viewport.ui.json"),
                .process("App/UIConfig/ProjectTree.ui.json"),
                .process("App/UIConfig/TopBar.ui.json"),
                .process("App/UIConfig/MIR4DUIConfig.json"),
                .process("Renderers/SwiftUI/Resources")
            ],
            swiftSettings: [
                .define("MIR4D_SWIFTPM")
            ]
        )
    ]
)
