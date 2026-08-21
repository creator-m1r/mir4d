import SwiftUI

extension Notification.Name {
    static let mir4DRadialMenuBegan = Notification.Name("MIR4D.RadialMenuBegan")
    static let mir4DRadialMenuMoved = Notification.Name("MIR4D.RadialMenuMoved")
    static let mir4DRadialMenuEnded = Notification.Name("MIR4D.RadialMenuEnded")
}

struct RadialMenuTool: Codable, Identifiable, Hashable { var id: UUID = UUID(); var title: String; var icon: String; var command: String }
struct RadialMenuPanel: Codable, Identifiable, Hashable { var id: UUID = UUID(); var title: String; var icon: String; var enabled: Bool = true; var tools: [RadialMenuTool] }
struct RadialMenuSettings: Codable {
    var enabled = true; var holdDuration: Double = 0.18; var deadZone: Double = 28
    var panelRadius: Double = 82; var submenuOffset: Double = 128; var activationRadius: Double = 172
    var sectorGap: Double = 0.10; var showLabels = true; var hapticEnabled = true
    var keyboardTriggerEnabled = true; var middleMouseTriggerEnabled = true; var leftMouseHoldTriggerEnabled = true
    var magneticStrength: Double = 0.72; var magneticHysteresis: Double = 0.08
    var panels: [RadialMenuPanel] = RadialMenuSettings.defaultPanels

    var hideUnavailable = false
    var showPreview = true
    var soundEnabled = false
    var autoConfirm = false
    var adaptiveRadius = true
    var showContextLabel = true
    var velocityAware = true
    var confirmationRequired = true
    var centerCoreEnabled = true
    var groupByWorkbench = false

    static let defaultPanels: [RadialMenuPanel] = [
        RadialMenuPanel(title:"Создать",icon:"plus",tools:[RadialMenuTool(title:"Тело",icon:"cube.transparent",command:"create.body"),RadialMenuTool(title:"Эскиз",icon:"pencil.and.ruler",command:"create.sketch"),RadialMenuTool(title:"Форма",icon:"shape",command:"create.form"),RadialMenuTool(title:"Компонент",icon:"shippingbox",command:"assembly.component")]),
        RadialMenuPanel(title:"Изменить",icon:"slider.horizontal.3",tools:[RadialMenuTool(title:"Размер",icon:"ruler",command:"modify.dimension"),RadialMenuTool(title:"Форма",icon:"wand.and.stars",command:"modify.form"),RadialMenuTool(title:"Положение",icon:"arrow.up.and.down.and.arrow.left.and.arrow.right",command:"transform.move"),RadialMenuTool(title:"Связь",icon:"link",command:"modify.constraint")]),
        RadialMenuPanel(title:"Соединить",icon:"link",tools:[RadialMenuTool(title:"Связать",icon:"link",command:"assembly.constraint"),RadialMenuTool(title:"Собрать",icon:"square.stack.3d.up",command:"assembly.component"),RadialMenuTool(title:"Разделить",icon:"rectangle.split.3x1",command:"modify.split")]),
        RadialMenuPanel(title:"Измерить",icon:"ruler",tools:[RadialMenuTool(title:"Расстояние",icon:"arrow.left.and.right",command:"measure.distance"),RadialMenuTool(title:"Угол",icon:"angle",command:"measure.angle"),RadialMenuTool(title:"Размер",icon:"ruler",command:"measure.dimension")]),
        RadialMenuPanel(title:"Посмотреть",icon:"eye",tools:[RadialMenuTool(title:"Показать всё",icon:"arrow.up.left.and.arrow.down.right",command:"view.fit"),RadialMenuTool(title:"Изометрия",icon:"cube",command:"view.isometric"),RadialMenuTool(title:"Проекция",icon:"square.split.2x2",command:"view.orthographic")]),
        RadialMenuPanel(title:"Время",icon:"clock.arrow.circlepath",tools:[RadialMenuTool(title:"Сценарий",icon:"point.3.connected.trianglepath.dotted",command:"fourD.scenario"),RadialMenuTool(title:"Время",icon:"clock",command:"fourD.timeline"),RadialMenuTool(title:"Изменение",icon:"waveform.path.ecg",command:"fourD.change")]),
        RadialMenuPanel(title:"Проект",icon:"folder",tools:[RadialMenuTool(title:"Открыть",icon:"folder",command:"project.open"),RadialMenuTool(title:"Сохранить",icon:"square.and.arrow.down",command:"file.save"),RadialMenuTool(title:"Экспорт",icon:"square.and.arrow.up",command:"file.export")]),
        RadialMenuPanel(title:"Назад",icon:"arrow.uturn.backward",tools:[RadialMenuTool(title:"Отменить",icon:"arrow.uturn.backward",command:"edit.undo"),RadialMenuTool(title:"Повторить",icon:"arrow.uturn.forward",command:"edit.redo"),RadialMenuTool(title:"Закрыть",icon:"xmark",command:"navigation.close")])
    ]

