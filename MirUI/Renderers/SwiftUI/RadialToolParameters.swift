import SwiftUI
import Foundation

struct RadialToolParameterState: Equatable {
    var command: String
    var value: Double = 10.0
    var secondaryValue: Double = 0.0
    var isReversed: Bool = false
    var isSymmetric: Bool = false

    var titleRU: String {
        switch command {
        case "feature.extrude", "model.extrude": return "Выдавливание"
        case "transform.move": return "Перемещение"
        case "model.revolve": return "Вращение"
        case "measure.distance": return "Измерение"
        case "manufacturing.route": return "Технологический маршрут"
        case "manufacturing.submit": return "Производственный выпуск"
        default: return "Параметры инструмента"
        }
    }

    var valueTitleRU: String {
        switch command {
        case "feature.extrude", "model.extrude": return "Длина"
        case "transform.move": return "Смещение"
        case "model.revolve": return "Угол"
        case "measure.distance": return "Расстояние"
        default: return "Значение"
        }
    }

    var valueRange: ClosedRange<Double> {
        switch command {
        case "model.revolve": return -360.0...360.0
        case "measure.distance": return 0.0...10000.0
        default: return -1000.0...1000.0
        }
    }

    var valueStep: Double {
        switch command {
        case "model.revolve": return 1.0
        case "measure.distance": return 0.1
        default: return 0.5
        }
    }
}

extension Notification.Name {
    static let mir4DRadialToolActivated = Notification.Name("MIR4D.RadialToolActivated")
    static let mir4DRadialToolParameterChanged = Notification.Name("MIR4D.RadialToolParameterChanged")
    static let mir4DRadialToolCancelled = Notification.Name("MIR4D.RadialToolCancelled")
}

@MainActor
final class RadialToolParameterStore: ObservableObject {
    static let shared = RadialToolParameterStore()

    @Published private(set) var active: RadialToolParameterState?

    private init() {}

    func begin(command: String) {
        active = RadialToolParameterState(command: command)
        publishChange()
    }

    func update(value: Double) {
        guard var current = active else { return }
        current.value = min(max(value, current.valueRange.lowerBound), current.valueRange.upperBound)
        active = current
        publishChange()
    }

    func update(secondaryValue: Double) {
        guard var current = active else { return }
        current.secondaryValue = secondaryValue
        active = current
        publishChange()
    }

    func toggleReverse() {
        guard var current = active else { return }
        current.isReversed.toggle()
        active = current
        publishChange()
    }

    func toggleSymmetric() {
        guard var current = active else { return }
        current.isSymmetric.toggle()
        active = current
        publishChange()
    }

    func commit() {
        guard let current = active else { return }
        NotificationCenter.default.post(
            name: .mir4DRadialToolActivated,
            object: nil,
            userInfo: [
                "command": current.command,
                "value": current.value,
                "secondaryValue": current.secondaryValue,
                "reversed": current.isReversed,
                "symmetric": current.isSymmetric
            ]
        )
        active = nil
    }

    func cancel() {
        active = nil
        NotificationCenter.default.post(name: .mir4DRadialToolCancelled, object: nil)
    }

    private func publishChange() {
        guard let current = active else { return }
        NotificationCenter.default.post(
            name: .mir4DRadialToolParameterChanged,
            object: nil,
            userInfo: [
                "command": current.command,
                "value": current.value,
                "secondaryValue": current.secondaryValue,
                "reversed": current.isReversed,
                "symmetric": current.isSymmetric
            ]
        )
    }
}

struct RadialToolParameterOverlay: View {
    @ObservedObject var store: RadialToolParameterStore

    var body: some View {
        Group {
            if let parameter = store.active {
                VStack(alignment: .leading, spacing: 8) {
                    HStack(spacing: 7) {
                        Image(systemName: "slider.horizontal.3")
                        Text(parameter.titleRU)
                            .font(.system(size: 11, weight: .semibold))
                        Spacer(minLength: 4)
                    }

                    HStack(spacing: 10) {
                        Text(parameter.valueTitleRU)
                            .font(.system(size: 9))
                            .foregroundStyle(.white.opacity(0.7))

                        Slider(
                            value: Binding(
                                get: { parameter.value },
                                set: { store.update(value: $0) }
                            ),
                            in: parameter.valueRange,
                            step: parameter.valueStep
                        )

                        Text(String(format: "%.2f", parameter.value))
                            .font(.system(size: 10, weight: .semibold))
                            .monospacedDigit()
                            .frame(width: 58, alignment: .trailing)
                    }

                    HStack(spacing: 6) {
                        Toggle("Обратно", isOn: Binding(
                            get: { parameter.isReversed },
                            set: { _ in store.toggleReverse() }
                        ))
                        .toggleStyle(.checkbox)

                        Toggle("Симметрично", isOn: Binding(
                            get: { parameter.isSymmetric },
                            set: { _ in store.toggleSymmetric() }
                        ))
                        .toggleStyle(.checkbox)

                        Spacer()

                        Button("Отмена") { store.cancel() }
                            .buttonStyle(.borderless)

                        Button("Применить") { store.commit() }
                            .buttonStyle(.borderedProminent)
                            .controlSize(.small)
                    }
                    .font(.system(size: 9))
                }
                .padding(10)
                .frame(width: 330)
                .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 14))
                .overlay(
                    RoundedRectangle(cornerRadius: 14)
                        .stroke(MirTheme.Colors.selection.opacity(0.45), lineWidth: 1)
                )
                .shadow(radius: 14, y: 6)
                .transition(.opacity.combined(with: .scale(scale: 0.96)))
            }
        }
    }
}
