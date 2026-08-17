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
}

@MainActor final class RadialMenuSettingsStore: ObservableObject {
    static let shared = RadialMenuSettingsStore(); @Published var settings: RadialMenuSettings { didSet { save() } }
    private let key="MIR4D.RadialMenu.Settings"
    private init(){ if let data=UserDefaults.standard.data(forKey:key),let value=try? JSONDecoder().decode(RadialMenuSettings.self,from:data){settings=value}else{settings=RadialMenuSettings()} }
    func reset(){settings=RadialMenuSettings()}; private func save(){guard let data=try? JSONEncoder().encode(settings) else{return};UserDefaults.standard.set(data,forKey:key)}
}

enum RadialMenuGeometry {
    static func normalizedAngle(_ angle:Double)->Double{var value=angle.truncatingRemainder(dividingBy:Double.pi*2);if value<0{value += Double.pi*2};return value}
    static func enabledPanels(_ settings:RadialMenuSettings)->[RadialMenuPanel]{settings.panels.filter(\.enabled)}
    static func nearestIndex(angle:Double,count:Int)->Int?{guard count>0 else{return nil};let sector=Double.pi*2/Double(count);return min(max(Int(floor((normalizedAngle(angle)+sector/2)/sector)),0),count-1)}
    static func magneticAngle(raw:Double,count:Int,strength:Double)->Double{guard count>0 else{return raw};let sector=Double.pi*2/Double(count);let center=Double(nearestIndex(angle:raw,count:count) ?? 0)*sector;let delta=atan2(sin(center-raw),cos(center-raw));return normalizedAngle(raw+delta*min(max(strength,0),1))}
    static func panelIndex(for dx:Double,dy:Double,settings s:RadialMenuSettings)->Int?{let panels=enabledPanels(s);guard !panels.isEmpty,hypot(dx,dy)>=s.deadZone else{return nil};let raw=normalizedAngle(atan2(-dy,dx)+Double.pi/2);return nearestIndex(angle:raw,count:panels.count)}
    static func toolIndex(for dx:Double,dy:Double,panel:RadialMenuPanel,settings s:RadialMenuSettings)->Int?{guard hypot(dx,dy)>=s.activationRadius,!panel.tools.isEmpty else{return nil};let raw=normalizedAngle(atan2(-dy,dx)+Double.pi/2);return nearestIndex(angle:raw,count:panel.tools.count)}
}

