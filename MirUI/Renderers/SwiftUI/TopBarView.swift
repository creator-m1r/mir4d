import SwiftUI

struct TopBarView: View {
    @ObservedObject var appState: CADAppState
    @State private var activeTab = "4D Тестирование"
    
    let tabs = ["Модель", "Сборка", "CAE Анализ", "4D Тестирование", "Совместная работа"]
    
    var body: some View {
        HStack {
            // Logo
            HStack(spacing: 10) {
                Image(systemName: "sun.max.fill")
                    .foregroundColor(Color(hex: "FFB84D"))
                    .font(.system(size: 18))
                Text("M1R.PRO – 4D САПР")
                    .font(.system(size: 15, weight: .bold))
                    .foregroundStyle(
                        LinearGradient(colors: [.white, Color(hex: "8A929F")],
                                       startPoint: .leading, endPoint: .trailing)
                    )
            }
            
            Spacer()
            
            // Tabs
            HStack(spacing: 4) {
                ForEach(tabs, id: \.self) { tab in
                    Button(tab) {
                        activeTab = tab
                        appState.showNotification(tab, type: .success)
                    }
                    .buttonStyle(.plain)
                    .padding(.horizontal, 14)
                    .padding(.vertical, 6)
                    .background(activeTab == tab ? Color(hex: "4D8DFF").opacity(0.2) : Color.clear)
                    .foregroundColor(activeTab == tab ? .white : Color(hex: "8A929F"))
                    .clipShape(RoundedRectangle(cornerRadius: 8))
                }
            }
            .padding(3)
            .background(Color.white.opacity(0.03))
            .clipShape(RoundedRectangle(cornerRadius: 10))
            
            Spacer()
            
            // Actions
            HStack(spacing: 18) {
                Image(systemName: "icloud.and.arrow.up")
                Image(systemName: "bell")
                Image(systemName: "paintpalette")
                
                Circle()
                    .fill(LinearGradient(colors: [Color(hex: "4D8DFF"), Color(hex: "2ED1FF")],
                                         startPoint: .topLeading, endPoint: .bottomTrailing))
                    .frame(width: 30, height: 30)
                    .overlay(Text("AK").font(.system(size: 11, weight: .semibold)).foregroundColor(.white))
            }
            .foregroundColor(Color(hex: "8A929F"))
            .font(.system(size: 15))
        }
        .padding(.horizontal, 24)
        .background(.ultraThinMaterial)
        .overlay(Rectangle().frame(height: 1).foregroundColor(Color.white.opacity(0.05)), alignment: .bottom)
    }
}