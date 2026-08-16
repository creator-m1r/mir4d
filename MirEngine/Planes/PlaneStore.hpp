// MirEngine/Planes/PlaneStore.hpp
// =================================================================================
// Реестр рабочих плоскостей проекта (ТЗ раздел 2 / 28).
//
// Владеет плоскостями, автоматически создаёт базовые XY/XZ/YZ (удалить нельзя)
// и выдаёт уникальные идентификаторы пользовательским плоскостям.
// Header-only, чтобы не менять CMake-цели STATIC-библиотек.
// =================================================================================

#pragma once

#include "Plane.hpp"
#include "PlaneFactory.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace mir
{

class PlaneStore
{
public:
    PlaneStore() = default;

    /// Гарантирует наличие трёх системных плоскостей с фиксированными id.
    void ensureBasePlanes()
    {
        if (!find(kBasePlaneXY))
            add(PlaneFactory::createBaseXY());
        if (!find(kBasePlaneXZ))
            add(PlaneFactory::createBaseXZ());
        if (!find(kBasePlaneYZ))
            add(PlaneFactory::createBaseYZ());
    }

    /// Добавляет плоскость, назначая ей идентификатор, если он ещё не задан.
    /// Возвращает false, если id уже занят.
    bool add(std::shared_ptr<Plane> plane)
    {
        if (!plane)
            return false;
        if (plane->id() == 0)
        {
            plane->setId(allocateId());
        }
        else if (planes_.contains(plane->id()))
        {
            return false;
        }
        planes_.emplace(plane->id(), plane);
        return true;
    }

    /// Удаляет пользовательскую плоскость. Системные плоскости не удаляются.
    bool remove(std::uint32_t id)
    {
        const auto it = planes_.find(id);
        if (it == planes_.end())
            return false;
        if (!it->second->deletable())
            return false;
        planes_.erase(it);
        return true;
    }

    [[nodiscard]] std::shared_ptr<Plane> find(std::uint32_t id) const
    {
        const auto it = planes_.find(id);
        return it == planes_.end() ? nullptr : it->second;
    }

    [[nodiscard]] bool contains(std::uint32_t id) const
    {
        return planes_.contains(id);
    }

    /// Все плоскости в порядке создания (системные — первыми после ensure).
    [[nodiscard]] std::vector<std::shared_ptr<Plane>> list() const
    {
        std::vector<std::shared_ptr<Plane>> result;
        result.reserve(planes_.size());
        for (const auto& [id, plane] : planes_)
            result.push_back(plane);
        return result;
    }

    [[nodiscard]] std::size_t size() const noexcept { return planes_.size(); }

private:
    static bool isReservedId(std::uint32_t id) noexcept
    {
        return id == kBasePlaneXY || id == kBasePlaneXZ || id == kBasePlaneYZ;
    }

    std::uint32_t allocateId() noexcept
    {
        while (planes_.contains(nextUserId_) || isReservedId(nextUserId_))
            ++nextUserId_;
        return nextUserId_++;
    }

    std::unordered_map<std::uint32_t, std::shared_ptr<Plane>> planes_;
    std::uint32_t nextUserId_{100};
};

} // namespace mir
