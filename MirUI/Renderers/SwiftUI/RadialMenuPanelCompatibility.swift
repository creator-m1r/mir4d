import Foundation

// Compatibility adapter for panel-centric selection code.
// The canonical collection remains [RadialMenuPanel] and integer indexing is unchanged.
extension Array where Element == RadialMenuPanel {
    subscript(_ panel: RadialMenuPanel) -> RadialMenuPanel {
        panel
    }
}
