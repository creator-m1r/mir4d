#pragma once

// MirEngine/BRep/Boolean/BRepBooleanAPI.hpp

#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Core/BRepHandles.hpp"
#include "MirEngine/BRep/Builders/BRepBuilderAPI.hpp"
#include "MirEngine/BRep/Builders/BRepPrimAPI_MakeBox.hpp"
#include "MirEngine/BRep/Validator/BRepValidator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace mir
{

enum class BRepBooleanOperation : std::uint8_t
{
    Fuse = 0,
    Cut,
    Common
};

enum class BRepBooleanStatus : std::uint8_t
{
    NotImplemented = 0,
    Success,
    InvalidArgument,
    EmptyResult,
    Failed
};

struct BRepBooleanResult
{
    BRepBooleanStatus status{BRepBooleanStatus::NotImplemented};
    BRepSolidHandle solid{};
    std::string message{"BRep boolean kernel is not implemented yet"};

    [[nodiscard]] bool success() const noexcept
    {
        return status == BRepBooleanStatus::Success && solid.valid();
    }
};

class BRepBooleanAPI
{
public:
    [[nodiscard]] static BRepBooleanResult execute(
        BRepModel& outModel,
        const BRepModel& argumentModel,
        BRepSolidHandle argumentSolid,
        const BRepModel& toolModel,
        BRepSolidHandle toolSolid,
        BRepBooleanOperation operation)
    {
        switch (operation)
        {
            case BRepBooleanOperation::Fuse:
                return fuse(outModel, argumentModel, argumentSolid, toolModel, toolSolid);
            case BRepBooleanOperation::Cut:
                return cut(outModel, argumentModel, argumentSolid, toolModel, toolSolid);
            case BRepBooleanOperation::Common:
                return common(outModel, argumentModel, argumentSolid, toolModel, toolSolid);
        }
        return {};
    }

    /// Объединение двух непересекающихся тел: результат — новый solid,
    /// содержащий оболочки аргумента и инструмента. Пересекающиеся тела
    /// возвращают NotImplemented (требуется kernel разбиения граней).
    /// Операция транзакционна: при сбое outModel откатывается к исходному состоянию.
    [[nodiscard]] static BRepBooleanResult fuse(
        BRepModel& outModel,
        const BRepModel& argumentModel,
        BRepSolidHandle argumentSolid,
        const BRepModel& toolModel,
        BRepSolidHandle toolSolid)
    {
        BRepBooleanResult result = validateOperands(argumentModel, argumentSolid, toolModel, toolSolid);
        if (result.status != BRepBooleanStatus::Success)
            return result;

        const Bounds3 argumentBounds = solidBounds(argumentModel, argumentSolid);
        const Bounds3 toolBounds = solidBounds(toolModel, toolSolid);
        if (!argumentBounds.valid || !toolBounds.valid)
        {
            result.status = BRepBooleanStatus::Failed;
            result.message = "Fuse could not traverse solid boundary loops";
            return result;
        }
        if (overlaps(argumentBounds, toolBounds))
        {
            // Пересекающиеся axis-aligned box'ы: точное объединение через
            // замощение граней по общей сетке координат (разбиение граней
            // на клетки; клетки, полностью поглощённые другим телом, не
            // строятся; рёбра и вершины дедуплицируются по сетке).
            if (isAxisAlignedBox(argumentModel, argumentSolid) &&
                isAxisAlignedBox(toolModel, toolSolid) &&
                hasNoInnerWires(argumentModel, argumentSolid) &&
                hasNoInnerWires(toolModel, toolSolid))
            {
                return fuseOverlappingBoxes(outModel, argumentBounds, toolBounds);
            }
            // Повёрнутые box'ы: обобщённый kernel разбиения граней.
            if (isBox(argumentModel, argumentSolid) &&
                isBox(toolModel, toolSolid) &&
                hasNoInnerWires(argumentModel, argumentSolid) &&
                hasNoInnerWires(toolModel, toolSolid))
            {
                return fuseOrientedBoxes(outModel, argumentModel, argumentSolid,
                                         toolModel, toolSolid);
            }
            result.status = BRepBooleanStatus::NotImplemented;
            result.message = "Fuse of overlapping non-box solids requires the boundary intersection kernel (next milestone)";
            return result;
        }

        const auto checkpoint = outModel.checkpoint();
        const BRepIndexMap argumentMap = copyModel(outModel, argumentModel);
        const BRepIndexMap toolMap = copyModel(outModel, toolModel);

        const BRepSolid* argumentRecord = argumentModel.topology().solid(argumentSolid);
        const BRepSolid* toolRecord = toolModel.topology().solid(toolSolid);
        std::vector<BRepShellHandle> fusedShells;
        fusedShells.reserve(argumentRecord->shells.size() + toolRecord->shells.size());
        for (BRepShellHandle shell : argumentRecord->shells)
            fusedShells.push_back(remap(argumentMap.shells, shell));
        for (BRepShellHandle shell : toolRecord->shells)
            fusedShells.push_back(remap(toolMap.shells, shell));

        BRepBuilderAPI builder(outModel);
        const BRepSolidHandle fused = builder.makeSolid(fusedShells);
        if (!fused.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Fuse failed to assemble the merged solid";
            return result;
        }

        if (!validated(outModel))
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Fuse produced an invalid B-Rep model; operation rolled back";
            return result;
        }

        result.status = BRepBooleanStatus::Success;
        result.solid = fused;
        result.message = "Fuse completed (disjoint solids merged)";
        return result;
    }

    /// Разность A \ B.
    ///
    /// Реализованные случаи:
    ///   • B не пересекает A          → копия A (результат математически равен A);
    ///   • B целиком строго внутри A  → A с полостью: внешняя оболочка A +
    ///     внутренняя оболочка B (грани с обратной ориентацией). Полость
    ///     изолирована от внешнего пространства — это точный топологический
    ///     результат для замкнутого тела;
    ///   • B пересекает границу A     → NotImplemented (требуется kernel
    ///     разбиения граней).
    /// Операция транзакционна: при сбое outModel откатывается к исходному состоянию.
    [[nodiscard]] static BRepBooleanResult cut(
        BRepModel& outModel,
        const BRepModel& argumentModel,
        BRepSolidHandle argumentSolid,
        const BRepModel& toolModel,
        BRepSolidHandle toolSolid)
    {
        BRepBooleanResult result = validateOperands(argumentModel, argumentSolid, toolModel, toolSolid);
        if (result.status != BRepBooleanStatus::Success)
            return result;

        const Bounds3 argumentBounds = solidBounds(argumentModel, argumentSolid);
        const Bounds3 toolBounds = solidBounds(toolModel, toolSolid);
        if (!argumentBounds.valid || !toolBounds.valid)
        {
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut could not traverse solid boundary loops";
            return result;
        }

        // A \ B = A, когда B не пересекает A.
        if (!overlaps(argumentBounds, toolBounds))
        {
            const CopiedSolid copy = copySolidAsNewRoot(outModel, argumentModel, argumentSolid);
            if (!copy.solid.valid())
            {
                result.status = BRepBooleanStatus::Failed;
                result.message = "Cut failed to copy the argument solid";
                return result;
            }
            result.status = BRepBooleanStatus::Success;
            result.solid = copy.solid;
            result.message = "Cut: tool does not intersect argument; argument returned";
            return result;
        }

        // Полное погружение инструмента: полость с внутренней оболочкой.
        if (fullyContained(toolBounds, argumentBounds))
            return buildCavity(outModel, argumentModel, argumentSolid, toolModel, toolSolid);

        // Сквозной проход инструмента: туннель с отверстиями на торцах.
        const int passAxis = passThroughAxis(argumentBounds, toolBounds);
        if (passAxis >= 0)
        {
            if (!isAxisAlignedBox(argumentModel, argumentSolid) ||
                !isAxisAlignedBox(toolModel, toolSolid))
            {
                result.status = BRepBooleanStatus::NotImplemented;
                result.message = "Cut-through requires axis-aligned box solids";
                return result;
            }
            if (!hasNoInnerWires(toolModel, toolSolid))
            {
                result.status = BRepBooleanStatus::NotImplemented;
                result.message = "Cut-through tool must not contain inner wires";
                return result;
            }
            return buildThroughHole(outModel, argumentModel, argumentSolid,
                                    argumentBounds, toolBounds, passAxis);
        }

        // Частичное перекрытие границ: вырез W = A ∩ B; клетки граней A
        // вне W (нестрогое поглощение — дыры открываются и на гранях,
        // совпадающих с W) + стенки сторон W, не совпадающих с границами A.
        // Единая оболочка: кольца клеток сшиваются со стенками по рёбрам.
        if (isAxisAlignedBox(argumentModel, argumentSolid) &&
            isAxisAlignedBox(toolModel, toolSolid) &&
            hasNoInnerWires(argumentModel, argumentSolid) &&
            hasNoInnerWires(toolModel, toolSolid))
        {
            return cutPartialBoxes(outModel, argumentBounds, toolBounds);
        }

        // Прочие тела: требует обобщённого kernel разбиения граней.
        result.status = BRepBooleanStatus::NotImplemented;
        result.message = "Cut with boundary-crossing tool requires the face splitting kernel (next milestone)";
        return result;
    }

    /// Пересечение A ∩ B.
    ///
    /// Реализованные случаи:
    ///   • B не пересекает A      → EmptyResult;
    ///   • оба тела — box'ы,
    ///     выровненные по осям    → новый box по пересечению AABB (точный
    ///     результат для прямоугольных параллелепипедов);
    ///   • прочие тела            → NotImplemented.
    [[nodiscard]] static BRepBooleanResult common(
        BRepModel& outModel,
        const BRepModel& argumentModel,
        BRepSolidHandle argumentSolid,
        const BRepModel& toolModel,
        BRepSolidHandle toolSolid)
    {
        BRepBooleanResult result = validateOperands(argumentModel, argumentSolid, toolModel, toolSolid);
        if (result.status != BRepBooleanStatus::Success)
            return result;

        const Bounds3 argumentBounds = solidBounds(argumentModel, argumentSolid);
        const Bounds3 toolBounds = solidBounds(toolModel, toolSolid);
        if (!argumentBounds.valid || !toolBounds.valid)
        {
            result.status = BRepBooleanStatus::Failed;
            result.message = "Common could not traverse solid boundary loops";
            return result;
        }

        if (!overlaps(argumentBounds, toolBounds))
        {
            result.status = BRepBooleanStatus::EmptyResult;
            result.message = "Common: solids do not intersect; result is empty";
            return result;
        }

        if (!isAxisAlignedBox(argumentModel, argumentSolid) ||
            !isAxisAlignedBox(toolModel, toolSolid))
        {
            result.status = BRepBooleanStatus::NotImplemented;
            result.message = "Common currently supports axis-aligned boxes only";
            return result;
        }

        const Bounds3 intersection = intersectBounds(argumentBounds, toolBounds);
        const BRepSolidHandle solid = makeBoxFromBounds(outModel, intersection);
        if (!solid.valid())
        {
            result.status = BRepBooleanStatus::Failed;
            result.message = "Common failed to build the intersection box";
            return result;
        }

        result.status = BRepBooleanStatus::Success;
        result.solid = solid;
        result.message = "Common completed (axis-aligned box intersection)";
        return result;
    }

private:
    // ── Проверка операндов ────────────────────────────────────

    [[nodiscard]] static BRepBooleanResult validateOperands(
        const BRepModel& argumentModel,
        BRepSolidHandle argumentSolid,
        const BRepModel& toolModel,
        BRepSolidHandle toolSolid)
    {
        BRepBooleanResult result{};
        result.status = BRepBooleanStatus::Success;
        if (!argumentSolid.valid() || !toolSolid.valid())
        {
            result.status = BRepBooleanStatus::InvalidArgument;
            result.message = "Boolean requires valid argument and tool solids";
            return result;
        }
        const BRepSolid* argumentRecord = argumentModel.topology().solid(argumentSolid);
        const BRepSolid* toolRecord = toolModel.topology().solid(toolSolid);
        if (!argumentRecord || !toolRecord)
        {
            result.status = BRepBooleanStatus::InvalidArgument;
            result.message = "Boolean solids are not present in the given models";
            return result;
        }
        if (argumentRecord->shells.empty() || toolRecord->shells.empty())
        {
            result.status = BRepBooleanStatus::InvalidArgument;
            result.message = "Boolean solids must contain at least one shell";
            return result;
        }
        return result;
    }

    // ── Маппинг индексов при копировании ──────────────────────

    struct BRepIndexMap
    {
        std::vector<BRepIndex> points;
        std::vector<BRepIndex> curves;
        std::vector<BRepIndex> surfaces;
        std::vector<BRepIndex> vertices;
        std::vector<BRepIndex> edges;
        std::vector<BRepIndex> wires;
        std::vector<BRepIndex> faces;
        std::vector<BRepIndex> shells;
    };

    [[nodiscard]] static BRepIndex remapIndex(const std::vector<BRepIndex>& map, BRepIndex index) noexcept
    {
        if (!isValidBRepIndex(index))
            return index;
        if (map.empty() || static_cast<std::size_t>(index) >= map.size())
            return InvalidBRepIndex;
        return map[static_cast<std::size_t>(index)];
    }

    template <BRepShapeType TypeTag>
    [[nodiscard]] static BRepTopoHandle<TypeTag> remap(
        const std::vector<BRepIndex>& map,
        BRepTopoHandle<TypeTag> handle) noexcept
    {
        return BRepTopoHandle<TypeTag>{remapIndex(map, handle.index)};
    }

    [[nodiscard]] static BRepPointHandle remapPoint(const std::vector<BRepIndex>& map, BRepPointHandle handle) noexcept
    { return BRepPointHandle{remapIndex(map, handle.index)}; }

    [[nodiscard]] static BRepCurveHandle remapCurve(const std::vector<BRepIndex>& map, BRepCurveHandle handle) noexcept
    { return BRepCurveHandle{remapIndex(map, handle.index)}; }

    [[nodiscard]] static BRepSurfaceHandle remapSurface(const std::vector<BRepIndex>& map, BRepSurfaceHandle handle) noexcept
    { return BRepSurfaceHandle{remapIndex(map, handle.index)}; }

    // ── Копирование модели в out (solid-записи и root-привязки не копируются) ──
    // Порядок копирования фиксирован: геометрия, затем топология сверху вниз.
    // Обратные ссылки владельцев (wire→face, face→shell) заполняются после
    // копирования соответствующих слоёв. При copyShells == false записи
    // оболочек не копируются, а владельцы граней остаются невалидными —
    // грани затем собираются в новые оболочки.

    [[nodiscard]] static BRepIndexMap copyModel(
        BRepModel& out,
        const BRepModel& src,
        bool copyShells = true)
    {
        BRepIndexMap map;
        const BRepGeometryStore& geometry = src.geometry();
        for (const BRepPointGeometry& item : geometry.points())
            map.points.push_back(out.geometry().addPoint(item).index);
        for (const BRepCurveGeometry& item : geometry.curves())
            map.curves.push_back(out.geometry().addCurve(item).index);
        for (const BRepSurfaceGeometry& item : geometry.surfaces())
            map.surfaces.push_back(out.geometry().addSurface(item).index);

        const BRepTopologyStore& topology = src.topology();
        for (const BRepVertex& item : topology.vertices())
        {
            BRepVertex copy = item;
            copy.point = remapPoint(map.points, item.point);
            map.vertices.push_back(out.topology().addVertex(copy).index);
        }
        for (const BRepEdge& item : topology.edges())
        {
            BRepEdge copy = item;
            copy.curve = remapCurve(map.curves, item.curve);
            copy.start = remap(map.vertices, item.start);
            copy.end = remap(map.vertices, item.end);
            map.edges.push_back(out.topology().addEdge(copy).index);
        }
        for (const BRepWire& item : topology.wires())
        {
            BRepWire copy = item;
            for (BRepOrientedEdge& oriented : copy.edges)
                oriented.edge = remap(map.edges, oriented.edge);
            copy.ownerFace = {};
            map.wires.push_back(out.topology().addWire(copy).index);
        }
        for (const BRepFace& item : topology.faces())
        {
            BRepFace copy = item;
            copy.surface = remapSurface(map.surfaces, item.surface);
            copy.outer = {remap(map.wires, item.outer.wire), item.outer.orientation};
            copy.inners.clear();
            for (const BRepOrientedWire& inner : item.inners)
                copy.inners.push_back({remap(map.wires, inner.wire), inner.orientation});
            copy.ownerShell = {};
            map.faces.push_back(out.topology().addFace(copy).index);
        }
        if (copyShells)
        {
            for (const BRepShell& item : topology.shells())
            {
                BRepShell copy = item;
                for (BRepOrientedFace& oriented : copy.faces)
                    oriented.face = remap(map.faces, oriented.face);
                copy.ownerSolid = {};
                map.shells.push_back(out.topology().addShell(copy).index);
            }
        }

        for (std::size_t i = 0; i < topology.wires().size(); ++i)
        {
            auto* wire = out.topology().wire(BRepWireHandle{map.wires[i]});
            wire->ownerFace = remap(map.faces, topology.wires()[i].ownerFace);
        }
        if (copyShells)
        {
            for (std::size_t i = 0; i < topology.faces().size(); ++i)
            {
                auto* face = out.topology().face(BRepFaceHandle{map.faces[i]});
                face->ownerShell = remap(map.shells, topology.faces()[i].ownerShell);
            }
        }
        return map;
    }

    // ── Копирование solid как нового root-объекта ─────────────

    struct CopiedSolid
    {
        BRepIndexMap map;
        BRepSolidHandle solid{};
    };

    [[nodiscard]] static CopiedSolid copySolidAsNewRoot(
        BRepModel& out,
        const BRepModel& src,
        BRepSolidHandle handle)
    {
        CopiedSolid result;
        const BRepSolid* record = src.topology().solid(handle);
        if (!record)
            return result;
        result.map = copyModel(out, src);
        BRepBuilderAPI builder(out);
        std::vector<BRepShellHandle> shells;
        shells.reserve(record->shells.size());
        for (BRepShellHandle shell : record->shells)
            shells.push_back(remap(result.map.shells, shell));
        result.solid = builder.makeSolid(shells);
        return result;
    }

    // ── Границы solid для проверки пересечений ────────────────

    struct Bounds3
    {
        bool valid{false};
        double minX{std::numeric_limits<double>::infinity()};
        double minY{std::numeric_limits<double>::infinity()};
        double minZ{std::numeric_limits<double>::infinity()};
        double maxX{-std::numeric_limits<double>::infinity()};
        double maxY{-std::numeric_limits<double>::infinity()};
        double maxZ{-std::numeric_limits<double>::infinity()};
    };

    static void include(Bounds3& bounds, const Vector3& point) noexcept
    {
        if (!point.isFinite())
            return;
        bounds.valid = true;
        bounds.minX = std::min(bounds.minX, point.x);
        bounds.minY = std::min(bounds.minY, point.y);
        bounds.minZ = std::min(bounds.minZ, point.z);
        bounds.maxX = std::max(bounds.maxX, point.x);
        bounds.maxY = std::max(bounds.maxY, point.y);
        bounds.maxZ = std::max(bounds.maxZ, point.z);
    }

    static void includeWireBounds(const BRepModel& model, const BRepOrientedWire& wire, Bounds3& bounds) noexcept
    {
        const BRepWire* record = model.topology().wire(wire.wire);
        if (!record)
            return;
        for (const BRepOrientedEdge& oriented : record->edges)
        {
            const BRepEdge* edge = model.topology().edge(oriented.edge);
            if (!edge)
                continue;
            const BRepVertex* start = model.topology().vertex(edge->start);
            const BRepVertex* end = model.topology().vertex(edge->end);
            if (start)
            {
                const BRepPointGeometry* point = model.geometry().point(start->point);
                if (point)
                    include(bounds, point->point);
            }
            if (end)
            {
                const BRepPointGeometry* point = model.geometry().point(end->point);
                if (point)
                    include(bounds, point->point);
            }
        }
    }

    [[nodiscard]] static Bounds3 solidBounds(const BRepModel& model, BRepSolidHandle solid) noexcept
    {
        Bounds3 bounds;
        const BRepSolid* record = model.topology().solid(solid);
        if (!record)
            return bounds;
        for (BRepShellHandle shellHandle : record->shells)
        {
            const BRepShell* shell = model.topology().shell(shellHandle);
            if (!shell)
                return {};
            for (const BRepOrientedFace& oriented : shell->faces)
            {
                const BRepFace* face = model.topology().face(oriented.face);
                if (!face)
                    return {};
                includeWireBounds(model, face->outer, bounds);
                for (const BRepOrientedWire& inner : face->inners)
                    includeWireBounds(model, inner, bounds);
            }
        }
        return bounds;
    }

    // Пересечение со строго положительным объёмом (простое касание допускается).
    [[nodiscard]] static bool overlaps(const Bounds3& a, const Bounds3& b) noexcept
    {
        if (!a.valid || !b.valid)
            return true;
        return a.minX < b.maxX && b.minX < a.maxX &&
               a.minY < b.maxY && b.minY < a.maxY &&
               a.minZ < b.maxZ && b.minZ < a.maxZ;
    }

    // Строгое вложение: inner не касается границ outer ни по одной оси.
    [[nodiscard]] static bool fullyContained(const Bounds3& inner, const Bounds3& outer) noexcept
    {
        if (!inner.valid || !outer.valid)
            return false;
        return inner.minX > outer.minX && inner.minY > outer.minY && inner.minZ > outer.minZ &&
               inner.maxX < outer.maxX && inner.maxY < outer.maxY && inner.maxZ < outer.maxZ;
    }

    [[nodiscard]] static Bounds3 intersectBounds(const Bounds3& a, const Bounds3& b) noexcept
    {
        Bounds3 result;
        result.valid = true;
        result.minX = std::max(a.minX, b.minX);
        result.minY = std::max(a.minY, b.minY);
        result.minZ = std::max(a.minZ, b.minZ);
        result.maxX = std::min(a.maxX, b.maxX);
        result.maxY = std::min(a.maxY, b.maxY);
        result.maxZ = std::min(a.maxZ, b.maxZ);
        return result;
    }

    // ── Проверка «тело — параллелепипед, выровненный по осям» ──

    [[nodiscard]] static bool isAxisAlignedBox(const BRepModel& model, BRepSolidHandle solid) noexcept
    {
        const BRepSolid* record = model.topology().solid(solid);
        if (!record)
            return false;

        bool seen[6]{false, false, false, false, false, false}; // +X, -X, +Y, -Y, +Z, -Z
        std::size_t faceCount = 0;
        for (BRepShellHandle shellHandle : record->shells)
        {
            const BRepShell* shell = model.topology().shell(shellHandle);
            if (!shell)
                return false;
            for (const BRepOrientedFace& oriented : shell->faces)
            {
                const BRepFace* face = model.topology().face(oriented.face);
                if (!face)
                    return false;
                const BRepSurfaceGeometry* surface = model.geometry().surface(face->surface);
                if (!surface || surface->type != BRepSurfaceType::Plane)
                    return false;

                const Vector3 n = surface->plane.normal;
                const double ax = std::abs(n.x);
                const double ay = std::abs(n.y);
                const double az = std::abs(n.z);
                int axis = -1;
                bool positive = false;
                if (ax > 0.999999 && ay < 1.0e-9 && az < 1.0e-9)
                {
                    axis = 0;
                    positive = n.x > 0.0;
                }
                else if (ay > 0.999999 && ax < 1.0e-9 && az < 1.0e-9)
                {
                    axis = 1;
                    positive = n.y > 0.0;
                }
                else if (az > 0.999999 && ax < 1.0e-9 && ay < 1.0e-9)
                {
                    axis = 2;
                    positive = n.z > 0.0;
                }
                else
                {
                    return false;
                }

                const int index = axis * 2 + (positive ? 0 : 1);
                if (seen[index])
                    return false;
                seen[index] = true;
                ++faceCount;
            }
        }
        return faceCount == 6 &&
               seen[0] && seen[1] && seen[2] && seen[3] && seen[4] && seen[5];
    }

    // ── Построение box'а по границам ───────────────────────────

    [[nodiscard]] static BRepSolidHandle makeBoxFromBounds(BRepModel& out, const Bounds3& bounds) noexcept
    {
        const double dx = bounds.maxX - bounds.minX;
        const double dy = bounds.maxY - bounds.minY;
        const double dz = bounds.maxZ - bounds.minZ;
        if (!(dx > 0.0) || !(dy > 0.0) || !(dz > 0.0) ||
            !std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(dz))
            return {};

        const auto result = BRepPrimAPI_MakeBox::build(
            out, dx, dy, dz, Vector3(bounds.minX, bounds.minY, bounds.minZ));
        return result.success ? result.solid : BRepSolidHandle{};
    }

    // ── Полость (cut при полном погружении инструмента) ────────

    [[nodiscard]] static BRepBooleanResult buildCavity(
        BRepModel& outModel,
        const BRepModel& argumentModel,
        BRepSolidHandle argumentSolid,
        const BRepModel& toolModel,
        BRepSolidHandle toolSolid)
    {
        BRepBooleanResult result{};
        const auto checkpoint = outModel.checkpoint();
        const BRepIndexMap argumentMap = copyModel(outModel, argumentModel, false);
        const BRepIndexMap toolMap = copyModel(outModel, toolModel, false);

        const BRepSolid* argumentRecord = argumentModel.topology().solid(argumentSolid);
        const BRepSolid* toolRecord = toolModel.topology().solid(toolSolid);

        BRepBuilderAPI builder(outModel);

        // Внешняя оболочка — грани аргумента.
        std::vector<BRepOrientedFace> outerFaces;
        for (BRepShellHandle shellHandle : argumentRecord->shells)
        {
            const BRepShell* shell = argumentModel.topology().shell(shellHandle);
            if (!shell)
            {
                outModel.rollback(checkpoint);
                result.status = BRepBooleanStatus::Failed;
                result.message = "Cut: argument shell is missing";
                return result;
            }
            for (const BRepOrientedFace& oriented : shell->faces)
                outerFaces.push_back(
                    {remap(argumentMap.faces, oriented.face), oriented.orientation});
        }
        if (outerFaces.empty())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut: argument has no faces to form the outer shell";
            return result;
        }
        const BRepShellHandle outerShell = builder.makeShell(outerFaces, true);
        if (!outerShell.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut failed to build the outer shell";
            return result;
        }

        // Внутренняя оболочка — грани инструмента с обратной ориентацией.
        std::vector<BRepOrientedFace> innerFaces;
        for (BRepShellHandle shellHandle : toolRecord->shells)
        {
            const BRepShell* shell = toolModel.topology().shell(shellHandle);
            if (!shell)
            {
                outModel.rollback(checkpoint);
                result.status = BRepBooleanStatus::Failed;
                result.message = "Cut: tool shell is missing";
                return result;
            }
            for (const BRepOrientedFace& oriented : shell->faces)
                innerFaces.push_back(
                    {remap(toolMap.faces, oriented.face), mir::reversed(oriented.orientation)});
        }
        if (innerFaces.empty())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut: tool has no faces to form the cavity";
            return result;
        }
        const BRepShellHandle innerShell = builder.makeShell(innerFaces, true);
        if (!innerShell.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut failed to build the cavity shell";
            return result;
        }

        const BRepSolidHandle solid = builder.makeSolid({outerShell, innerShell});
        if (!solid.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut failed to assemble the cavity solid";
            return result;
        }

        if (!validated(outModel))
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut produced an invalid B-Rep model; operation rolled back";
            return result;
        }

        result.status = BRepBooleanStatus::Success;
        result.solid = solid;
        result.message = "Cut completed (contained tool forms a cavity shell)";
        return result;
    }

    [[nodiscard]] static bool validated(const BRepModel& model) noexcept
    {
        const BRepValidator validator;
        const BRepValidationReport report = validator.validate(model);
        return report.ok() && model.isValid();
    }

    // ── Объединение пересекающихся box'ов (замощение граней) ───
    // Общая сетка координат строится из границ обоих тел; каждая из 12
    // граней замащивается клетками-прямоугольниками. Клетка, полностью
    // лежащая внутри другого тела (плоскость внутри его диапазона по оси
    // и клетка внутри его прямоугольника), не строится. Вершины и рёбра
    // дедуплицируются по сетке — соседние клетки и грани корректно
    // разделяют общую топологию.

    [[nodiscard]] static std::vector<double> coordinateGrid(double a0, double a1,
                                                            double b0, double b1)
    {
        std::vector<double> values{a0, a1, b0, b1};
        std::sort(values.begin(), values.end());
        values.erase(std::unique(values.begin(), values.end()), values.end());
        return values;
    }

    [[nodiscard]] static BRepVertexHandle gridVertex(
        BRepBuilderAPI& builder,
        std::vector<std::pair<Vector3, BRepVertexHandle>>& cache,
        const Vector3& point)
    {
        for (const auto& entry : cache)
        {
            if (entry.first.x == point.x && entry.first.y == point.y &&
                entry.first.z == point.z)
                return entry.second;
        }
        const BRepVertexHandle vertex = builder.makeVertex(point);
        if (vertex.valid())
            cache.emplace_back(point, vertex);
        return vertex;
    }

    // Ребро сетки: дедупликация по паре вершин; ориентация выбирается по
    // направлению ребра в хранилище (обход p → q).
    [[nodiscard]] static BRepOrientedEdge gridEdge(
        BRepModel& model,
        BRepBuilderAPI& builder,
        std::map<std::pair<BRepIndex, BRepIndex>, BRepEdgeHandle>& cache,
        BRepVertexHandle p,
        BRepVertexHandle q)
    {
        const BRepIndex a = std::min(p.index, q.index);
        const BRepIndex b = std::max(p.index, q.index);
        const auto found = cache.find({a, b});
        if (found != cache.end())
        {
            const BRepEdge* edge = model.topology().edge(found->second);
            if (!edge)
                return {};
            const bool forward = edge->start == p && edge->end == q;
            return {found->second, forward ? BRepOrientation::Forward
                                           : BRepOrientation::Reversed};
        }
        const BRepEdgeHandle edge = builder.makeEdgeLine(p, q);
        if (!edge.valid())
            return {};
        cache.emplace(std::make_pair(a, b), edge);
        return {edge, BRepOrientation::Forward};
    }

    // Замощение граней одного box'а клетками на плоскости (axis, plane).
    // strictly: плоскость сравнивается строго — совпадающие границы
    // другого тела не считаются поглощением.
    [[nodiscard]] static bool addBoxFaceTiles(
        BRepModel& outModel,
        BRepBuilderAPI& builder,
        const std::array<std::vector<double>, 3>& grids,
        std::vector<std::pair<Vector3, BRepVertexHandle>>& vertexCache,
        std::map<std::pair<BRepIndex, BRepIndex>, BRepEdgeHandle>& edgeCache,
        std::vector<BRepFaceHandle>& faces,
        const Bounds3& src,
        const Bounds3& other,
        bool strictly)
    {
        for (int axis = 0; axis < 3; ++axis)
        {
            const int axisA = (axis + 1) % 3;
            const int axisB = (axis + 2) % 3;
            const double minP = coordinate(src, axis, true);
            const double maxP = coordinate(src, axis, false);
            const double minA = coordinate(src, axisA, true);
            const double maxA = coordinate(src, axisA, false);
            const double minB = coordinate(src, axisB, true);
            const double maxB = coordinate(src, axisB, false);
            const double otherMinP = coordinate(other, axis, true);
            const double otherMaxP = coordinate(other, axis, false);
            const double otherMinA = coordinate(other, axisA, true);
            const double otherMaxA = coordinate(other, axisA, false);
            const double otherMinB = coordinate(other, axisB, true);
            const double otherMaxB = coordinate(other, axisB, false);

            for (int side = 0; side < 2; ++side)
            {
                const double plane = (side == 0) ? minP : maxP;
                const double sign = (side == 0) ? -1.0 : 1.0;
                const Vector3 normal = unitAxis(axis, sign);
                const bool insideP = strictly ? (otherMinP < plane && plane < otherMaxP)
                                              : (otherMinP <= plane && plane <= otherMaxP);

                // Контур клетки ориентирован CCW при взгляде снаружи.
                // «Положительный» порядок (p00→p10→p11→p01) соответствует
                // нормали cross(unit(axisA), unit(axisB)); иначе — обратный.
                const int crossAxis = (3 - axisA - axisB) % 3;
                const bool positive = (axis == crossAxis && sign > 0.0);

                for (std::size_t i = 0; i + 1 < grids[axisA].size(); ++i)
                {
                    const double a0 = grids[axisA][i];
                    const double a1 = grids[axisA][i + 1];
                    if (a1 <= minA || a0 >= maxA)
                        continue;
                    const bool insideA = a0 >= otherMinA && a1 <= otherMaxA;
                    for (std::size_t j = 0; j + 1 < grids[axisB].size(); ++j)
                    {
                        const double b0 = grids[axisB][j];
                        const double b1 = grids[axisB][j + 1];
                        if (b1 <= minB || b0 >= maxB)
                            continue;
                        const bool insideB = b0 >= otherMinB && b1 <= otherMaxB;
                        if (insideP && insideA && insideB)
                            continue; // клетка полностью внутри другого тела

                        const Vector3 p00 = pointOnAxes(axis, plane, axisA, a0, axisB, b0);
                        const Vector3 p10 = pointOnAxes(axis, plane, axisA, a1, axisB, b0);
                        const Vector3 p11 = pointOnAxes(axis, plane, axisA, a1, axisB, b1);
                        const Vector3 p01 = pointOnAxes(axis, plane, axisA, a0, axisB, b1);

                        const BRepVertexHandle v00 = gridVertex(builder, vertexCache, p00);
                        const BRepVertexHandle v10 = gridVertex(builder, vertexCache, p10);
                        const BRepVertexHandle v11 = gridVertex(builder, vertexCache, p11);
                        const BRepVertexHandle v01 = gridVertex(builder, vertexCache, p01);
                        if (!v00.valid() || !v10.valid() || !v11.valid() || !v01.valid())
                            return false;

                        const BRepOrientedEdge e0 = positive
                            ? gridEdge(outModel, builder, edgeCache, v00, v10)
                            : gridEdge(outModel, builder, edgeCache, v10, v00);
                        const BRepOrientedEdge e1 = positive
                            ? gridEdge(outModel, builder, edgeCache, v10, v11)
                            : gridEdge(outModel, builder, edgeCache, v00, v01);
                        const BRepOrientedEdge e2 = positive
                            ? gridEdge(outModel, builder, edgeCache, v11, v01)
                            : gridEdge(outModel, builder, edgeCache, v01, v11);
                        const BRepOrientedEdge e3 = positive
                            ? gridEdge(outModel, builder, edgeCache, v01, v00)
                            : gridEdge(outModel, builder, edgeCache, v11, v10);
                        if (!e0.valid() || !e1.valid() || !e2.valid() || !e3.valid())
                            return false;

                        const BRepWireHandle wire = builder.makeWire({e0, e1, e2, e3}, true);
                        if (!wire.valid())
                            return false;

                        const BRepFaceHandle face = builder.makePlanarFace(
                            wire, p00, normal, unitAxis(axisA, 1.0));
                        if (!face.valid())
                            return false;
                        faces.push_back(face);
                    }
                }
            }
        }
        return true;
    }

    [[nodiscard]] static BRepBooleanResult fuseOverlappingBoxes(
        BRepModel& outModel,
        const Bounds3& argumentBounds,
        const Bounds3& toolBounds)
    {
        BRepBooleanResult result{};
        const auto checkpoint = outModel.checkpoint();
        BRepBuilderAPI builder(outModel);

        const std::array<std::vector<double>, 3> grids{
            coordinateGrid(argumentBounds.minX, argumentBounds.maxX,
                           toolBounds.minX, toolBounds.maxX),
            coordinateGrid(argumentBounds.minY, argumentBounds.maxY,
                           toolBounds.minY, toolBounds.maxY),
            coordinateGrid(argumentBounds.minZ, argumentBounds.maxZ,
                           toolBounds.minZ, toolBounds.maxZ)};

        std::vector<std::pair<Vector3, BRepVertexHandle>> vertexCache;
        std::map<std::pair<BRepIndex, BRepIndex>, BRepEdgeHandle> edgeCache;
        std::vector<BRepFaceHandle> faces;

        // Грани аргумента: строгое поглощение — совпадающие с границей
        // инструмента плоскости не вырезаются.
        if (!addBoxFaceTiles(outModel, builder, grids, vertexCache, edgeCache, faces,
                             argumentBounds, toolBounds, true))
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Fuse failed to tile the argument faces";
            return result;
        }
        // Грани инструмента: нестрогое поглощение — совпадающие плоскости
        // не дублируются (грань аргумента уже покрыта).
        if (!addBoxFaceTiles(outModel, builder, grids, vertexCache, edgeCache, faces,
                             toolBounds, argumentBounds, false))
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Fuse failed to tile the tool faces";
            return result;
        }

        if (faces.empty())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Fuse produced no faces";
            return result;
        }

        std::vector<BRepOrientedFace> oriented;
        oriented.reserve(faces.size());
        for (BRepFaceHandle face : faces)
            oriented.push_back({face, BRepOrientation::Forward});

        const BRepShellHandle shell = builder.makeShell(oriented, true);
        if (!shell.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Fuse failed to build the merged shell";
            return result;
        }
        const BRepSolidHandle solid = builder.makeSolid({shell}, true);
        if (!solid.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Fuse failed to build the merged solid";
            return result;
        }

        if (!validated(outModel))
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Fuse produced an invalid B-Rep model; operation rolled back";
            return result;
        }

        result.status = BRepBooleanStatus::Success;
        result.solid = solid;
        result.message = "Fuse completed (overlapping boxes merged via face tiling)";
        return result;
    }

    // ── Вырез с частичным перекрытием box'ов (замощение) ──────
    // W = A ∩ B (ненулевой объём). Поверхность A \ B: клетки граней A,
    // не поглощённые W (нестрого — дыры открываются и на гранях,
    // совпадающих с W), плюс стенки тех сторон W, которые не совпадают
    // с границами A. Кольца клеток и стенки сшиваются по рёбрам общей
    // сетки — единая замкнутая оболочка без inner wires.

    // Одна стенка выреза: плоскость (axis, plane), прямоугольник
    // [wMinA, wMaxA]×[wMinB, wMaxB] из W; нормаль внутрь выреза.
    [[nodiscard]] static bool addCutWall(
        BRepModel& outModel,
        BRepBuilderAPI& builder,
        std::vector<std::pair<Vector3, BRepVertexHandle>>& vertexCache,
        std::map<std::pair<BRepIndex, BRepIndex>, BRepEdgeHandle>& edgeCache,
        std::vector<BRepFaceHandle>& faces,
        const Bounds3& cutBounds,
        int axis,
        bool minSide)
    {
        const int axisA = (axis + 1) % 3;
        const int axisB = (axis + 2) % 3;
        const double plane = coordinate(cutBounds, axis, minSide);
        const double minA = coordinate(cutBounds, axisA, true);
        const double maxA = coordinate(cutBounds, axisA, false);
        const double minB = coordinate(cutBounds, axisB, true);
        const double maxB = coordinate(cutBounds, axisB, false);
        const double sign = minSide ? 1.0 : -1.0;
        const Vector3 normal = unitAxis(axis, sign);

        const int crossAxis = (3 - axisA - axisB) % 3;
        const bool positive = (axis == crossAxis && sign > 0.0);

        const Vector3 p00 = pointOnAxes(axis, plane, axisA, minA, axisB, minB);
        const Vector3 p10 = pointOnAxes(axis, plane, axisA, maxA, axisB, minB);
        const Vector3 p11 = pointOnAxes(axis, plane, axisA, maxA, axisB, maxB);
        const Vector3 p01 = pointOnAxes(axis, plane, axisA, minA, axisB, maxB);

        const BRepVertexHandle v00 = gridVertex(builder, vertexCache, p00);
        const BRepVertexHandle v10 = gridVertex(builder, vertexCache, p10);
        const BRepVertexHandle v11 = gridVertex(builder, vertexCache, p11);
        const BRepVertexHandle v01 = gridVertex(builder, vertexCache, p01);
        if (!v00.valid() || !v10.valid() || !v11.valid() || !v01.valid())
            return false;

        const BRepOrientedEdge e0 = positive
            ? gridEdge(outModel, builder, edgeCache, v00, v10)
            : gridEdge(outModel, builder, edgeCache, v10, v00);
        const BRepOrientedEdge e1 = positive
            ? gridEdge(outModel, builder, edgeCache, v10, v11)
            : gridEdge(outModel, builder, edgeCache, v00, v01);
        const BRepOrientedEdge e2 = positive
            ? gridEdge(outModel, builder, edgeCache, v11, v01)
            : gridEdge(outModel, builder, edgeCache, v01, v11);
        const BRepOrientedEdge e3 = positive
            ? gridEdge(outModel, builder, edgeCache, v01, v00)
            : gridEdge(outModel, builder, edgeCache, v11, v10);
        if (!e0.valid() || !e1.valid() || !e2.valid() || !e3.valid())
            return false;

        const BRepWireHandle wire = builder.makeWire({e0, e1, e2, e3}, true);
        if (!wire.valid())
            return false;

        const BRepFaceHandle face = builder.makePlanarFace(
            wire, p00, normal, unitAxis(axisA, 1.0));
        if (!face.valid())
            return false;
        faces.push_back(face);
        return true;
    }

    [[nodiscard]] static BRepBooleanResult cutPartialBoxes(
        BRepModel& outModel,
        const Bounds3& argumentBounds,
        const Bounds3& toolBounds)
    {
        BRepBooleanResult result{};
        const auto checkpoint = outModel.checkpoint();
        BRepBuilderAPI builder(outModel);

        const Bounds3 cutBounds = intersectBounds(argumentBounds, toolBounds);
        if (!cutBounds.valid)
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut could not compute the intersection bounds";
            return result;
        }

        const std::array<std::vector<double>, 3> grids{
            coordinateGrid(argumentBounds.minX, argumentBounds.maxX,
                           toolBounds.minX, toolBounds.maxX),
            coordinateGrid(argumentBounds.minY, argumentBounds.maxY,
                           toolBounds.minY, toolBounds.maxY),
            coordinateGrid(argumentBounds.minZ, argumentBounds.maxZ,
                           toolBounds.minZ, toolBounds.maxZ)};

        std::vector<std::pair<Vector3, BRepVertexHandle>> vertexCache;
        std::map<std::pair<BRepIndex, BRepIndex>, BRepEdgeHandle> edgeCache;
        std::vector<BRepFaceHandle> faces;

        // Клетки граней аргумента: нестрогое поглощение вырезом W.
        if (!addBoxFaceTiles(outModel, builder, grids, vertexCache, edgeCache, faces,
                             argumentBounds, cutBounds, false))
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut failed to tile the argument faces";
            return result;
        }

        // Стенки выреза: стороны W, не совпадающие с границами аргумента.
        for (int axis = 0; axis < 3; ++axis)
        {
            const double wMin = coordinate(cutBounds, axis, true);
            const double wMax = coordinate(cutBounds, axis, false);
            const double aMin = coordinate(argumentBounds, axis, true);
            const double aMax = coordinate(argumentBounds, axis, false);
            if (wMin > aMin)
            {
                if (!addCutWall(outModel, builder, vertexCache, edgeCache, faces,
                                cutBounds, axis, true))
                {
                    outModel.rollback(checkpoint);
                    result.status = BRepBooleanStatus::Failed;
                    result.message = "Cut failed to build the cut wall";
                    return result;
                }
            }
            if (wMax < aMax)
            {
                if (!addCutWall(outModel, builder, vertexCache, edgeCache, faces,
                                cutBounds, axis, false))
                {
                    outModel.rollback(checkpoint);
                    result.status = BRepBooleanStatus::Failed;
                    result.message = "Cut failed to build the cut wall";
                    return result;
                }
            }
        }

        if (faces.empty())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut produced no faces";
            return result;
        }

        std::vector<BRepOrientedFace> oriented;
        oriented.reserve(faces.size());
        for (BRepFaceHandle face : faces)
            oriented.push_back({face, BRepOrientation::Forward});

        const BRepShellHandle shell = builder.makeShell(oriented, true);
        if (!shell.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut failed to build the shell";
            return result;
        }
        const BRepSolidHandle solid = builder.makeSolid({shell}, true);
        if (!solid.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut failed to build the solid";
            return result;
        }

        if (!validated(outModel))
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut produced an invalid B-Rep model; operation rolled back";
            return result;
        }

        result.status = BRepBooleanStatus::Success;
        result.solid = solid;
        result.message = "Cut completed (partial overlap cut via face tiling)";
        return result;
    }

    // ── Сквозной проход (cut-through) ─────────────────────────
    // Инструмент протыкает аргумент насквозь по одной оси и строго
    // лежит внутри по двум другим. Результат: внешняя оболочка аргумента
    // с прямоугольными отверстиями на торцах + внутренняя оболочка —
    // «короб» туннеля (4 грани, ориентированные внутрь выреза).

    [[nodiscard]] static int passThroughAxis(const Bounds3& arg, const Bounds3& tool) noexcept
    {
        const bool xPass = tool.minX < arg.minX && tool.maxX > arg.maxX &&
                           tool.minY > arg.minY && tool.maxY < arg.maxY &&
                           tool.minZ > arg.minZ && tool.maxZ < arg.maxZ;
        if (xPass) return 0;
        const bool yPass = tool.minY < arg.minY && tool.maxY > arg.maxY &&
                           tool.minX > arg.minX && tool.maxX < arg.maxX &&
                           tool.minZ > arg.minZ && tool.maxZ < arg.maxZ;
        if (yPass) return 1;
        const bool zPass = tool.minZ < arg.minZ && tool.maxZ > arg.maxZ &&
                           tool.minX > arg.minX && tool.maxX < arg.maxX &&
                           tool.minY > arg.minY && tool.maxY < arg.maxY;
        if (zPass) return 2;
        return -1;
    }

    [[nodiscard]] static bool hasNoInnerWires(const BRepModel& model, BRepSolidHandle solid) noexcept
    {
        const BRepSolid* record = model.topology().solid(solid);
        if (!record) return false;
        for (BRepShellHandle shellHandle : record->shells)
        {
            const BRepShell* shell = model.topology().shell(shellHandle);
            if (!shell) return false;
            for (const BRepOrientedFace& oriented : shell->faces)
            {
                const BRepFace* face = model.topology().face(oriented.face);
                if (!face || !face->inners.empty())
                    return false;
            }
        }
        return true;
    }

    [[nodiscard]] static BRepWireHandle makeHoleWire(
        BRepBuilderAPI& builder, int axis, double plane,
        const Bounds3& tool)
    {
        Vector3 p0;
        Vector3 p1;
        Vector3 p2;
        Vector3 p3;
        switch (axis)
        {
            case 0:
                p0 = {plane, tool.minY, tool.minZ};
                p1 = {plane, tool.minY, tool.maxZ};
                p2 = {plane, tool.maxY, tool.maxZ};
                p3 = {plane, tool.maxY, tool.minZ};
                break;
            case 1:
                p0 = {tool.minX, plane, tool.minZ};
                p1 = {tool.minX, plane, tool.maxZ};
                p2 = {tool.maxX, plane, tool.maxZ};
                p3 = {tool.maxX, plane, tool.minZ};
                break;
            default:
                p0 = {tool.minX, tool.minY, plane};
                p1 = {tool.minX, tool.maxY, plane};
                p2 = {tool.maxX, tool.maxY, plane};
                p3 = {tool.maxX, tool.minY, plane};
                break;
        }

        const BRepVertexHandle v0 = builder.makeVertex(p0);
        const BRepVertexHandle v1 = builder.makeVertex(p1);
        const BRepVertexHandle v2 = builder.makeVertex(p2);
        const BRepVertexHandle v3 = builder.makeVertex(p3);
        if (!v0.valid() || !v1.valid() || !v2.valid() || !v3.valid())
            return {};

        const BRepEdgeHandle e0 = builder.makeEdgeLine(v0, v1);
        const BRepEdgeHandle e1 = builder.makeEdgeLine(v1, v2);
        const BRepEdgeHandle e2 = builder.makeEdgeLine(v2, v3);
        const BRepEdgeHandle e3 = builder.makeEdgeLine(v3, v0);
        if (!e0.valid() || !e1.valid() || !e2.valid() || !e3.valid())
            return {};

        return builder.makeWire(
            {{e0, BRepOrientation::Forward},
             {e1, BRepOrientation::Forward},
             {e2, BRepOrientation::Forward},
             {e3, BRepOrientation::Forward}},
            true);
    }

    // Одна боковая грань короба: плоскость planeAxis == planeValue,
    // диапазоны по оси прохода — из аргумента, по третьей оси — из инструмента.
    [[nodiscard]] static BRepFaceHandle makeTunnelFace(
        BRepBuilderAPI& builder,
        int passAxis,
        int planeAxis,
        double planeValue,
        const Bounds3& argumentBounds,
        const Bounds3& toolBounds)
    {
        const int thirdAxis = 3 - passAxis - planeAxis;

        const double passMin = coordinate(argumentBounds, passAxis, true);
        const double passMax = coordinate(argumentBounds, passAxis, false);
        const double thirdMin = coordinate(toolBounds, thirdAxis, true);
        const double thirdMax = coordinate(toolBounds, thirdAxis, false);

        const Vector3 p0 = pointOnAxes(passAxis, passMin, thirdAxis, thirdMin, planeAxis, planeValue);
        const Vector3 p1 = pointOnAxes(passAxis, passMax, thirdAxis, thirdMin, planeAxis, planeValue);
        const Vector3 p2 = pointOnAxes(passAxis, passMax, thirdAxis, thirdMax, planeAxis, planeValue);
        const Vector3 p3 = pointOnAxes(passAxis, passMin, thirdAxis, thirdMax, planeAxis, planeValue);

        const BRepVertexHandle v0 = builder.makeVertex(p0);
        const BRepVertexHandle v1 = builder.makeVertex(p1);
        const BRepVertexHandle v2 = builder.makeVertex(p2);
        const BRepVertexHandle v3 = builder.makeVertex(p3);
        if (!v0.valid() || !v1.valid() || !v2.valid() || !v3.valid())
            return {};

        const BRepEdgeHandle e0 = builder.makeEdgeLine(v0, v1);
        const BRepEdgeHandle e1 = builder.makeEdgeLine(v1, v2);
        const BRepEdgeHandle e2 = builder.makeEdgeLine(v2, v3);
        const BRepEdgeHandle e3 = builder.makeEdgeLine(v3, v0);
        if (!e0.valid() || !e1.valid() || !e2.valid() || !e3.valid())
            return {};

        const BRepWireHandle wire = builder.makeWire(
            {{e0, BRepOrientation::Forward},
             {e1, BRepOrientation::Forward},
             {e2, BRepOrientation::Forward},
             {e3, BRepOrientation::Forward}},
            true);
        if (!wire.valid())
            return {};

        // Нормаль смотрит внутрь выреза: для min-плоскости — вдоль оси,
        // для max-плоскости — против оси.
        const double planeMin = coordinate(toolBounds, planeAxis, true);
        const double sign = planeValue <= planeMin ? 1.0 : -1.0;
        const Vector3 normal = unitAxis(planeAxis, sign);
        const Vector3 xDir = unitAxis(passAxis, 1.0);

        return builder.makePlanarFace(wire, p0, normal, xDir);
    }

    [[nodiscard]] static BRepBooleanResult buildThroughHole(
        BRepModel& outModel,
        const BRepModel& argumentModel,
        BRepSolidHandle argumentSolid,
        const Bounds3& argumentBounds,
        const Bounds3& toolBounds,
        int axis)
    {
        BRepBooleanResult result{};
        const auto checkpoint = outModel.checkpoint();
        const BRepIndexMap argumentMap = copyModel(outModel, argumentModel);

        const BRepSolid* argumentRecord = argumentModel.topology().solid(argumentSolid);
        BRepBuilderAPI builder(outModel);

        std::vector<BRepShellHandle> outerShells;
        for (BRepShellHandle shellHandle : argumentRecord->shells)
        {
            const BRepShell* shell = argumentModel.topology().shell(shellHandle);
            if (!shell)
            {
                outModel.rollback(checkpoint);
                result.status = BRepBooleanStatus::Failed;
                result.message = "Cut-through: argument shell is missing";
                return result;
            }
            const BRepShellHandle copyShell = remap(argumentMap.shells, shellHandle);
            for (const BRepOrientedFace& oriented : shell->faces)
            {
                const BRepFace* face = argumentModel.topology().face(oriented.face);
                if (!face)
                {
                    outModel.rollback(checkpoint);
                    result.status = BRepBooleanStatus::Failed;
                    result.message = "Cut-through: argument face is missing";
                    return result;
                }

                // Торец (плоскость аргумента по оси прохода)?
                const int faceAxis = planeAxisOf(argumentModel, *face);
                if (faceAxis == axis && isEndCap(argumentBounds, axis, faceAxisCoordinate(argumentModel, *face)))
                {
                    const BRepFaceHandle copyFace = remap(argumentMap.faces, oriented.face);
                    const BRepWireHandle hole = makeHoleWire(builder, axis,
                                                             faceAxisCoordinate(argumentModel, *face),
                                                             toolBounds);
                    if (!hole.valid())
                    {
                        outModel.rollback(checkpoint);
                        result.status = BRepBooleanStatus::Failed;
                        result.message = "Cut-through failed to build the end-cap hole";
                        return result;
                    }
                    if (!BRepTopologyEditor::addInnerWire(
                            outModel.topology(), copyFace,
                            BRepOrientedWire{hole, BRepOrientation::Forward}))
                    {
                        outModel.rollback(checkpoint);
                        result.status = BRepBooleanStatus::Failed;
                        result.message = "Cut-through failed to attach the end-cap hole";
                        return result;
                    }
                }
            }
            outerShells.push_back(copyShell);
        }

        // Короб туннеля: 4 боковые грани вдоль оси прохода.
        std::vector<BRepOrientedFace> tunnelFaces;
        const int sideA = (axis + 1) % 3;
        const int sideB = (axis + 2) % 3;
        const double minA = coordinate(toolBounds, sideA, true);
        const double maxA = coordinate(toolBounds, sideA, false);
        const double minB = coordinate(toolBounds, sideB, true);
        const double maxB = coordinate(toolBounds, sideB, false);

        const std::array<std::tuple<int, double>, 4> planes{
            std::tuple<int, double>{sideA, minA},
            std::tuple<int, double>{sideA, maxA},
            std::tuple<int, double>{sideB, minB},
            std::tuple<int, double>{sideB, maxB}};
        for (const auto& plane : planes)
        {
            const BRepFaceHandle face = makeTunnelFace(
                builder, axis, std::get<0>(plane), std::get<1>(plane),
                argumentBounds, toolBounds);
            if (!face.valid())
            {
                outModel.rollback(checkpoint);
                result.status = BRepBooleanStatus::Failed;
                result.message = "Cut-through failed to build the tunnel face";
                return result;
            }
            tunnelFaces.push_back({face, BRepOrientation::Forward});
        }

        const BRepShellHandle innerShell = builder.makeShell(tunnelFaces, true);
        if (!innerShell.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut-through failed to build the tunnel shell";
            return result;
        }

        std::vector<BRepShellHandle> shells = outerShells;
        shells.push_back(innerShell);
        const BRepSolidHandle solid = builder.makeSolid(shells);
        if (!solid.valid())
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut-through failed to assemble the solid";
            return result;
        }

        if (!validated(outModel))
        {
            outModel.rollback(checkpoint);
            result.status = BRepBooleanStatus::Failed;
            result.message = "Cut-through produced an invalid B-Rep model; operation rolled back";
            return result;
        }

        result.status = BRepBooleanStatus::Success;
        result.solid = solid;
        result.message = "Cut-through completed (tunnel with end-cap holes)";
        return result;
    }

    [[nodiscard]] static double coordinate(const Bounds3& bounds, int axis, bool minValue) noexcept
    {
        switch (axis)
        {
            case 0: return minValue ? bounds.minX : bounds.maxX;
            case 1: return minValue ? bounds.minY : bounds.maxY;
            default: return minValue ? bounds.minZ : bounds.maxZ;
        }
    }

    [[nodiscard]] static Vector3 unitAxis(int axis, double sign) noexcept
    {
        switch (axis)
        {
            case 0: return {sign, 0.0, 0.0};
            case 1: return {0.0, sign, 0.0};
            default: return {0.0, 0.0, sign};
        }
    }

    [[nodiscard]] static Vector3 pointOnAxes(int axisA, double valueA,
                                             int axisB, double valueB,
                                             int axisC, double valueC) noexcept
    {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        assignAxis(x, y, z, axisA, valueA);
        assignAxis(x, y, z, axisB, valueB);
        assignAxis(x, y, z, axisC, valueC);
        return {x, y, z};
    }

    static void assignAxis(double& x, double& y, double& z, int axis, double value) noexcept
    {
        switch (axis)
        {
            case 0: x = value; break;
            case 1: y = value; break;
            default: z = value; break;
        }
    }

    [[nodiscard]] static int planeAxisOf(const BRepModel& model, const BRepFace& face) noexcept
    {
        const BRepSurfaceGeometry* surface = model.geometry().surface(face.surface);
        if (!surface || surface->type != BRepSurfaceType::Plane)
            return -1;
        const Vector3 n = surface->plane.normal;
        const double ax = std::abs(n.x);
        const double ay = std::abs(n.y);
        const double az = std::abs(n.z);
        if (ax > 0.999999 && ay < 1.0e-9 && az < 1.0e-9) return 0;
        if (ay > 0.999999 && ax < 1.0e-9 && az < 1.0e-9) return 1;
        if (az > 0.999999 && ax < 1.0e-9 && ay < 1.0e-9) return 2;
        return -1;
    }

    [[nodiscard]] static double faceAxisCoordinate(const BRepModel& model, const BRepFace& face) noexcept
    {
        const BRepSurfaceGeometry* surface = model.geometry().surface(face.surface);
        if (!surface || surface->type != BRepSurfaceType::Plane)
            return 0.0;
        const Vector3 n = surface->plane.normal;
        const Vector3 p = surface->plane.location;
        return n.x * p.x + n.y * p.y + n.z * p.z;
    }

    [[nodiscard]] static bool isEndCap(const Bounds3& bounds, int axis, double plane) noexcept
    {
        constexpr double tolerance = 1.0e-6;
        const double minValue = coordinate(bounds, axis, true);
        const double maxValue = coordinate(bounds, axis, false);
        return std::abs(plane - minValue) <= tolerance ||
               std::abs(plane - maxValue) <= tolerance;
    }
};

} // namespace mir