    enum CodingKeys: String, CodingKey {
        case enabled, holdDuration, deadZone, panelRadius, submenuOffset, activationRadius, sectorGap
        case showLabels, hapticEnabled, keyboardTriggerEnabled, middleMouseTriggerEnabled, leftMouseHoldTriggerEnabled
        case magneticStrength, magneticHysteresis, panels
        case hideUnavailable, showPreview, soundEnabled, autoConfirm, adaptiveRadius
        case showContextLabel, velocityAware, confirmationRequired, centerCoreEnabled, groupByWorkbench
    }

    init() {}

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        enabled = try c.decodeIfPresent(Bool.self, forKey: .enabled) ?? true
        holdDuration = try c.decodeIfPresent(Double.self, forKey: .holdDuration) ?? 0.18
        deadZone = try c.decodeIfPresent(Double.self, forKey: .deadZone) ?? 28
        panelRadius = try c.decodeIfPresent(Double.self, forKey: .panelRadius) ?? 82
        submenuOffset = try c.decodeIfPresent(Double.self, forKey: .submenuOffset) ?? 128
        activationRadius = try c.decodeIfPresent(Double.self, forKey: .activationRadius) ?? 172
        sectorGap = try c.decodeIfPresent(Double.self, forKey: .sectorGap) ?? 0.10
        showLabels = try c.decodeIfPresent(Bool.self, forKey: .showLabels) ?? true
        hapticEnabled = try c.decodeIfPresent(Bool.self, forKey: .hapticEnabled) ?? true
        keyboardTriggerEnabled = try c.decodeIfPresent(Bool.self, forKey: .keyboardTriggerEnabled) ?? true
        middleMouseTriggerEnabled = try c.decodeIfPresent(Bool.self, forKey: .middleMouseTriggerEnabled) ?? true
        leftMouseHoldTriggerEnabled = try c.decodeIfPresent(Bool.self, forKey: .leftMouseHoldTriggerEnabled) ?? true
        magneticStrength = try c.decodeIfPresent(Double.self, forKey: .magneticStrength) ?? 0.72
        magneticHysteresis = try c.decodeIfPresent(Double.self, forKey: .magneticHysteresis) ?? 0.08
        panels = try c.decodeIfPresent([RadialMenuPanel].self, forKey: .panels) ?? RadialMenuSettings.defaultPanels
        hideUnavailable = try c.decodeIfPresent(Bool.self, forKey: .hideUnavailable) ?? false
        showPreview = try c.decodeIfPresent(Bool.self, forKey: .showPreview) ?? true
        soundEnabled = try c.decodeIfPresent(Bool.self, forKey: .soundEnabled) ?? false
        autoConfirm = try c.decodeIfPresent(Bool.self, forKey: .autoConfirm) ?? false
        adaptiveRadius = try c.decodeIfPresent(Bool.self, forKey: .adaptiveRadius) ?? true
        showContextLabel = try c.decodeIfPresent(Bool.self, forKey: .showContextLabel) ?? true
        velocityAware = try c.decodeIfPresent(Bool.self, forKey: .velocityAware) ?? true
        confirmationRequired = try c.decodeIfPresent(Bool.self, forKey: .confirmationRequired) ?? true
        centerCoreEnabled = try c.decodeIfPresent(Bool.self, forKey: .centerCoreEnabled) ?? true
        groupByWorkbench = try c.decodeIfPresent(Bool.self, forKey: .groupByWorkbench) ?? false
    }

    func encode(to encoder: Encoder) throws {
        var c = encoder.container(keyedBy: CodingKeys.self)
        try c.encode(enabled, forKey: .enabled)
        try c.encode(holdDuration, forKey: .holdDuration)
        try c.encode(deadZone, forKey: .deadZone)
        try c.encode(panelRadius, forKey: .panelRadius)
        try c.encode(submenuOffset, forKey: .submenuOffset)
        try c.encode(activationRadius, forKey: .activationRadius)
        try c.encode(sectorGap, forKey: .sectorGap)
        try c.encode(showLabels, forKey: .showLabels)
        try c.encode(hapticEnabled, forKey: .hapticEnabled)
        try c.encode(keyboardTriggerEnabled, forKey: .keyboardTriggerEnabled)
        try c.encode(middleMouseTriggerEnabled, forKey: .middleMouseTriggerEnabled)
        try c.encode(leftMouseHoldTriggerEnabled, forKey: .leftMouseHoldTriggerEnabled)
        try c.encode(magneticStrength, forKey: .magneticStrength)
        try c.encode(magneticHysteresis, forKey: .magneticHysteresis)
        try c.encode(panels, forKey: .panels)
        try c.encode(hideUnavailable, forKey: .hideUnavailable)
        try c.encode(showPreview, forKey: .showPreview)
        try c.encode(soundEnabled, forKey: .soundEnabled)
        try c.encode(autoConfirm, forKey: .autoConfirm)
        try c.encode(adaptiveRadius, forKey: .adaptiveRadius)
        try c.encode(showContextLabel, forKey: .showContextLabel)
        try c.encode(velocityAware, forKey: .velocityAware)
        try c.encode(confirmationRequired, forKey: .confirmationRequired)
        try c.encode(centerCoreEnabled, forKey: .centerCoreEnabled)
        try c.encode(groupByWorkbench, forKey: .groupByWorkbench)
    }
}

