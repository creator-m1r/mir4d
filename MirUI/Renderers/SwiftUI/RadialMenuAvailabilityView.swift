import SwiftUI

struct RadialMenuAvailabilityView: View {
    let tool: RadialMenuTool?
    let context: CADActiveContext
    @ObservedObject var policyStore: RadialMenuContextPolicyStore
    let registry: CADCommandRegistry

    private var commandAvailable: Bool {
        guard let command = tool?.command else { return false }
        guard policyStore.isAllowed(command, context: context) else { return false }
        guard let registered = registry.commands.first(where: { $0.id == command }) else { return false }
        return registered.workbenches.contains(context.workbench) && registered.isAvailable(context)
    }

    private var reason: String? {
        guard let tool else { return nil }
        guard let policy = policyStore.policy(for: tool.command) else {
            return registry.commands.contains(where: { $0.id == tool.command }) ? nil : "Команда ещё не зарегистрирована"
        }

        if !policy.enabled {
            return "Команда отключена в настройках"
        }

        if !policy.workbenches.isEmpty && !policy.workbenches.contains(context.workbench.rawValue) {
            return "Недоступно в среде «\(context.workbench.titleRU)»"
        }

        if !policy.selectionKinds.isEmpty && !policy.selectionKinds.contains(context.selection.primaryKind.rawValue) {
            if !context.selection.hasSelection {
                return "Сначала выберите объект"
            }
            return "Нужен объект другого типа: \(selectionKindTitle(policy.selectionKinds))"
        }

        guard let command = registry.commands.first(where: { $0.id == tool.command }) else {
            return "Команда ещё не зарегистрирована"
        }

        if !command.workbenches.contains(context.workbench) {
            return "Команда недоступна в этой рабочей среде"
        }

        if !command.isAvailable(context) {
            return context.selection.hasSelection ? "Команда недоступна для текущего состояния" : "Сначала выберите объект"
        }

        return nil
    }

    private func selectionKindTitle(_ values: Set<String>) -> String {
        let titles = values.compactMap { value -> String? in
            switch value {
            case CADSelectionKind.vertex.rawValue: return "вершина"
            case CADSelectionKind.edge.rawValue: return "ребро"
            case CADSelectionKind.face.rawValue: return "поверхность"
            case CADSelectionKind.body.rawValue: return "тело"
            case CADSelectionKind.feature.rawValue: return "элемент"
            case CADSelectionKind.sketch.rawValue: return "эскиз"
            case CADSelectionKind.component.rawValue: return "компонент"
            case CADSelectionKind.simulationResult.rawValue: return "результат"
            case CADSelectionKind.drawingView.rawValue: return "вид чертежа"
            default: return nil
            }
        }
        return titles.joined(separator: " / ")
    }

    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: commandAvailable ? "checkmark.circle.fill" : "exclamationmark.circle.fill")
                .foregroundStyle(commandAvailable ? MirTheme.Colors.success : .orange)

            VStack(alignment: .leading, spacing: 2) {
                Text(tool?.title ?? "Выберите инструмент")
                    .font(.system(size: 11, weight: .semibold))
                    .foregroundStyle(.white)

                if let reason {
                    Text(reason)
                        .font(.system(size: 9))
                        .foregroundStyle(.white.opacity(0.7))
                        .lineLimit(2)
                } else if tool != nil {
                    Text("Готово к выполнению")
                        .font(.system(size: 9))
                        .foregroundStyle(.white.opacity(0.7))
                } else {
                    Text("Двигайте указатель по секторам")
                        .font(.system(size: 9))
                        .foregroundStyle(.white.opacity(0.7))
                }
            }
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
        .frame(minWidth: 220, maxWidth: 320)
        .background(.black.opacity(0.48), in: RoundedRectangle(cornerRadius: 12))
        .overlay(
            RoundedRectangle(cornerRadius: 12)
                .stroke(Color.white.opacity(0.12), lineWidth: 0.8)
        )
        .shadow(radius: 10, y: 5)
    }
}
