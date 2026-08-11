import SwiftUI

struct FloatingToolbarView: View {
    @ObservedObject var appState: CADAppState
    
    private let groups: [[(id: String, icon: String, tip: String)]] = [
        [
            ("select",   "cursorarrow",          "Выбор (V)"),
            ("move",     "arrow.up.and.down.and.arrow.left.and.right", "Перемещение (M)"),
            ("rotate",   "arrow.triangle.2.circlepath", "Вращение (R)")
        ],
        [
            ("zoom",     "plus.magnifyingglass", "Масштаб (Z)"),
            ("measure",  "ruler",                "Измерение (D)")
        ],
        [
            ("section",  "scissors",             "Сечение (X)"),
            ("hide",     "eye.slash",            "Скрыть (H)"),
            ("isolate",  "square.on.square.dashed", "Изолировать (I)")
        ]
    ]
    
    var body: some View {
        VStack(spacing: 2) {
            ForEach(Array(groups.enumerated()), id: \.offset) { gIdx, group in
                VStack(spacing: 2) {
                    ForEach(group, id: \.id) { tool in
                        Button {
                            appState.selectedTool = tool.id
                            appState.showNotification(tool.tip, type: .info)
                        } label: {
                            Image(systemName: tool.icon)
                                .font(.system(size: 13))
                                .frame(width: 34, height: 34)
                                .foregroundColor(appState.selectedTool == tool.id ? Color(hex: "4D8DFF") : Color(hex: "8A929F"))
                                .background(appState.selectedTool == tool.id ? Color(hex: "4D8DFF").opacity(0.15) : Color.clear)
                                .clipShape(RoundedRectangle(cornerRadius: 6))
                        }
                        .buttonStyle(.plain)
                        .help(tool.tip)
                    }
                }
                .padding(.vertical, 2)
                
                if gIdx < groups.count - 1 {
                    Divider()
                        .background(Color.white.opacity(0.04))
                        .padding(.horizontal, 6)
                }
            }
        }
        .padding(6)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 12))
        .overlay(
            RoundedRectangle(cornerRadius: 12)
                .stroke(Color.white.opacity(0.05), lineWidth: 1)
        )
        .shadow(color: .black.opacity(0.35), radius: 16, y: 8)
    }
}