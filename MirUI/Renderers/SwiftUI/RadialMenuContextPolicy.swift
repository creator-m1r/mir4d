import Foundation

struct RadialMenuCommandPolicy: Codable, Hashable {
    var command: String
    var enabled: Bool = true
    var workbenches: Set<String> = []
    var selectionKinds: Set<String> = []

    func allows(_ context: CADActiveContext) -> Bool {
        guard enabled else { return false }

        if !workbenches.isEmpty && !workbenches.contains(context.workbench.rawValue) {
            return false
        }

        if !selectionKinds.isEmpty && !selectionKinds.contains(context.selection.primaryKind.rawValue) {
            return false
        }

        return true
    }
}

@MainActor
final class RadialMenuContextPolicyStore: ObservableObject {
    static let shared = RadialMenuContextPolicyStore()

    @Published var policies: [RadialMenuCommandPolicy] {
        didSet { save() }
    }

    private let key = "MIR4D.RadialMenu.ContextPolicies"

    private init() {
        if let data = UserDefaults.standard.data(forKey: key),
           let decoded = try? JSONDecoder().decode([RadialMenuCommandPolicy].self, from: data) {
            policies = decoded
        } else {
            policies = RadialMenuContextPolicyStore.defaultPolicies
        }
    }

    func policy(for command: String) -> RadialMenuCommandPolicy? {
        policies.first { $0.command == command }
    }

    func isAllowed(_ command: String, context: CADActiveContext) -> Bool {
        policy(for: command)?.allows(context) ?? true
    }

    func reset() {
        policies = Self.defaultPolicies
    }

    private func save() {
        guard let data = try? JSONEncoder().encode(policies) else { return }
        UserDefaults.standard.set(data, forKey: key)
    }

    static let defaultPolicies: [RadialMenuCommandPolicy] = [
        .init(command: "create.body", workbenches: [CADWorkbench.model.rawValue]),
        .init(command: "create.sketch", workbenches: [CADWorkbench.model.rawValue, CADWorkbench.sketch.rawValue]),
        .init(command: "feature.extrude", workbenches: [CADWorkbench.model.rawValue], selectionKinds: [CADSelectionKind.face.rawValue, CADSelectionKind.sketch.rawValue]),
        .init(command: "transform.move", workbenches: [CADWorkbench.model.rawValue, CADWorkbench.assembly.rawValue], selectionKinds: [CADSelectionKind.body.rawValue, CADSelectionKind.feature.rawValue, CADSelectionKind.component.rawValue]),
        .init(command: "assembly.component", workbenches: [CADWorkbench.assembly.rawValue]),
        .init(command: "assembly.constraint", workbenches: [CADWorkbench.assembly.rawValue], selectionKinds: [CADSelectionKind.component.rawValue]),
        .init(command: "simulation.run", workbenches: [CADWorkbench.simulation.rawValue]),
        .init(command: "simulation.pause", workbenches: [CADWorkbench.simulation.rawValue]),
        .init(command: "simulation.results", workbenches: [CADWorkbench.simulation.rawValue], selectionKinds: [CADSelectionKind.simulationResult.rawValue]),
        .init(command: "fourD.scenario", workbenches: [CADWorkbench.fourD.rawValue]),
        .init(command: "fourD.timeline", workbenches: [CADWorkbench.fourD.rawValue]),
        .init(command: "fourD.branch", workbenches: [CADWorkbench.fourD.rawValue]),
        .init(command: "drawing.view", workbenches: [CADWorkbench.drawing.rawValue]),
        .init(command: "drawing.dimension", workbenches: [CADWorkbench.drawing.rawValue], selectionKinds: [CADSelectionKind.drawingView.rawValue]),
        .init(command: "drawing.release", workbenches: [CADWorkbench.drawing.rawValue]),
        .init(command: "manufacturing.bom", workbenches: [CADWorkbench.model.rawValue, CADWorkbench.assembly.rawValue, CADWorkbench.drawing.rawValue]),
        .init(command: "manufacturing.route", workbenches: [CADWorkbench.model.rawValue, CADWorkbench.assembly.rawValue, CADWorkbench.drawing.rawValue]),
        .init(command: "manufacturing.submit", workbenches: [CADWorkbench.model.rawValue, CADWorkbench.assembly.rawValue, CADWorkbench.drawing.rawValue])
    ]
}
