// MirEngine/Tests/Planes/WorkPlaneTest.cpp
// =================================================================================
// Проверка подсистемы рабочих плоскостей (Этап 1 ТЗ):
//   - базовые XY/XZ/YZ;
//   - смещение / три точки / угол;
//   - локальные ↔ мировые координаты;
//   - привязка SketchDocument к плоскости.
// =================================================================================

#include "MirEngine/Planes/Plane.hpp"
#include "MirEngine/Planes/PlaneFactory.hpp"
#include "MirEngine/Planes/PlaneStore.hpp"
#include "MirEngine/Sketch/SketchDocument.hpp"

#include <cmath>
#include <cstdio>
#include <iostream>

namespace
{

int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "[FAIL] " << message << "\n";
        ++failures;
    }
}

double approx(double a, double b)
{
    return std::fabs(a - b);
}

} // namespace

int main()
{
    // ── Базовые плоскости ─────────────────────────────────────────────────
    mir::PlaneStore store;
    store.ensureBasePlanes();

    auto xy = store.find(mir::kBasePlaneXY);
    auto xz = store.find(mir::kBasePlaneXZ);
    auto yz = store.find(mir::kBasePlaneYZ);

    check(static_cast<bool>(xy), "XY plane exists");
    check(static_cast<bool>(xz), "XZ plane exists");
    check(static_cast<bool>(yz), "YZ plane exists");

    check(xy && xy->id() == mir::kBasePlaneXY, "XY plane id");
    check(xz && xz->id() == mir::kBasePlaneXZ, "XZ plane id");
    check(yz && yz->id() == mir::kBasePlaneYZ, "YZ plane id");

    check(xy && approx(xy->normal().z, 1.0) < 1e-9, "XY normal = +Z");
    check(xz && approx(xz->normal().y, 1.0) < 1e-9, "XZ normal = +Y");
    check(yz && approx(yz->normal().x, 1.0) < 1e-9, "YZ normal = +X");

    // Системные плоскости нельзя удалить (ТЗ 3.1).
    check(!store.remove(mir::kBasePlaneXY), "base plane not deletable");

    // ── Плоскость со смещением ───────────────────────────────────────────
    auto offset = mir::PlaneFactory::createOffset(*xy, 25.0);
    check(store.add(offset), "offset plane added");
    check(offset->deletable(), "user plane deletable");
    check(approx(offset->origin().z, 25.0) < 1e-9, "offset plane origin.z = 25");
    check(approx(offset->normal().z, 1.0) < 1e-9, "offset plane parallel to XY");

    // ── Плоскость по трём точкам ─────────────────────────────────────────
    mir::Plane threePt = *mir::PlaneFactory::createThreePoint(
        mir::Point3{0.0, 0.0, 0.0},
        mir::Point3{10.0, 0.0, 0.0},
        mir::Point3{0.0, 10.0, 5.0});
    check(approx(threePt.normal().length(), 1.0) < 1e-9, "three-point normal normalized");

    // ── Плоскость под углом ──────────────────────────────────────────────
    auto angled = mir::PlaneFactory::createAngle(*xy, mir::Vector3{1.0, 0.0, 0.0}, 45.0);
    check(approx(angled->angleDeg(), 45.0) < 1e-9, "angle plane stores 45 deg");
    // После поворота вокруг X нормаль (0,0,1) → (0,-sin,cos): |z| = cos(45).
    check(approx(std::fabs(angled->normal().z), std::cos(0.7853981633974483)) < 1e-6,
          "angle plane normal rotated");

    // ── Координатные преобразования плоскости ───────────────────────────
    {
        double wx = 25.0, wy = 40.0;
        mir::Point3 world = xy->toWorld(wx, wy);
        check(approx(world.x, 25.0) < 1e-9 && approx(world.y, 40.0) < 1e-9 &&
                  approx(world.z, 0.0) < 1e-9,
              "XY toWorld(25,40) = (25,40,0)");

        double lx = 0.0, ly = 0.0;
        xy->toLocal(world, lx, ly);
        check(approx(lx, 25.0) < 1e-9 && approx(ly, 40.0) < 1e-9,
              "XY toLocal round-trip");

        // Смещённая плоскость: локальная (0,0) → мировая origin (z=25).
        mir::Point3 oworld = offset->toWorld(0.0, 0.0);
        check(approx(oworld.z, 25.0) < 1e-9, "offset toWorld(0,0) at z=25");

        double olx = 0.0, oly = 0.0;
        offset->toLocal(oworld, olx, oly);
        check(approx(olx, 0.0) < 1e-9 && approx(oly, 0.0) < 1e-9,
              "offset toLocal round-trip");
    }

    // ── Привязка SketchDocument к плоскости (ТЗ 25) ─────────────────────
    {
        mir::SketchDocument doc("Sketch 01");
        doc.setPlane(xy->id(), xy->localToWorld());
        check(doc.planeId() == mir::kBasePlaneXY, "sketch anchored to XY");

        mir::Point3 sw = doc.toWorld(25.0, 40.0);
        check(approx(sw.x, 25.0) < 1e-9 && approx(sw.y, 40.0) < 1e-9 &&
                  approx(sw.z, 0.0) < 1e-9,
              "sketch toWorld(25,40) = (25,40,0)");

        double slx = 0.0, sly = 0.0;
        doc.toLocal(sw, slx, sly);
        check(approx(slx, 25.0) < 1e-9 && approx(sly, 40.0) < 1e-9,
              "sketch toLocal round-trip");
    }

    if (failures == 0)
    {
        std::cout << "WorkPlaneTest: ALL PASSED\n";
        return 0;
    }
    std::cerr << "WorkPlaneTest: " << failures << " FAILURE(S)\n";
    return 1;
}
