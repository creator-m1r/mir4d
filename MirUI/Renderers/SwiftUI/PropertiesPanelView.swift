import SwiftUI

struct PropertiesPanelView: View {
    @ObservedObject var appState: CADAppState
    @State private var search = ""
    
    private let tabs = ["Основное", "Геометрия", "4D", "Материалы"]
    
    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            // Header
            HStack {
                Text(appState.selectedTreeItem)
                    .font(.system(size: 14, weight: .medium))
                    .foregroundColor(.white)
                Spacer()
                Image(systemName: "info.circle")
                    .foregroundColor(Color(hex: "8A929F"))
            }
            .padding(.horizontal, 16)
            .padding(.top, 16)
            .padding(.bottom, 10)
            
            // Search
            HStack(spacing: 6) {
                Image(systemName: "magnifyingglass")
                    .font(.system(size: 11))
                    .foregroundColor(Color(hex: "8A929F"))
                TextField("Поиск по свойствам...", text: $search)
                    .textFieldStyle(.plain)
                    .font(.system(size: 11))
            }
            .padding(8)
            .background(Color.white.opacity(0.03))
            .clipShape(RoundedRectangle(cornerRadius: 8))
            .padding(.horizontal, 16)
            .padding(.bottom, 12)
            
            // Tabs
            HStack(spacing: 2) {
                ForEach(Array(tabs.enumerated()), id: \.offset) { idx, title in
                    Button(title) {
                        appState.activePropTab = idx
                    }
                    .buttonStyle(.plain)
                    .font(.system(size: 11))
                    .padding(.vertical, 5)
                    .frame(maxWidth: .infinity)
                    .background(appState.activePropTab == idx ? Color(hex: "4D8DFF").opacity(0.15) : Color.clear)
                    .foregroundColor(appState.activePropTab == idx ? Color(hex: "4D8DFF") : Color(hex: "8A929F"))
                    .clipShape(RoundedRectangle(cornerRadius: 6))
                }
            }
            .padding(3)
            .background(Color.white.opacity(0.03))
            .clipShape(RoundedRectangle(cornerRadius: 8))
            .padding(.horizontal, 16)
            .padding(.bottom, 14)
            
            // Content
            ScrollView {
                VStack(alignment: .leading, spacing: 14) {
                    switch appState.activePropTab {
                    case 0: mainProps
                    case 1: geometryProps
                    case 2: fourDProps
                    case 3: materialProps
                    default: EmptyView()
                    }
                }
                .padding(.horizontal, 16)
            }
            
            Spacer()
        }
        .background(Color(hex: "171C24"))
    }
    
    // MARK: - Tab contents
    
    private var mainProps: some View {
        PropGroup(title: "Основные параметры") {
            PropRow(label: "Тип", value: "Сборка")
            PropRow(label: "ID", value: "ASM-EVA-0045", highlight: true)
            PropRow(label: "Конфигурация", value: "Prototype A")
            PropRow(label: "Материал", value: "Ti-6Al-4V")
            PropRow(label: "Статус") {
                StatusBadge(text: "В работе", color: Color(hex: "36D98C"))
            }
        }
    }
    
    private var geometryProps: some View {
        PropGroup(title: "Геометрические характеристики") {
            PropRow(label: "Высота", value: "345 мм")
            PropRow(label: "Ширина", value: "298 мм")
            PropRow(label: "Масса", value: "0.82 кг")
            PropRow(label: "Количество деталей", value: "29 шт")
        }
    }
    
    private var fourDProps: some View {
        PropGroup(title: "4D Разработка") {
            PropRow(label: "Начало этапа", value: "12.06.2026")
            PropRow(label: "Окончание", value: "30.12.2026")
            PropRow(label: "Этап", value: "Проектирование визора", highlight: true)
            PropRow(label: "Готовность", value: "63%")
        }
    }
    
    private var materialProps: some View {
        PropGroup(title: "Материалы") {
            PropRow(label: "Корпус", value: "Титановый сплав")
            PropRow(label: "Визор", value: "—")
            PropRow(label: "Уплотнения", value: "Фторсиликон")
        }
    }
}

// MARK: - Helpers

struct PropGroup<Content: View>: View {
    let title: String
    @ViewBuilder let content: Content
    
    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(title.uppercased())
                .font(.system(size: 10, weight: .medium))
                .foregroundColor(Color(hex: "8A929F"))
                .tracking(0.4)
                .padding(.bottom, 4)
                .overlay(Rectangle().frame(height: 1).foregroundColor(Color.white.opacity(0.04)), alignment: .bottom)
            
            content
        }
    }
}

struct PropRow<Content: View>: View {
    let label: String
    var value: String? = nil
    var highlight: Bool = false
    @ViewBuilder var trailing: () -> Content
    
    init(label: String, value: String, highlight: Bool = false) where Content == EmptyView {
        self.label = label
        self.value = value
        self.highlight = highlight
        self.trailing = { EmptyView() }
    }
    
    init(label: String, @ViewBuilder trailing: @escaping () -> Content) {
        self.label = label
        self.trailing = trailing
    }
    
    var body: some View {
        HStack {
            Text(label)
                .font(.system(size: 12))
                .foregroundColor(Color(hex: "8A929F"))
            Spacer()
            if let value {
                Text(value)
                    .font(.system(size: 12))
                    .foregroundColor(highlight ? Color(hex: "2ED1FF") : .white)
            } else {
                trailing()
            }
        }
        .padding(.vertical, 4)
        .overlay(Rectangle().frame(height: 1).foregroundColor(Color.white.opacity(0.02)), alignment: .bottom)
    }
}

struct StatusBadge: View {
    let text: String
    let color: Color
    
    var body: some View {
        HStack(spacing: 4) {
            Circle().fill(color).frame(width: 6, height: 6)
            Text(text)
                .font(.system(size: 11))
                .foregroundColor(color)
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 2)
        .background(color.opacity(0.12))
        .clipShape(Capsule())
        .overlay(Capsule().stroke(color.opacity(0.15), lineWidth: 1))
    }
}