@MainActor final class RadialMenuSettingsStore: ObservableObject {
    static let shared = RadialMenuSettingsStore(); @Published var settings: RadialMenuSettings { didSet { save() } }
    private let key="MIR4D.RadialMenu.Settings"
    private init(){ if let data=UserDefaults.standard.data(forKey:key),let value=try? JSONDecoder().decode(RadialMenuSettings.self,from:data){settings=value}else{settings=RadialMenuSettings()} }
    func reset(){settings=RadialMenuSettings()}; private func save(){guard let data=try? JSONEncoder().encode(settings) else{return};UserDefaults.standard.set(data,forKey:key)}
}

enum RadialMenuGeometry {

    static func normalizedAngle(_ angle:Double)->Double{var value=angle.truncatingRemainder(dividingBy:Double.pi*2);if value<0{value += Double.pi*2};return value}
    static func shortestSignedAngle(from:Double,to:Double)->Double{atan2(sin(to-from),cos(to-from))}

    static func sectorCenter(_ index:Int,_ count:Int)->Double{guard count>0 else{return -Double.pi/2};return normalizedAngle(-Double.pi/2+(Double.pi*2/Double(count))*Double(index))}

    static func nearestIndex(angle:Double,count:Int)->Int?{guard count>0 else{return nil};let sector=Double.pi*2/Double(count);return min(max(Int(floor((normalizedAngle(angle)+sector/2)/sector)),0),count-1)}

    static func hystereticSector(angle:Double,count:Int,previous:Int?,hysteresis:Double)->Int{
        guard count>0 else{return 0}
        let sector=Double.pi*2/Double(count)
        let candidate=nearestIndex(angle:angle,count:count) ?? 0
        guard let previous,previous>=0,previous<count else{return candidate}
        if candidate==previous{return previous}
        let previousCenter=sectorCenter(previous,count)
        let distance=abs(shortestSignedAngle(from:previousCenter,to:angle))
        let threshold=sector*(0.5+min(max(hysteresis,0),0.45))
        return distance>threshold ? candidate : previous
    }

    static func magneticStrength(forDistance distance:Double,settings s:RadialMenuSettings)->Double{
        guard distance>s.deadZone else{return 0}
        let normalized=min(max((distance-s.deadZone)/120.0,0),1)
        return min(max(s.magneticStrength*(0.4+0.6*normalized),0),1)
    }

    static func magneticAngle(raw:Double,count:Int,strength:Double)->Double{guard count>0 else{return raw};let sector=Double.pi*2/Double(count);let center=Double(nearestIndex(angle:raw,count:count) ?? 0)*sector;let delta=atan2(sin(center-raw),cos(center-raw));return normalizedAngle(raw+delta*min(max(strength,0),1))}

    static func panelIndex(for dx:Double,dy:Double,settings s:RadialMenuSettings,previous:Int?=nil)->Int?{
        panelIndex(for:dx,dy:dy,panels:enabledPanels(s),settings:s,previous:previous)
    }

