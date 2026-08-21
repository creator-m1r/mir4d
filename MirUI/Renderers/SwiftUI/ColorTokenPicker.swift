
import SwiftUI
#if os(macOS)
import AppKit
#endif

class ColorTokenPickerViewModel: ObservableObject {
    @Published var selectedColor: Color = .blue
    @Published var isPickerShown = false

    enum ApplyMode {
        case themeToken(String)
        case widgetStyleField(widgetType: String, widgetState: String, fieldName: String)
        case custom(((Color) -> Void)?)
    }

    var applyMode: ApplyMode = .themeToken("")

    func loadColor() {
        var hex = "#000000FF"

        switch applyMode {
        case .themeToken(let token):
            let value = cStrToString(MirUI_GetThemeColor(token.cString(using: .utf8)))
            hex = value ?? hex

        case .widgetStyleField(let widgetType, let widgetState, let fieldName):
            let value = cStrToString(MirUI_GetWidgetStyleField(
                widgetType.cString(using: .utf8),
                widgetState.cString(using: .utf8),
                fieldName.cString(using: .utf8)
            ))
            hex = value ?? hex

        case .custom:
            return
        }

        selectedColor = Color(hex: hex)
    }

    func applyColor() {
        let hex = colorToHex(selectedColor)

        switch applyMode {
        case .themeToken(let token):
            MirUI_SetThemeColor(token.cString(using: .utf8), hex.cString(using: .utf8))

        case .widgetStyleField(let widgetType, let widgetState, let fieldName):
            MirUI_SetWidgetStyleField(
                widgetType.cString(using: .utf8),
                widgetState.cString(using: .utf8),
                fieldName.cString(using: .utf8),
                hex.cString(using: .utf8)
            )

        case .custom(let callback):
            callback?(selectedColor)
        }

        MirUI_RenderFrame()
    }

    func colorToHex(_ color: Color) -> String {
#if os(macOS)
        let nsColor = NSColor(color)
        guard let rgbColor = nsColor.usingColorSpace(.sRGB) else { return "#000000FF" }
        let r = Int(round(rgbColor.redComponent * 255))
        let g = Int(round(rgbColor.greenComponent * 255))
        let b = Int(round(rgbColor.blueComponent * 255))
        let a = Int(round(rgbColor.alphaComponent * 255))
        return String(format: "#%02X%02X%02X%02X", r, g, b, a)
#else
        return "#000000FF"
#endif
    }
}

struct ColorTokenPicker: View {
    let label: String
    @ObservedObject var vm: ColorTokenPickerViewModel

    init(label: String, applyMode: ColorTokenPickerViewModel.ApplyMode) {
        self.label = label
        _vm = ObservedObject(wrappedValue: ColorTokenPickerViewModel())
        vm.applyMode = applyMode
        vm.loadColor()
    }

    init(label: String, color: Color, onColorChanged: @escaping (Color) -> Void) {
        self.label = label
        _vm = ObservedObject(wrappedValue: ColorTokenPickerViewModel())
        vm.selectedColor = color
        vm.applyMode = .custom(onColorChanged)
    }

    var body: some View {
        HStack {
            Text(label + ":")
                .frame(width: 100, alignment: .leading)

            Circle()
                .fill(vm.selectedColor)
                .frame(width: 24, height: 24)
                .overlay(Circle().stroke(Color.gray.opacity(0.5), lineWidth: 1))
                .onTapGesture { vm.isPickerShown.toggle() }
                .popover(isPresented: $vm.isPickerShown) {
                    VStack(spacing: 12) {
                        ColorPicker("Выберите цвет", selection: $vm.selectedColor)
                            .padding()
                        HStack {
                            Button("Отмена") { vm.isPickerShown = false }
                            Spacer()
                            Button("Применить") {
                                vm.applyColor()
                                vm.isPickerShown = false
                            }
                            .buttonStyle(.borderedProminent)
                        }
                        .padding(.horizontal)
                    }
                    .frame(width: 250)
                }

            TextField("#HEX", text: Binding(
                get: { vm.colorToHex(vm.selectedColor) },
                set: { newHex in
                    vm.selectedColor = Color(hex: newHex)
                    vm.applyColor()
                }
            ))
            .font(.caption)
            .frame(width: 100)

            Button("✓") { vm.applyColor() }
                .font(.caption)
        }
        .onAppear { vm.loadColor() }
    }
}
