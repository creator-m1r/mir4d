import SwiftUI

struct RadialMenuContextPolicyView: View {
    @ObservedObject var store: RadialMenuContextPolicyStore

    private let workbenchOptions = CADWorkbench.allCases
    private let selectionOptions = [
        CADSelectionKind.none,
        .body,
        .feature,
        .sketch,
        .face,
        .edge,
        .vertex,
        .component,
        .simulationResult,
        .drawingView,
        .unknown
    ]

    var body: some View {
        List {
            ForEach(store.policies.indices, id: \.self) { index in
                policyEditor(index: index)
            }

            Section {
                Button("Восстановить политики по умолчанию") {
                    store.reset()
                }
            }

            Section("Правило") {
                Text("Пустой список рабочих сред или типов выделения означает: ограничение по этому признаку не применяется. Политика не меняет состав меню — только определяет, когда команда считается допустимой.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
        .navigationTitle("Контекст команд")
        .formStyle(.grouped)
    }

    @ViewBuilder
    private func policyEditor(index: Int) -> some View {
        let policy = store.policies[index]

        Section {
            Toggle(
                "Команда доступна",
                isOn: Binding(
                    get: { store.policies[index].enabled },
                    set: { store.policies[index].enabled = $0 }
                )
            )

            Text(policy.command)
                .font(.system(.caption, design: .monospaced))
                .foregroundStyle(.secondary)

            Menu("Рабочие среды") {
                ForEach(workbenchOptions) { workbench in
                    Button {
                        toggleWorkbench(workbench, index: index)
                    } label: {
                        Label(
                            workbench.titleRU,
                            systemImage: policy.workbenches.contains(workbench.rawValue) ? "checkmark" : ""
                        )
                    }
                }
            }

            Menu("Типы выделения") {
                ForEach(selectionOptions, id: \.rawValue) { kind in
                    Button {
                        toggleSelectionKind(kind, index: index)
                    } label: {
                        Label(
                            selectionTitle(kind),
                            systemImage: policy.selectionKinds.contains(kind.rawValue) ? "checkmark" : ""
                        )
                    }
                }
            }

            Text(policySummary(policy))
                .font(.caption)
                .foregroundStyle(.secondary)
        } header: {
            HStack {
                Image(systemName: icon(for: policy.command))
                Text(policy.command)
                    .font(.system(.subheadline, design: .monospaced))
                Spacer()
                Circle()
                    .fill(policy.enabled ? MirTheme.Colors.success : MirTheme.Colors.textTertiary)
                    .frame(width: 7, height: 7)
            }
        }
    }

    private func toggleWorkbench(_ value: CADWorkbench, index: Int) {
        var values = store.policies[index].workbenches
        if values.contains(value.rawValue) {
            values.remove(value.rawValue)
        } else {
            values.insert(value.rawValue)
        }
        store.policies[index].workbenches = values
    }

    private func toggleSelectionKind(_ value: CADSelectionKind, index: Int) {
        var values = store.policies[index].selectionKinds
        if values.contains(value.rawValue) {
            values.remove(value.rawValue)
        } else {
            values.insert(value.rawValue)
        }
        store.policies[index].selectionKinds = values
    }

    private func selectionTitle(_ kind: CADSelectionKind) -> String {
        switch kind {
        case .none: return "Нет выделения"
        case .body: return "Тело"
        case .feature: return "Элемент"
        case .sketch: return "Эскиз"
        case .face: return "Поверхность"
        case .edge: return "Ребро"
        case .vertex: return "Вершина"
        case .component: return "Компонент"
        case .simulationResult: return "Результат расчёта"
        case .drawingView: return "Вид чертежа"
        case .unknown: return "Объект"
        }
    }

    private func icon(for command: String) -> String {
        if command.hasPrefix("model.") || command.hasPrefix("feature.") { return "cube" }
        if command.hasPrefix("assembly.") { return "square.stack.3d.up" }
        if command.hasPrefix("simulation.") { return "waveform.path.ecg" }
        if command.hasPrefix("fourD.") { return "clock.arrow.circlepath" }
        if command.hasPrefix("drawing.") { return "doc.text" }
        if command.hasPrefix("manufacturing.") { return "hammer" }
        return "slider.horizontal.3"
    }

    private func policySummary(_ policy: RadialMenuCommandPolicy) -> String {
        let wb = policy.workbenches.isEmpty ? "все рабочие среды" : policy.workbenches.sorted().joined(separator: ", ")
        let sk = policy.selectionKinds.isEmpty ? "любой выбор" : policy.selectionKinds.sorted().joined(separator: ", ")
        return "Среды: \(wb) · Выбор: \(sk)"
    }
}
