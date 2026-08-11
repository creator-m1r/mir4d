import SwiftUI

struct TimelinePanelView: View {
    @ObservedObject var appState: CADAppState
    @State private var viewMode = "День"
    
    private let tasks: [(wbs: String, name: String, duration: String, progress: Int, color: Color, offset: CGFloat, width: CGFloat)] = [
        ("1",   "Концепция изделия",          "5", 100, Color(hex: "4D8DFF"), 0.02, 0.22),
        ("1.1", "Эскизное проектирование",    "4", 100, Color(hex: "36D98C"), 0.18, 0.16),
        ("1.2", "3D-моделирование корпуса",   "6", 100, Color(hex: "4D8DFF"), 0.02, 0.24),
        ("1.3", "Проектирование визора",      "4",  75, Color(hex: "4D8DFF"), 0.30, 0.18),
        ("1.4", "CAE-анализ",                 "3",  20, Color(hex: "36D98C"), 0.44, 0.14),
        ("1.5", "4D-моделирование корпуса",   "4",   0, Color(hex: "FFB84D"), 0.56, 0.16),
        ("1.6", "Подготовка CAM",             "3",  45, Color(hex: "4D8DFF"), 0.70, 0.18)
    ]
    
    var body: some View {
        VStack(spacing: 0) {
            // Controls
            HStack(spacing: 14) {
                Button { } label: { Image(systemName: "backward.end.fill") }
                Button {
                    appState.isPlaying.toggle()
                    appState.showNotification(appState.isPlaying ? "▶️ Воспроизведение" : "⏸ Пауза", type: .success)
                } label: {
                    Image(systemName: appState.isPlaying ? "pause.fill" : "play.fill")
                }
                Button { } label: { Image(systemName: "forward.end.fill") }
                
                Text("Июнь 2026")
                    .font(.system(size: 12, weight: .medium))
                    .foregroundColor(Color(hex: "8A929F"))
                
                Button { } label: { Image(systemName: "minus.magnifyingglass") }
                Button { } label: { Image(systemName: "plus.magnifyingglass") }
                
                Spacer()
                
                HStack(spacing: 2) {
                    ForEach(["День", "Неделя", "Месяц"], id: \.self) { mode in
                        Button(mode) { viewMode = mode }
                            .buttonStyle(.plain)
                            .font(.system(size: 11))
                            .padding(.horizontal, 10)
                            .padding(.vertical, 3)
                            .background(viewMode == mode ? Color(hex: "4D8DFF").opacity(0.15) : Color.clear)
                            .foregroundColor(viewMode == mode ? .white : Color(hex: "8A929F"))
                            .clipShape(RoundedRectangle(cornerRadius: 4))
                    }
                }
                .padding(2)
                .background(Color.white.opacity(0.03))
                .clipShape(RoundedRectangle(cornerRadius: 6))
            }
            .buttonStyle(.plain)
            .foregroundColor(Color(hex: "8A929F"))
            .font(.system(size: 13))
            .padding(.horizontal, 16)
            .frame(height: 40)
            .overlay(Rectangle().frame(height: 1).foregroundColor(Color.white.opacity(0.04)), alignment: .bottom)
            
            // Legend
            HStack(spacing: 14) {
                legendItem(color: Color(hex: "4D8DFF"), text: "Концепт")
                legendItem(color: Color(hex: "36D98C"), text: "3D-моделирование")
                legendItem(color: Color(hex: "FFB84D"), text: "CAE-анализ")
                legendItem(color: Color(hex: "FF7B4D"), text: "4D-моделирование")
            }
            .font(.system(size: 10))
            .foregroundColor(Color(hex: "8A929F"))
            .padding(.horizontal, 16)
            .padding(.vertical, 4)
            .overlay(Rectangle().frame(height: 1).foregroundColor(Color.white.opacity(0.03)), alignment: .bottom)
            
            // Gantt
            HStack(spacing: 0) {
                // Left table
                VStack(spacing: 0) {
                    ganttHeaderRow
                    ForEach(tasks, id: \.wbs) { task in
                        HStack(spacing: 6) {
                            Text(task.wbs).frame(width: 28, alignment: .leading)
                                .foregroundColor(Color(hex: "8A929F"))
                            Text(task.name)
                                .lineLimit(1)
                                .foregroundColor(.white)
                            Spacer()
                            Text(task.duration)
                                .frame(width: 32, alignment: .trailing)
                                .foregroundColor(Color(hex: "8A929F"))
                            Text("\(task.progress)")
                                .frame(width: 28, alignment: .trailing)
                                .foregroundColor(task.progress == 100 ? Color(hex: "36D98C") :
                                                 task.progress > 0 ? Color(hex: "FFB84D") : Color(hex: "8A929F"))
                        }
                        .font(.system(size: 11))
                        .padding(.horizontal, 10)
                        .frame(height: 30)
                        .background(task.wbs == "1.3" ? Color(hex: "4D8DFF").opacity(0.05) : Color.clear)
                    }
                }
                .frame(width: 280)
                .overlay(Rectangle().frame(width: 1).foregroundColor(Color.white.opacity(0.04)), alignment: .trailing)
                
                // Chart
                GeometryReader { geo in
                    ZStack(alignment: .topLeading) {
                        VStack(spacing: 0) {
                            // Days header
                            HStack {
                                ForEach(["1 июн","5","10","15","20","25","30"], id: \.self) { d in
                                    Text(d).font(.system(size: 9)).foregroundColor(Color(hex: "8A929F"))
                                    if d != "30" { Spacer() }
                                }
                            }
                            .padding(.horizontal, 8)
                            .frame(height: 22)
                            
                            ForEach(tasks, id: \.wbs) { task in
                                ZStack(alignment: .leading) {
                                    Color.clear.frame(height: 30)
                                    RoundedRectangle(cornerRadius: 4)
                                        .fill(LinearGradient(colors: [task.color, task.color.opacity(0.7)],
                                                             startPoint: .leading, endPoint: .trailing))
                                        .frame(width: max(8, geo.size.width * task.width), height: 16)
                                        .offset(x: geo.size.width * task.offset)
                                        .shadow(color: .black.opacity(0.2), radius: 3, y: 1)
                                        .onTapGesture {
                                            appState.showNotification("📊 \(task.name)", type: .warning)
                                        }
                                }
                                .background(task.wbs == "1.3" ? Color(hex: "4D8DFF").opacity(0.05) : Color.clear)
                            }
                        }
                        
                        // Current day line
                        Rectangle()
                            .fill(Color(hex: "FFB84D"))
                            .frame(width: 2)
                            .offset(x: geo.size.width * 0.45)
                            .overlay(
                                Text("Сегодня")
                                    .font(.system(size: 9))
                                    .foregroundColor(Color(hex: "FFB84D"))
                                    .padding(.horizontal, 5)
                                    .background(Color.black.opacity(0.6))
                                    .clipShape(Capsule())
                                    .offset(y: -14),
                                alignment: .top
                            )
                    }
                }
            }
        }
        .background(Color(hex: "171C24"))
    }
    
    private var ganttHeaderRow: some View {
        HStack {
            Text("WBS").frame(width: 28, alignment: .leading)
            Text("Задача")
            Spacer()
            Text("Длит.").frame(width: 32, alignment: .trailing)
            Text("%").frame(width: 28, alignment: .trailing)
        }
        .font(.system(size: 11, weight: .semibold))
        .foregroundColor(Color(hex: "8A929F"))
        .padding(.horizontal, 10)
        .frame(height: 28)
        .background(Color.white.opacity(0.02))
    }
    
    private func legendItem(color: Color, text: String) -> some View {
        HStack(spacing: 4) {
            RoundedRectangle(cornerRadius: 2).fill(color).frame(width: 10, height: 10)
            Text(text)
        }
    }
}