    static func panelIndex(for dx:Double,dy:Double,panels:[RadialMenuPanel],settings s:RadialMenuSettings,previous:Int?=nil)->Int?{
        guard !panels.isEmpty else{return nil}
        let distance=hypot(dx,dy)
        guard distance>=s.deadZone else{return nil}
        let raw=normalizedAngle(atan2(dy,dx))
        let stabilized=magneticAngle(raw:raw,count:panels.count,strength:magneticStrength(forDistance:distance,settings:s))
        return selectSector(angle:stabilized,count:panels.count,gap:s.sectorGap,previous:previous,hysteresis:s.magneticHysteresis)
    }

    static func toolIndex(for dx:Double,dy:Double,panel:RadialMenuPanel,settings s:RadialMenuSettings,previous:Int?=nil)->Int?{
        toolIndex(for:dx,dy:dy,tools:panel.tools,settings:s,previous:previous)
    }

    static func toolIndex(for dx:Double,dy:Double,tools:[RadialMenuTool],settings s:RadialMenuSettings,previous:Int?=nil)->Int?{
        guard hypot(dx,dy)>=s.activationRadius,!tools.isEmpty else{return nil}
        let raw=normalizedAngle(atan2(dy,dx))
        let stabilized=magneticAngle(raw:raw,count:tools.count,strength:magneticStrength(forDistance:hypot(dx,dy),settings:s))
        return selectSector(angle:stabilized,count:tools.count,gap:s.sectorGap,previous:previous,hysteresis:s.magneticHysteresis)
    }

    static func selectSector(angle:Double,count:Int,gap:Double,previous:Int?,hysteresis:Double)->Int{
        guard count>0 else{return 0}
        let sector=Double.pi*2/Double(count)
        let nearest=nearestIndex(angle:angle,count:count) ?? 0
        let center=sectorCenter(nearest,count)
        let fromCenter=abs(shortestSignedAngle(from:center,to:angle))
        let selectableHalf=sector/2*(1-min(max(gap,0),0.9))
        if fromCenter>selectableHalf{
            if let previous,previous>=0,previous<count{return previous}
            return nearest
        }
        return hystereticSector(angle:angle,count:count,previous:previous,hysteresis:hysteresis)
    }

    static func enabledPanels(_ settings:RadialMenuSettings)->[RadialMenuPanel]{settings.panels.filter(\.enabled)}
}

struct RadialMenuView:View{
    @ObservedObject var store:RadialMenuSettingsStore
    let center:CGPoint
    let vector:CGVector
    let appState:CADAppState
    let onToolActivated:(RadialMenuTool)->Void
    let onSettings:()->Void
    var forcedPanelIndex:Int? = nil
    var forcedToolIndex:Int? = nil

    private var context:RadialMenuContext{ RadialMenuContext(appState:appState) }
    private var distance:Double{ hypot(vector.dx,vector.dy) }

    private func isToolAvailable(_ tool:RadialMenuTool)->Bool{
        guard let command=RadialCommandRegistry.shared.command(for:tool.command) else { return true }
        return command.isAvailable(context)
    }
    private func isToolVisible(_ tool:RadialMenuTool)->Bool{
        store.settings.hideUnavailable ? isToolAvailable(tool) : true
    }
    private var visiblePanels:[RadialMenuPanel]{
        store.settings.panels.filter { p in
            guard p.enabled else { return false }
            if store.settings.hideUnavailable, !p.tools.isEmpty {
                return p.tools.contains { isToolAvailable($0) }
            }
            return true
        }
    }
    private func visibleTools(for panel:RadialMenuPanel)->[RadialMenuTool]{
        panel.tools.filter { isToolVisible($0) }
    }
    private var selectedPanelIndex:Int?{
        if let f=forcedPanelIndex, visiblePanels.indices.contains(f) { return f }
        return RadialMenuGeometry.panelIndex(for:vector.dx,dy:vector.dy,panels:visiblePanels,settings:store.settings)
    }
    private var selectedPanel:RadialMenuPanel?{ guard let i=selectedPanelIndex,visiblePanels.indices.contains(i) else { return nil }; return visiblePanels[i] }
    private var selectedToolIndex:Int?{
        if let f=forcedToolIndex, let p=selectedPanel, visibleTools(for:p).indices.contains(f) { return f }
        guard let p=selectedPanel else { return nil }
        return RadialMenuGeometry.toolIndex(for:vector.dx,dy:vector.dy,tools:visibleTools(for:p),settings:store.settings)
    }
    private var selectedTool:RadialMenuTool?{ guard let p=selectedPanel,let i=selectedToolIndex,visibleTools(for:p).indices.contains(i) else { return nil }; return visibleTools(for:p)[i] }

