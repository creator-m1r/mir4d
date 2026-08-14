import SwiftUI

struct RadialMenuLogicSettingsView: View {
    @ObservedObject var store: RadialMenuSettingsStore

    var body: some View {
        List {
            ForEach(store.settings.panels.indices, id: \.self) { panelIndex in
                Section {
                    TextField(
                        "Название панели",
                        text: Binding(
                            get: { store.settings.panels[panelIndex].title },
                            set: { store.settings.panels[panelIndex].title = $0 }
                        )
                    )

                    ForEach(store.settings.panels[panelIndex].tools.indices, id: \.self) { toolIndex in
                        VStack(alignment: .leading, spacing: 6) {
                            TextField(
                                "Инструмент",
                                text: Binding(
                                    get: { store.settings.panels[panelIndex].tools[toolIndex].title },
                                    set: { store.settings.panels[panelIndex].tools[toolIndex].title = $0 }
                                )
                            )
                            TextField(
                                "SF Symbol",
                                text: Binding(
                                    get: { store.settings.panels[panelIndex].tools[toolIndex].icon },
                                    set: { store.settings.panels[panelIndex].tools[toolIndex].icon = $0 }
                                )
                            )
                            TextField(
                                "Command ID",
                                text: Binding(
                                    get: { store.settings.panels[panelIndex].tools[toolIndex].command },
                                    set: { store.settings.panels[panelIndex].tools[toolIndex].command = $0 }
                                )
                            )
                            .font(.system(.caption, design: .monospaced))
                        }
                        .padding(.vertical, 4)
                    }
                } header: {
                    HStack {
                        Image(systemName: store.settings.panels[panelIndex].icon)
                        Text(store.settings.panels[panelIndex].title)
                        Spacer()
                        Toggle(
                            "",
                            isOn: Binding(
                                get: { store.settings.panels[panelIndex].enabled },
                                set: { store.settings.panels[panelIndex].enabled = $0 }
                            )
                        )
                        .labelsHidden()
                    }
                }
            }

            Section("Как работает") {
                Text("Command ID передаётся в CADCommandRegistry. Поэтому одна и та же команда исполняется одинаково из радиального меню, обычной панели и клавиатуры.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
        .navigationTitle("Логика радиального меню")
        .formStyle(.grouped)
    }
}

struct RadialMenuSettingsHubView: View {
    @ObservedObject var store: RadialMenuSettingsStore
    @StateObject private var contextPolicyStore = RadialMenuContextPolicyStore.shared

    var body: some View {
        NavigationStack {
            List {
                NavigationLink("Жест, радиусы и поведение") {
                    RadialMenuSettingsView(store: store)
                }
                NavigationLink("Панели и команды") {
                    RadialMenuLogicSettingsView(store: store)
                }
                NavigationLink("Контекст доступности команд") {
                    RadialMenuContextPolicyView(store: contextPolicyStore)
                }
            }
            .navigationTitle("Радиальное меню")
        }
        .frame(minWidth: 560, minHeight: 620)
    }
}