struct RadialMenuView:View{
    @ObservedObject var store:RadialMenuSettingsStore;let center:CGPoint;let vector:CGVector;let onToolActivated:(RadialMenuTool)->Void;let onSettings:()->Void
    private var panels:[RadialMenuPanel]{RadialMenuGeometry.enabledPanels(store.settings)}
    private var selectedPanelIndex:Int?{RadialMenuGeometry.panelIndex(for:vector.dx,dy:vector.dy,settings:store.settings)}
    private var selectedPanel:RadialMenuPanel?{guard let i=selectedPanelIndex,panels.indices.contains(i) else{return nil};return panels[i]}
    private var selectedToolIndex:Int?{guard let p=selectedPanel else{return nil};return RadialMenuGeometry.toolIndex(for:vector.dx,dy:vector.dy,panel:p,settings:store.settings)}
    private var distance:Double{hypot(vector.dx,vector.dy)}
    var body:some View{ZStack{gestureRay;centerCore;panelRing;if let p=selectedPanel{toolRing(panel:p)}}.frame(width:520,height:520).position(center).allowsHitTesting(false).transition(.opacity.combined(with:.scale(scale:0.88)))}
    private var gestureRay:some View{let l=max(distance,1);let ux=vector.dx/l,uy=vector.dy/l;return Path{path in path.move(to:CGPoint(x:260,y:260));path.addLine(to:CGPoint(x:260+vector.dx*0.62,y:260+vector.dy*0.62))}.stroke(MirTheme.Colors.selection.opacity(0.28),style:StrokeStyle(lineWidth:1.2,lineCap:.round,dash:[3,7])).overlay{Circle().fill(MirTheme.Colors.selection.opacity(0.9)).frame(width:6,height:6).position(x:260+ux*min(distance,210),y:260+uy*min(distance,210))}}
    private var centerCore:some View{VStack(spacing:3){Image(systemName:selectedToolIndex != nil ? "scope" : "cursorarrow.motionlines").font(.system(size:18,weight:.semibold));Text(selectedToolIndex != nil ? (selectedPanel?.tools[selectedToolIndex!].title ?? "") : (selectedPanel?.title ?? "МИР")).font(.system(size:10,weight:.bold)).lineLimit(1)}.foregroundStyle(.white).frame(width:68,height:68).background(.ultraThinMaterial,in:Circle()).overlay(Circle().stroke(MirTheme.Colors.accentBright.opacity(0.8),lineWidth:1)).overlay(alignment:.bottomTrailing){Button(action:onSettings){Image(systemName:"gearshape.fill").font(.system(size:9)).foregroundStyle(.white.opacity(0.7)).padding(7).background(.black.opacity(0.45),in:Circle())}.buttonStyle(.plain).allowsHitTesting(true)}}
    private var panelRing:some View{ZStack{Circle().stroke(.white.opacity(0.10),lineWidth:1).frame(width:store.settings.panelRadius*2.1,height:store.settings.panelRadius*2.1);ForEach(Array(panels.enumerated()),id:\.element.id){i,p in let selected=selectedPanelIndex==i;radialButton(title:p.title,icon:p.icon,selected:selected,position:point(radius:store.settings.panelRadius,angle:sectorAngle(index:i,count:panels.count)))}}}
    private func toolRing(panel:RadialMenuPanel)->some View{let l=max(distance,1);let origin=CGPoint(x:260+vector.dx/l*store.settings.submenuOffset,y:260+vector.dy/l*store.settings.submenuOffset);return ZStack{Path{path in path.move(to:CGPoint(x:260,y:260));path.addLine(to:origin)}.stroke(MirTheme.Colors.selection.opacity(0.32),lineWidth:1);Circle().stroke(MirTheme.Colors.selection.opacity(0.32),lineWidth:1).frame(width:116,height:116).position(origin);ForEach(Array(panel.tools.enumerated()),id:\.element.id){i,t in let selected=selectedToolIndex==i;radialButton(title:store.settings.showLabels ? t.title:"",icon:t.icon,selected:selected,position:CGPoint(x:origin.x+CGFloat(cos(sectorAngle(index:i,count:panel.tools.count)))*52,y:origin.y+CGFloat(sin(sectorAngle(index:i,count:panel.tools.count)))*52))}}}
    private func radialButton(title:String,icon:String,selected:Bool,position:CGPoint)->some View{VStack(spacing:2){Image(systemName:icon).font(.system(size:selected ? 15:12,weight:.semibold));if !title.isEmpty{Text(title).font(.system(size:8,weight:.semibold)).lineLimit(1)}}.foregroundStyle(selected ? .white:.white.opacity(0.74)).frame(width:selected ? 78:64,height:selected ? 48:42).background(selected ? MirTheme.Colors.selection.opacity(0.82):Color.black.opacity(0.42),in:RoundedRectangle(cornerRadius:12)).overlay(RoundedRectangle(cornerRadius:12).stroke(selected ? MirTheme.Colors.selection:Color.white.opacity(0.12),lineWidth:selected ? 1.5:0.8)).scaleEffect(selected ? 1.10:1).animation(.interactiveSpring(response:0.22,dampingFraction:0.74),value:selected).position(position)}
    private func sectorAngle(index:Int,count:Int)->Double{-Double.pi/2+(Double.pi*2/Double(max(count,1)))*Double(index)}
    private func point(radius:Double,angle:Double)->CGPoint{CGPoint(x:260+CGFloat(cos(angle)*radius),y:260+CGFloat(sin(angle)*radius))}
}