    static func visiblePanels(store:RadialMenuSettingsStore,context:RadialMenuContext)->[RadialMenuPanel]{
        store.settings.panels.filter { p in
            guard p.enabled else { return false }
            if store.settings.hideUnavailable, !p.tools.isEmpty {
                return p.tools.contains { tool in
                    guard let c=RadialCommandRegistry.shared.command(for:tool.command) else { return true }
                    return c.isAvailable(context)
                }
            }
            return true
        }
    }
    static func visibleTools(panel:RadialMenuPanel,store:RadialMenuSettingsStore,context:RadialMenuContext)->[RadialMenuTool]{
        panel.tools.filter { tool in
            store.settings.hideUnavailable ? (RadialCommandRegistry.shared.command(for:tool.command)?.isAvailable(context) ?? true) : true
        }
    }

    var body:some View{
        ZStack{ gestureRay; centerCore; panelRing; if let p=selectedPanel { toolRing(panel:p) } }
            .frame(width:520,height:520).position(center).allowsHitTesting(false)
            .transition(.opacity.combined(with:.scale(scale:0.88)))
            .onAppear { MirEventBus.shared.publish(.radialMenu(.opened)) }
            .onChange(of:selectedPanelIndex) { MirEventBus.shared.publish(.radialMenu(.panelChanged(selectedPanel?.title))) }
            .onChange(of:selectedToolIndex) { MirEventBus.shared.publish(.radialMenu(.toolChanged(selectedTool?.title))) }
    }

    private var gestureRay:some View{
        let l=max(distance,1); let ux=vector.dx/l,uy=vector.dy/l
        return Path{ path in path.move(to:CGPoint(x:260,y:260)); path.addLine(to:CGPoint(x:260+vector.dx*0.62,y:260+vector.dy*0.62)) }
            .stroke(MirTheme.Colors.selection.opacity(0.28),style:StrokeStyle(lineWidth:1.2,lineCap:.round,dash:[3,7]))
            .overlay{ Circle().fill(MirTheme.Colors.selection.opacity(0.9)).frame(width:6,height:6).position(x:260+ux*min(distance,210),y:260+uy*min(distance,210)) }
    }

    private var centerCore:some View{
        VStack(spacing:3){
            Image(systemName:selectedToolIndex != nil ? "scope" : "cursorarrow.motionlines").font(.system(size:18,weight:.semibold))
            if store.settings.showContextLabel, let ctx=contextLabel {
                Text(ctx).font(.system(size:8,weight:.bold)).lineLimit(1).foregroundStyle(.white.opacity(0.65))
            }
            Text(selectedToolIndex != nil ? (selectedTool?.title ?? "") : (selectedPanel?.title ?? "МИР")).font(.system(size:10,weight:.bold)).lineLimit(1)
            if let cmd=selectedTool.flatMap({ RadialCommandRegistry.shared.command(for:$0.command) }), store.settings.confirmationRequired {
                Text(cmd.confirmation == .destructive ? "Подтвердите Enter" : (cmd.confirmation == .preview ? "Предпросмотр" : ""))
                    .font(.system(size:7,weight:.bold)).lineLimit(1)
                    .foregroundStyle(cmd.confirmation == .destructive ? .red : .white.opacity(0.6))
            }
        }
        .foregroundStyle(.white)
        .frame(width:68,height:68)
        .background(.ultraThinMaterial,in:Circle())
        .overlay(Circle().stroke(MirTheme.Colors.accentBright.opacity(0.8),lineWidth:1))
        .accessibilityElement(children:.combine)
        .accessibilityLabel("Радиальное меню")
        .accessibilityValue(selectedTool?.title ?? selectedPanel?.title ?? "МИР")
        .overlay(alignment:.bottomTrailing){
            Button(action:onSettings){ Image(systemName:"gearshape.fill").font(.system(size:9)).foregroundStyle(.white.opacity(0.7)).padding(7).background(.black.opacity(0.45),in:Circle()) }.buttonStyle(.plain).allowsHitTesting(true)
        }
    }
    private var contextLabel:String?{
        switch appState.workbench {
        case .model: return "3D Модель"
        case .sketch: return "Эскиз"
        case .assembly: return "Сборка"
        case .simulation: return "Симуляция"
        case .fourD: return "4D"
        case .drawing: return "Чертёж"
        case .collaboration: return "Совместно"
        case .visualization: return "Визуализация"
        }
    }

