import SwiftUI

struct IdeaStudioView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var store: ProductionWorldStore

    @State private var brief = "Оборудование для автоматизированной производственной линии"
    @State private var constraints = "Габариты, производительность, безопасность, ремонтопригодность"
    @State private var goal = "Создать машину, которую можно испытать в цифровом мире и отправить в производство"
    @State private var variantCount = 3

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            header
            fields
            presets
            footer
        }
        .padding(12)
        .frame(maxWidth: 620)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 12))
        .overlay {
            RoundedRectangle(cornerRadius: 12)
                .stroke(MirTheme.Colors.panelBorder.opacity(0.7), lineWidth: 1)
        }
        .padding(.leading, 14)
        .padding(.top, 110)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
    }

    private var header: some View {
        HStack {
            Label(appState.ui.language == .russian ? "Инженерная мастерская идеи" : "Engineering Idea Studio", systemImage: "lightbulb.max")
                .font(.system(size: 13, weight: .bold))
            Spacer()
            Text("MIR AI")
                .font(.system(size: 9, weight: .bold, design: .monospaced))
                .foregroundStyle(MirTheme.Colors.time)
        }
    }

    private var fields: some View {
        VStack(spacing: 8) {
            field("Замысел / Brief", text: $brief)
            field("Ограничения", text: $constraints)
            field("Цель", text: $goal)
        }
    }

    private func field(_ title: String, text: Binding<String>) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(title)
                .font(.system(size: 10, weight: .semibold))
                .foregroundStyle(MirTheme.Colors.textSecondary)
            TextField("", text: text, axis: .vertical)
                .textFieldStyle(.roundedBorder)
                .lineLimit(1...3)
        }
    }

    private var presets: some View {
        HStack(spacing: 6) {
            preset("Машина", icon: "gearshape.2")
            preset("Линия", icon: "rectangle.3.group")
            preset("Робот", icon: "figure.walk.motion")
            preset("Оснастка", icon: "wrench.and.screwdriver")
        }
    }

    private func preset(_ title: String, icon: String) -> some View {
        Button {
            brief = "Разработать: \(title.lowercased()) для производственного процесса"
        } label: {
            Label(title, systemImage: icon)
        }
        .buttonStyle(.bordered)
        .controlSize(.small)
    }

    private var footer: some View {
        HStack {
            Stepper("Вариантов: \(variantCount)", value: $variantCount, in: 1...8)
                .font(.system(size: 10))

            Spacer()

            Button {
                store.completion[.idea] = 1
                store.advance(to: .model)
                appState.selectWorkbench(.model)
                appState.showNotification(
                    appState.ui.language == .russian ? "Замысел принят · переходим к 3D-модели" : "Brief accepted · moving to 3D model",
                    type: .success
                )
            } label: {
                Label(appState.ui.language == .russian ? "Создать варианты" : "Generate Variants", systemImage: "wand.and.stars")
            }
            .buttonStyle(.borderedProminent)
        }
    }
}
