import Foundation

// MARK: - Work plane controller (ТЗ Этап 1)

/// Владеет реестром рабочих плоскостей документа и публикует их в renderer
/// для отрисовки оверлея. Ядро (PlaneStore / PlaneFactory) живёт в MirEngine;
/// этот контроллер — только Swift-обёртка над C ABI.
@MainActor
final class WorkPlaneController {

    static let shared = WorkPlaneController()

    private let store: UnsafeMutableRawPointer?

    private init() {
        store = MirEngineCreatePlaneStore()
        MirEnginePlaneStoreAddBasePlanes(store)
    }

    /// Возвращает текущий снимок плоскостей (базовые + пользовательские).
    func snapshot() -> [MirEnginePlane] {
        guard let store else { return [] }
        let count = Int(MirEnginePlaneStoreSnapshot(store, 0, nil, nil, nil, nil, nil, nil, nil, nil, nil))
        guard count > 0 else { return [] }

        var ids = [UInt32](repeating: 0, count: count)
        var origins = [Float](repeating: 0, count: count * 3)
        var normals = [Float](repeating: 0, count: count * 3)
        var xAxes = [Float](repeating: 0, count: count * 3)
        var yAxes = [Float](repeating: 0, count: count * 3)
        var colors = [Float](repeating: 0, count: count * 3)
        var sizes = [Float](repeating: 0, count: count)
        var active = [Bool](repeating: false, count: count)
        var selected = [Bool](repeating: false, count: count)

        let n = Int(MirEnginePlaneStoreSnapshot(
            store, Int32(count),
            &ids, &origins, &normals, &xAxes, &yAxes, &colors, &sizes, &active, &selected))
        guard n > 0 else { return [] }

        var result: [MirEnginePlane] = []
        result.reserveCapacity(n)
        for i in 0 ..< n {
            let o = i * 3
            result.append(MirEnginePlane(
                id: ids[i],
                origin: (origins[o], origins[o + 1], origins[o + 2]),
                normal: (normals[o], normals[o + 1], normals[o + 2]),
                xAxis: (xAxes[o], xAxes[o + 1], xAxes[o + 2]),
                yAxis: (yAxes[o], yAxes[o + 1], yAxes[o + 2]),
                color: (colors[o], colors[o + 1], colors[o + 2]),
                size: sizes[i],
                active: active[i],
                selected: selected[i]
            ))
        }
        return result
    }

    /// Создаёт пользовательскую плоскость параллельно базовой (со смещением/углом).
    /// Возвращает id новой плоскости (0 при ошибке).
    @discardableResult
    func createOffsetPlane(basePlane: UInt32, offset: Double, angleDeg: Double) -> UInt32 {
        guard let store else { return 0 }
        return MirEnginePlaneStoreCreateOffsetPlane(store, basePlane, offset, angleDeg)
    }

    /// Публикует текущий снимок плоскостей в renderer.
    func push(to renderer: UnsafeMutableRawPointer?) {
        MirEnginePushWorkPlanes(renderer, snapshot())
    }
}