    private var panelRing:some View{
        ZStack{
            Circle().stroke(.white.opacity(0.10),lineWidth:1).frame(width:store.settings.panelRadius*2.1,height:store.settings.panelRadius*2.1)
            ForEach(Array(visiblePanels.enumerated()),id:\.element.id){ i,p in
                let selected=selectedPanelIndex==i
                radialButton(title:p.title,icon:p.icon,selected:selected,position:point(radius:store.settings.panelRadius,angle:sectorAngle(index:i,count:visiblePanels.count)))
                    .accessibilityLabel(p.title)
                    .accessibilityHint("Направление")
            }
        }
    }

    private func toolRing(panel:RadialMenuPanel)->some View{
        let tools=visibleTools(for:panel)
        let l=max(distance,1)
        let origin=CGPoint(x:260+vector.dx/l*store.settings.submenuOffset,y:260+vector.dy/l*store.settings.submenuOffset)
        return ZStack{
            Path{ path in path.move(to:CGPoint(x:260,y:260)); path.addLine(to:origin) }.stroke(MirTheme.Colors.selection.opacity(0.32),lineWidth:1)
            Circle().stroke(MirTheme.Colors.selection.opacity(0.32),lineWidth:1).frame(width:116,height:116).position(origin)
            ForEach(Array(tools.enumerated()),id:\.element.id){ i,t in
                let selected=selectedToolIndex==i
                let available=isToolAvailable(t)
                radialButton(title:store.settings.showLabels ? t.title:"",icon:t.icon,selected:selected,available:available,position:CGPoint(x:origin.x+CGFloat(cos(sectorAngle(index:i,count:tools.count)))*52,y:origin.y+CGFloat(sin(sectorAngle(index:i,count:tools.count)))*52))
                    .accessibilityLabel(t.title)
                    .accessibilityHint(available ? "Инструмент" : "Недоступно в текущем контексте")
            }
        }
    }

    private func radialButton(title:String,icon:String,selected:Bool,available:Bool=true,position:CGPoint)->some View{
        VStack(spacing:2){
            Image(systemName:icon).font(.system(size:selected ? 15:12,weight:.semibold))
            if !title.isEmpty{ Text(title).font(.system(size:8,weight:.semibold)).lineLimit(1) }
        }
        .foregroundStyle(selected ? .white : (available ? .white.opacity(0.74) : .white.opacity(0.28)))
        .frame(width:selected ? 78:64,height:selected ? 48:42)
        .background(selected ? MirTheme.Colors.selection.opacity(0.82) : Color.black.opacity(available ? 0.42 : 0.2),in:RoundedRectangle(cornerRadius:12))
        .overlay(RoundedRectangle(cornerRadius:12).stroke(selected ? MirTheme.Colors.selection : Color.white.opacity(available ? 0.12 : 0.04),lineWidth:selected ? 1.5:0.8))
        .scaleEffect(selected ? 1.10:1)
        .animation(.interactiveSpring(response:0.22,dampingFraction:0.74),value:selected)
        .position(position)
    }

    private func sectorAngle(index:Int,count:Int)->Double{ -Double.pi/2+(Double.pi*2/Double(max(count,1)))*Double(index) }
    private func point(radius:Double,angle:Double)->CGPoint{ CGPoint(x:260+CGFloat(cos(angle)*radius),y:260+CGFloat(sin(angle)*radius)) }
}

