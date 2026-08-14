// MirEngine/Tests/BRep/BRepBoolean.cpp
// Проверка BRepBooleanAPI: Fuse/Cut/Common с честными отказами.

#include "MirEngine/BRep/BRep.hpp"
#include "MirEngine/BRep/Boolean/BRepBooleanAPI.hpp"

#include <cassert>

namespace
{

[[nodiscard]] mir::BRepSolidHandle box(mir::BRepModel& model, double dx, double dy, double dz,
                                       const mir::Vector3& origin)
{
    const auto result = mir::BRepPrimAPI_MakeBox::build(model, dx, dy, dz, origin);
    assert(result.success);
    return result.solid;
}

} // namespace

int main()
{
    // ══ Fuse: непересекающиеся тела ══════════════════════════
    {
        mir::BRepModel argument;
        const auto box1 = box(argument, 2.0, 2.0, 2.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto box2 = box(tool, 2.0, 2.0, 2.0, mir::Vector3(10.0, 0.0, 0.0));

        mir::BRepModel fused;
        const auto result = mir::BRepBooleanAPI::fuse(fused, argument, box1, tool, box2);
        assert(result.success());
        assert(result.status == mir::BRepBooleanStatus::Success);
        assert(fused.rootSolids().size() == 1);
        assert(fused.rootSolids().front() == result.solid);

        const auto* solid = fused.topology().solid(result.solid);
        assert(solid && solid->shells.size() == 2);
        assert(fused.topology().vertexCount() == 16);
        assert(fused.topology().edgeCount() == 24);
        assert(fused.topology().wireCount() == 12);
        assert(fused.topology().faceCount() == 12);
        assert(fused.topology().shellCount() == 2);
        assert(fused.topology().solidCount() == 1);
        assert(fused.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(fused).ok());
    }

    // ══ Fuse: пересекающиеся box'ы — выступ по одной оси ═════════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 4.0, 2.0, 2.0, mir::Vector3(1.0, 1.0, 1.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::fuse(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.rootSolids().size() == 1);
        assert(out.rootSolids().front() == result.solid);

        const auto* solid = out.topology().solid(result.solid);
        assert(solid && solid->shells.size() == 1);
        assert(out.topology().vertexCount() == 48);
        assert(out.topology().edgeCount() == 92);
        assert(out.topology().wireCount() == 46);
        assert(out.topology().faceCount() == 46);
        assert(out.topology().shellCount() == 1);
        assert(out.topology().solidCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());

        // Объединение [0,4]³ ∪ [1,5]×[1,3]×[1,3]: диапазон [0,5]×[0,4]×[0,4].
        double minX = 1.0e9, minY = 1.0e9, minZ = 1.0e9;
        double maxX = -1.0e9, maxY = -1.0e9, maxZ = -1.0e9;
        for (const mir::BRepVertex& vertex : out.topology().vertices())
        {
            const auto* point = out.geometry().point(vertex.point);
            assert(point);
            minX = std::min(minX, point->point.x);
            minY = std::min(minY, point->point.y);
            minZ = std::min(minZ, point->point.z);
            maxX = std::max(maxX, point->point.x);
            maxY = std::max(maxY, point->point.y);
            maxZ = std::max(maxZ, point->point.z);
        }
        assert(minX == 0.0 && minY == 0.0 && minZ == 0.0);
        assert(maxX == 5.0 && maxY == 4.0 && maxZ == 4.0);
    }

    // ══ Fuse: инструмент поглощён аргументом → аргумент ════════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 2.0, 4.0, 4.0, mir::Vector3(1.0, 0.0, 0.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::fuse(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.rootSolids().size() == 1);
        assert(out.topology().vertexCount() == 16);
        assert(out.topology().edgeCount() == 28);
        assert(out.topology().wireCount() == 14);
        assert(out.topology().faceCount() == 14);
        assert(out.topology().shellCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());

        // Fuse = A: диапазон [0,4]³.
        double minX = 1.0e9, maxX = -1.0e9;
        double minY = 1.0e9, maxY = -1.0e9;
        double minZ = 1.0e9, maxZ = -1.0e9;
        for (const mir::BRepVertex& vertex : out.topology().vertices())
        {
            const auto* point = out.geometry().point(vertex.point);
            assert(point);
            minX = std::min(minX, point->point.x);
            maxX = std::max(maxX, point->point.x);
            minY = std::min(minY, point->point.y);
            maxY = std::max(maxY, point->point.y);
            minZ = std::min(minZ, point->point.z);
            maxZ = std::max(maxZ, point->point.z);
        }
        assert(minX == 0.0 && minY == 0.0 && minZ == 0.0);
        assert(maxX == 4.0 && maxY == 4.0 && maxZ == 4.0);
    }

    // ══ Fuse: выступ по двум осям («уголок») ═══════════════════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 4.0, 4.0, 2.0, mir::Vector3(1.0, 1.0, 1.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::fuse(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.rootSolids().size() == 1);
        assert(out.topology().vertexCount() == 44);
        assert(out.topology().edgeCount() == 84);
        assert(out.topology().wireCount() == 42);
        assert(out.topology().faceCount() == 42);
        assert(out.topology().shellCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());

        // Диапазон [0,5]×[0,5]×[0,4].
        double maxX = -1.0e9, maxY = -1.0e9, maxZ = -1.0e9;
        for (const mir::BRepVertex& vertex : out.topology().vertices())
        {
            const auto* point = out.geometry().point(vertex.point);
            assert(point);
            maxX = std::max(maxX, point->point.x);
            maxY = std::max(maxY, point->point.y);
            maxZ = std::max(maxZ, point->point.z);
        }
        assert(maxX == 5.0 && maxY == 5.0 && maxZ == 4.0);
    }

    // ══ Fuse: невалидные аргументы ═══════════════════════════
    {
        mir::BRepModel argument;
        const auto box1 = box(argument, 2.0, 2.0, 2.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto box2 = box(tool, 2.0, 2.0, 2.0, mir::Vector3(10.0, 0.0, 0.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::fuse(out, argument, {}, tool, box2);
        assert(!result.success());
        assert(result.status == mir::BRepBooleanStatus::InvalidArgument);
        assert(out.empty());
    }

    // ══ Cut: инструмент не пересекает аргумент → копия A ═════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 2.0, 2.0, 2.0, mir::Vector3(10.0, 0.0, 0.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::cut(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.topology().vertexCount() == 8);
        assert(out.topology().edgeCount() == 12);
        assert(out.topology().faceCount() == 6);
        assert(out.topology().shellCount() == 1);
        assert(out.topology().solidCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());
    }

    // ══ Cut: инструмент полностью внутри → полость ═══════════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 2.0, 2.0, 2.0, mir::Vector3(1.0, 1.0, 1.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::cut(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.rootSolids().size() == 1);
        assert(out.rootSolids().front() == result.solid);

        const auto* solid = out.topology().solid(result.solid);
        assert(solid && solid->shells.size() == 2);
        assert(out.topology().vertexCount() == 16);
        assert(out.topology().edgeCount() == 24);
        assert(out.topology().wireCount() == 12);
        assert(out.topology().faceCount() == 12);
        assert(out.topology().shellCount() == 2);
        assert(out.topology().solidCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());
    }

    // ══ Cut: частичное перекрытие — колодец (выступ по одной оси) ═
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 2.0, 2.0, 2.0, mir::Vector3(3.0, 1.0, 1.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::cut(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.rootSolids().size() == 1);
        assert(out.rootSolids().front() == result.solid);

        const auto* solid = out.topology().solid(result.solid);
        assert(solid && solid->shells.size() == 1);
        assert(out.topology().vertexCount() == 48);
        assert(out.topology().edgeCount() == 92);
        assert(out.topology().wireCount() == 46);
        assert(out.topology().faceCount() == 46);
        assert(out.topology().shellCount() == 1);
        assert(out.topology().solidCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());

        // Результат ограничен аргументом [0,4]³.
        double minX = 1.0e9, maxX = -1.0e9;
        double minY = 1.0e9, maxY = -1.0e9;
        double minZ = 1.0e9, maxZ = -1.0e9;
        for (const mir::BRepVertex& vertex : out.topology().vertices())
        {
            const auto* point = out.geometry().point(vertex.point);
            assert(point);
            minX = std::min(minX, point->point.x);
            maxX = std::max(maxX, point->point.x);
            minY = std::min(minY, point->point.y);
            maxY = std::max(maxY, point->point.y);
            minZ = std::min(minZ, point->point.z);
            maxZ = std::max(maxZ, point->point.z);
        }
        assert(minX == 0.0 && minY == 0.0 && minZ == 0.0);
        assert(maxX == 4.0 && maxY == 4.0 && maxZ == 4.0);
    }

    // ══ Cut: угловой вырез (выступ по двум осям) ═══════════════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 2.0, 2.0, 2.0, mir::Vector3(3.0, 3.0, 1.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::cut(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.rootSolids().size() == 1);
        assert(out.topology().vertexCount() == 36);
        assert(out.topology().edgeCount() == 68);
        assert(out.topology().wireCount() == 34);
        assert(out.topology().faceCount() == 34);
        assert(out.topology().shellCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());
    }

    // ══ Cut: инструмент касается грани аргумента изнутри ═══════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 1.0, 2.0, 2.0, mir::Vector3(3.0, 1.0, 1.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::cut(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.topology().vertexCount() == 48);
        assert(out.topology().edgeCount() == 92);
        assert(out.topology().wireCount() == 46);
        assert(out.topology().faceCount() == 46);
        assert(out.topology().shellCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());
    }

    // ══ Cut: сквозной проход инструмента → туннель ════════════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 6.0, 2.0, 2.0, mir::Vector3(-1.0, 1.0, 1.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::cut(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.rootSolids().size() == 1);
        assert(out.rootSolids().front() == result.solid);

        const auto* solid = out.topology().solid(result.solid);
        assert(solid && solid->shells.size() == 2);
        assert(out.topology().vertexCount() == 32);
        assert(out.topology().edgeCount() == 36);
        assert(out.topology().wireCount() == 12);
        assert(out.topology().faceCount() == 10);
        assert(out.topology().shellCount() == 2);
        assert(out.topology().solidCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());

        // Геометрия: тело ограничено x ∈ [0,4]; короб и отверстия
        // лежат на x ∈ {0,4}, y,z ∈ {1,3}.
        double minX = 1.0e9, maxX = -1.0e9;
        int tunnelPoints = 0;
        for (const mir::BRepVertex& vertex : out.topology().vertices())
        {
            const auto* point = out.geometry().point(vertex.point);
            assert(point);
            const mir::Vector3& p = point->point;
            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            const bool tunnelX = std::abs(p.x) < 1.0e-9 || std::abs(p.x - 4.0) < 1.0e-9;
            const bool tunnelY = std::abs(p.y - 1.0) < 1.0e-9 || std::abs(p.y - 3.0) < 1.0e-9;
            const bool tunnelZ = std::abs(p.z - 1.0) < 1.0e-9 || std::abs(p.z - 3.0) < 1.0e-9;
            if (tunnelX && tunnelY && tunnelZ)
                ++tunnelPoints;
        }
        assert(minX == 0.0 && maxX == 4.0);
        assert(tunnelPoints == 24);
    }

    // ══ Cut: сквозной проход по другой оси (Y) ════════════════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 2.0, 6.0, 2.0, mir::Vector3(1.0, -1.0, 1.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::cut(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.topology().vertexCount() == 32);
        assert(out.topology().edgeCount() == 36);
        assert(out.topology().wireCount() == 12);
        assert(out.topology().faceCount() == 10);
        assert(out.topology().shellCount() == 2);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());
    }

    // ══ Common: пересекающиеся box'ы → box пересечения ═══════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 2.0, 2.0, 2.0, mir::Vector3(1.0, 1.0, 1.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::common(out, argument, boxA, tool, boxB);
        assert(result.success());
        assert(out.rootSolids().size() == 1);
        assert(out.topology().vertexCount() == 8);
        assert(out.topology().edgeCount() == 12);
        assert(out.topology().faceCount() == 6);
        assert(out.topology().shellCount() == 1);
        assert(out.isValid());

        mir::BRepValidator validator;
        assert(validator.validate(out).ok());

        // Пересечение [1,3]^3: все точки в этом диапазоне.
        double minX = 1.0e9, minY = 1.0e9, minZ = 1.0e9;
        double maxX = -1.0e9, maxY = -1.0e9, maxZ = -1.0e9;
        for (const mir::BRepVertex& vertex : out.topology().vertices())
        {
            const auto* point = out.geometry().point(vertex.point);
            assert(point);
            minX = std::min(minX, point->point.x);
            minY = std::min(minY, point->point.y);
            minZ = std::min(minZ, point->point.z);
            maxX = std::max(maxX, point->point.x);
            maxY = std::max(maxY, point->point.y);
            maxZ = std::max(maxZ, point->point.z);
        }
        assert(minX == 1.0 && minY == 1.0 && minZ == 1.0);
        assert(maxX == 3.0 && maxY == 3.0 && maxZ == 3.0);
    }

    // ══ Common: непересекающиеся — EmptyResult ════════════════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 2.0, 2.0, 2.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 2.0, 2.0, 2.0, mir::Vector3(10.0, 0.0, 0.0));

        mir::BRepModel out;
        const auto result = mir::BRepBooleanAPI::common(out, argument, boxA, tool, boxB);
        assert(!result.success());
        assert(result.status == mir::BRepBooleanStatus::EmptyResult);
        assert(out.empty());
    }

    // ══ execute() маршрутизирует все три операции ═════════════
    {
        mir::BRepModel argument;
        const auto boxA = box(argument, 4.0, 4.0, 4.0, mir::Vector3::zero());

        mir::BRepModel tool;
        const auto boxB = box(tool, 2.0, 2.0, 2.0, mir::Vector3(10.0, 0.0, 0.0));

        mir::BRepModel fuseOut;
        const auto fuseResult = mir::BRepBooleanAPI::execute(
            fuseOut, argument, boxA, tool, boxB, mir::BRepBooleanOperation::Fuse);
        assert(fuseResult.success());

        mir::BRepModel cutOut;
        const auto cutResult = mir::BRepBooleanAPI::execute(
            cutOut, argument, boxA, tool, boxB, mir::BRepBooleanOperation::Cut);
        assert(cutResult.success());

        mir::BRepModel commonOut;
        const auto commonResult = mir::BRepBooleanAPI::execute(
            commonOut, argument, boxA, tool, boxB, mir::BRepBooleanOperation::Common);
        assert(commonResult.status == mir::BRepBooleanStatus::EmptyResult);
    }

    return 0;
}