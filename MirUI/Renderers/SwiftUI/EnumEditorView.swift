
import SwiftUI

struct EnumEditorView: View {
    let widgetId: Int64
    let propertyName: String
    let possibleValues: [String]

    @State private var selectedValue: String

    init(widgetId: Int64, propertyName: String, currentValue: String, possibleValues: [String]) {
        self.widgetId = widgetId
        self.propertyName = propertyName
        self.possibleValues = possibleValues
        _selectedValue = State(initialValue: currentValue)
    }

    var body: some View {
        HStack {
            Text(propertyName.capitalized + ":")
                .font(.caption)
                .foregroundColor(.secondary)

            Picker("", selection: $selectedValue) {
                ForEach(possibleValues, id: \.self) { value in
                    Text(value).tag(value)
                }
            }
            .pickerStyle(MenuPickerStyle())
            .frame(maxWidth: .infinity, alignment: .leading)
            .onChange(of: selectedValue) { _, newValue in
                newValue.withCString { cStr in
                    MirUI_SetPropertyString(widgetId, propertyName.cString(using: .utf8), cStr)
                }
                MirUI_RenderFrame()
            }
        }
    }
}