struct RadialMenuSettingsView:View{@ObservedObject var store:RadialMenuSettingsStore;var body:some View{NavigationStack{Form{Section("Управление"){Toggle("Радиальное меню включено",isOn:binding(\.enabled));Toggle("Средняя кнопка мыши",isOn:binding(\.middleMouseTriggerEnabled));Toggle("Удержание левой кнопки",isOn:binding(\.leftMouseHoldTriggerEnabled));Toggle("Клавиша ]",isOn:binding(\.keyboardTriggerEnabled));Toggle("Тактильная обратная связь",isOn:binding(\.hapticEnabled))};Section("Жест"){slider("Зона покоя",keyPath:\.deadZone,range:8...80,step:1,suffix:" pt");slider("Радиус панелей",keyPath:\.panelRadius,range:50...150,step:1,suffix:" pt");slider("Смещение подменю",keyPath:\.submenuOffset,range:80...220,step:1,suffix:" pt");slider("Радиус активации",keyPath: \.activationRadius,range:110...280,step:1,suffix:" pt");slider("Магнитное притяжение",keyPath:\.magneticStrength,range:0...1,step:0.01,suffix:"");slider("Устойчивость выбора",keyPath:\.magneticHysteresis,range:0...0.2,step:0.01,suffix:"");Toggle("Подписи инструментов",isOn:binding(\.showLabels))};Section("Направления"){Text("Первый уровень выражает человеческое намерение. Продолжение движения раскрывает следующий уровень.").font(.callout).foregroundStyle(.secondary)};Section("Панели и инструменты"){ForEach(store.settings.panels.indices,id:\.self){PanelEditor(store:store,index:$0)}};Section{Button("Восстановить заводскую схему"){store.reset()}}}.navigationTitle("Настройки радиального меню").formStyle(.grouped)}.frame(minWidth:560,minHeight:620)}
    private func binding<T>(_ keyPath:WritableKeyPath<RadialMenuSettings,T>)->Binding<T>{Binding(get:{store.settings[keyPath:keyPath]},set:{store.settings[keyPath:keyPath]=$0})}
    private func slider(_ title:String,keyPath:WritableKeyPath<RadialMenuSettings,Double>,range:ClosedRange<Double>,step:Double,suffix:String)->some View{VStack(alignment:.leading){HStack{Text(title);Spacer();Text(String(format:"%.2f%@",store.settings[keyPath:keyPath],suffix)).foregroundStyle(MirTheme.Colors.textSecondary)};Slider(value:binding(keyPath),in:range,step:step)}}
}

private struct PanelEditor:View{@ObservedObject var store:RadialMenuSettingsStore;let index:Int;@State private var toolsText="";var body:some View{let panel=store.settings.panels[index];VStack(alignment:.leading,spacing:8){HStack{Image(systemName:panel.icon);TextField("Название направления",text:panelTitleBinding);Toggle("Вкл",isOn:panelEnabledBinding).labelsHidden()};TextField("Инструменты через запятую",text:toolsBinding).textFieldStyle(.roundedBorder)}.onAppear{toolsText=panel.tools.map(\.title).joined(separator:", ")}}
    private var panelTitleBinding:Binding<String>{Binding(get:{store.settings.panels[index].title},set:{store.settings.panels[index].title=$0})}
    private var panelEnabledBinding:Binding<Bool>{Binding(get:{store.settings.panels[index].enabled},set:{store.settings.panels[index].enabled=$0})}
    private var toolsBinding:Binding<String>{Binding(get:{toolsText},set:{newValue in toolsText=newValue;let titles=newValue.split(separator:",").map{$0.trimmingCharacters(in:.whitespacesAndNewlines)}.filter{!$0.isEmpty};store.settings.panels[index].tools=titles.map{title in RadialMenuTool(title:title,icon:"circle",command:"custom.\(title.lowercased().replacingOccurrences(of:" ",with:"."))")}})}}
