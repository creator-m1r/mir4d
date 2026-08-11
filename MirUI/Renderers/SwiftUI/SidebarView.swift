import SwiftUI

struct SidebarView: View {
    @ObservedObject var appState: CADAppState
    @State private var search = ""
    
    var body: some View {
        VStack(spacing: 0) {
            // Search
            HStack {
                Image(systemName: "magnifyingglass")
                    .foregroundColor(Color(hex: "8A929F"))
                TextField("Поиск по модели...", text: $search)
                    .textFieldStyle(.plain)
                    .font(.system(size: 12))
            }
            .padding(10)
            .background(Color.white.opacity(0.04))
            .clipShape(RoundedRectangle(cornerRadius: 10))
            .padding(14)
            
            // Tools
            HStack(spacing: 4) {
                ForEach(["Дерево", "Фильтр", "Слои"], id: \.self) { name in
                    Button(name) {}
                        .buttonStyle(.plain)
                        .font(.system(size: 11))
                        .padding(.vertical, 5)
                        .frame(maxWidth: .infinity)
                        .background(name == "Дерево" ? Color(hex: "4D8DFF").opacity(0.15) : Color.clear)
                        .foregroundColor(name == "Дерево" ? Color(hex: "4D8DFF") : Color(hex: "8A929F"))
                        .clipShape(RoundedRectangle(cornerRadius: 6))
                }
            }
            .padding(4)
            .background(Color.white.opacity(0.03))
            .clipShape(RoundedRectangle(cornerRadius: 8))
            .padding(.horizontal, 14)
            
            // Tree
            ScrollView {
                LazyVStack(alignment: .leading, spacing: 1) {
                    ForEach(appState.treeData) { node in
                        TreeItemView(node: node, appState: appState, level: 0)
                    }
                }
                .padding(.horizontal, 10)
                .padding(.top, 12)
            }
            
            Spacer()
            
            // Load model button – теперь открывает NSOpenPanel
            Button {
                openModelPanel()
            } label: {
                Label("Добавить 3D-модель", systemImage: "plus.square.on.square")
                    .font(.system(size: 12, weight: .medium))
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 10)
            }
            .buttonStyle(.plain)
            .background(Color(hex: "4D8DFF").opacity(0.15))
            .foregroundColor(Color(hex: "4D8DFF"))
            .clipShape(RoundedRectangle(cornerRadius: 10))
            .padding(14)
        }
        .background(Color(hex: "171C24"))
    }
    
    // Новая функция для выбора файла 3D-модели
    private func openModelPanel() {
        let panel = NSOpenPanel()
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        panel.allowedContentTypes = [
            .init(filenameExtension: "obj")!,
            .init(filenameExtension: "stl")!,
            .init(filenameExtension: "gltf")!,
            .init(filenameExtension: "glb")!,
            .init(filenameExtension: "fbx")!,
            .init(filenameExtension: "step")!,
            .init(filenameExtension: "stp")!
        ]
        panel.begin { response in
            if response == .OK, let url = panel.url {
                appState.loadModel(url: url)
            }
        }
    }
}