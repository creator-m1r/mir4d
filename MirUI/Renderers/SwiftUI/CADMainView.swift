import SwiftUI

struct CADMainView: View {
    @StateObject private var appState = CADAppState()
    
    var body: some View {
        ZStack {
            Color(hex: "0E1116").ignoresSafeArea()
            
            VStack(spacing: 1) {
                // TOP BAR
                TopBarView(appState: appState)
                    .frame(height: 64)
                
                HStack(spacing: 1) {
                    // LEFT SIDEBAR
                    SidebarView(appState: appState)
                        .frame(width: 280)
                    
                    // VIEWPORT
                    ZStack {
                        MirGLView()
                            .background(Color(hex: "0B0E12"))
                        
                        // Floating toolbar
                        HStack {
                            FloatingToolbarView(appState: appState)
                                .padding(.leading, 12)
                            Spacer()
                        }
                        
                        // Hint
                        VStack {
                            Spacer()
                            Text("ЛКМ: выбор · Колесико: зум · ПКМ: панорама · Tab: цикл инструментов")
                                .font(.system(size: 11))
                                .foregroundColor(Color(hex: "8A929F"))
                                .padding(.horizontal, 14)
                                .padding(.vertical, 5)
                                .background(.ultraThinMaterial)
                                .clipShape(Capsule())
                                .padding(.bottom, 14)
                        }
                    }
                    
                    // PROPERTIES
                    PropertiesPanelView(appState: appState)
                        .frame(width: 320)
                }
                
                // TIMELINE
                TimelinePanelView(appState: appState)
                    .frame(height: 260)
            }
            
            // Notifications
            VStack {
                HStack {
                    Spacer()
                    NotificationsView(appState: appState)
                        .padding(.trailing, 20)
                        .padding(.top, 72)
                }
                Spacer()
            }
        }
        .onAppear {
            appState.showNotification("Активный этап: проектирование системы жизнеобеспечения", type: .success)
        }
    }
}