struct RadialMenuSettingsView:View{@ObservedObject var store:RadialMenuSettingsStore;var body:some View{NavigationStack{Form{Section("Управление"){Toggle("Радиальное меню включено",isOn:binding(\.enabled));Toggle("Средняя кнопка мыши",isOn:binding(\.middleMouseTriggerEnabled));Toggle("Удержание левой кнопки",isOn:binding(\.leftMouseHoldTriggerEnabled));Toggle("Клавиша ]",isOn:binding(\.keyboardTriggerEnabled));Toggle("Тактильная обратная связь",isOn:binding(\.hapticEnabled))};Section("Контекст и обратная связь"){Toggle("Скрывать недоступные команды",isOn:binding(\.hideUnavailable));Toggle("Предпросмотр команды",isOn:binding(\.showPreview));Toggle("Звуковая обратная связь",isOn:binding(\.soundEnabled));Toggle("Автоподтверждение",isOn:binding(\.autoConfirm));Toggle("Адаптивный радиус",isOn:binding(\.adaptiveRadius));Toggle("Показывать контекст в центре",isOn:binding(\.showContextLabel));Toggle("Учёт скорости жеста",isOn:binding(\.velocityAware));Toggle("Подтверждение действий",isOn:binding(\.confirmationRequired));Toggle("Центральное ядро",isOn:binding(\.centerCoreEnabled));Toggle("Группировать по рабочему столу",isOn:binding(\.groupByWorkbench))};Section("Жест"){slider("Зона покоя",keyPath:\.deadZone,range:8...80,step:1,suffix:" pt");slider("Радиус панелей",keyPath:\.panelRadius,range:50...150,step:1,suffix:" pt");slider("Смещение подменю",keyPath:\.submenuOffset,range:80...220,step:1,suffix:" pt");slider("Радиус активации",keyPath: \.activationRadius,range:110...280,step:1,suffix:" pt");slider("Магнитное притяжение",keyPath:\.magneticStrength,range:0...1,step:0.01,suffix:"");slider("Устойчивость выбора",keyPath:\.magneticHysteresis,range:0...0.2,step:0.01,suffix:"");Toggle("Подписи инструментов",isOn:binding(\.showLabels))};Section("Направления"){Text("Первый уровень выражает человеческое намерение. Продолжение движения раскрывает следующий уровень.").font(.callout).foregroundStyle(.secondary)};Section("Панели и инструменты"){ForEach(store.settings.panels.indices,id:\.self){PanelEditor(store:store,index:$0)}};Section{Button("Восстановить заводскую схему"){store.reset()}}}.navigationTitle("Настройки радиального меню").formStyle(.grouped)}.frame(minWidth:560,minHeight:620)}
    private func binding<T>(_ keyPath:WritableKeyPath<RadialMenuSettings,T>)->Binding<T>{Binding(get:{store.settings[keyPath:keyPath]},set:{store.settings[keyPath:keyPath]=$0})}
    private func slider(_ title:String,keyPath:WritableKeyPath<RadialMenuSettings,Double>,range:ClosedRange<Double>,step:Double,suffix:String)->some View{VStack(alignment:.leading){HStack{Text(title);Spacer();Text(String(format:"%.2f%@",store.settings[keyPath:keyPath],suffix)).foregroundStyle(MirTheme.Colors.textSecondary)};Slider(value:binding(keyPath),in:range,step:step)}}
}

private struct PanelEditor: View {
    @ObservedObject var store: RadialMenuSettingsStore
    let index: Int

    private var panel: RadialMenuPanel { store.settings.panels[index] }

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Image(systemName: panel.icon)
                TextField("Название направления", text: Binding(
                    get: { store.settings.panels[index].title },
                    set: { store.settings.panels[index].title = $0 }
                ))
                Toggle("Вкл", isOn: Binding(
                    get: { store.settings.panels[index].enabled },
                    set: { store.settings.panels[index].enabled = $0 }
                )).labelsHidden()
            }
            ForEach(store.settings.panels[index].tools.indices, id: \.self) { ti in
                HStack {
                    TextField("Название", text: Binding(
                        get: { store.settings.panels[index].tools[ti].title },
                        set: { store.settings.panels[index].tools[ti].title = $0 }
                    ))
                    TextField("Иконка", text: Binding(
                        get: { store.settings.panels[index].tools[ti].icon },
                        set: { store.settings.panels[index].tools[ti].icon = $0 }
                    ))
                    TextField("Команда", text: Binding(
                        get: { store.settings.panels[index].tools[ti].command },
                        set: { store.settings.panels[index].tools[ti].command = $0 }
                    ))
                    Button(action: { store.settings.panels[index].tools.remove(at: ti) }) {
                        Image(systemName: "trash")
                    }.buttonStyle(.plain)
                }
            }
            Button(action: {
                store.settings.panels[index].tools.append(RadialMenuTool(title: "Новый", icon: "circle", command: "custom.command"))
            }) {
                Label("Добавить инструмент", systemImage: "plus")
            }
        }
        .textFieldStyle(.roundedBorder)
    }
}
