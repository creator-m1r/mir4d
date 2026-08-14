import SwiftUI

/// Тонкий SwiftUI-представитель инспектора выделенной геометрии.
/// C++ остаётся владельцем инженерных данных; Swift отвечает только за отображение.
public struct SelectionPropertiesPanel: View {
    @ObservedObject private var inspector: SelectionInspector

    public init(inspector: SelectionInspector) {
        self.inspector = inspector
    }

    public var body: some View {
        Group {
            if inspector.properties.isEmpty {
                ContentUnavailableView(
                    "Нет выделения",
                    systemImage: "square.dashed",
                    description: Text("Выберите грань в 3D-виде")
                )
            } else {
                List {
                    ForEach(inspector.properties) { property in
                        HStack(alignment: .top, spacing: 12) {
                            Text(property.name)
                                .foregroundStyle(.secondary)

                            Spacer(minLength: 8)

                            Text(property.value)
                                .multilineTextAlignment(.trailing)
                                .textSelection(.enabled)
                        }
                        .padding(.vertical, 3)
                    }
                }
                .listStyle(.inset)
            }
        }
        .navigationTitle("Свойства")
    }
}
