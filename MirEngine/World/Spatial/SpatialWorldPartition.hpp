#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace mir
{

struct WorldPosition
{
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

struct WorldBounds
{
    WorldPosition min{};
    WorldPosition max{};
};

struct SpatialCellCoordinate
{
    std::int64_t x{0};
    std::int64_t y{0};
    std::int64_t z{0};

    [[nodiscard]] bool operator==(const SpatialCellCoordinate&) const noexcept = default;
};

struct SpatialCellCoordinateHash
{
    [[nodiscard]] std::size_t operator()(const SpatialCellCoordinate& value) const noexcept
    {
        std::size_t hash = static_cast<std::size_t>(value.x * 73856093LL);
        hash ^= static_cast<std::size_t>(value.y * 19349663LL);
        hash ^= static_cast<std::size_t>(value.z * 83492791LL);
        return hash;
    }
};

enum class SpatialCellState : std::uint8_t
{
    Unloaded,
    Loading,
    Loaded,
    Active
};

struct SpatialCell
{
    SpatialCellCoordinate coordinate{};
    SpatialCellState state{SpatialCellState::Unloaded};
    std::vector<std::uint64_t> objectIds;
};

struct SpatialPartitionSettings
{
    double cellSize{32.0};
    int activeRadius{2};
    int loadedRadius{4};
};

class SpatialWorldPartition
{
public:
    explicit SpatialWorldPartition(SpatialPartitionSettings settings = {})
        : settings_(settings)
    {
        settings_.cellSize = std::max(1.0, settings_.cellSize);
        settings_.activeRadius = std::max(0, settings_.activeRadius);
        settings_.loadedRadius = std::max(settings_.activeRadius, settings_.loadedRadius);
    }

    [[nodiscard]] SpatialCellCoordinate cellFor(WorldPosition position) const noexcept
    {
        return {
            static_cast<std::int64_t>(std::floor(position.x / settings_.cellSize)),
            static_cast<std::int64_t>(std::floor(position.y / settings_.cellSize)),
            static_cast<std::int64_t>(std::floor(position.z / settings_.cellSize))};
    }

    SpatialCell& ensureCell(SpatialCellCoordinate coordinate)
    {
        return cells_[coordinate];
    }

    void addObject(std::uint64_t objectId, WorldPosition position)
    {
        auto& cell = ensureCell(cellFor(position));
        cell.coordinate = cellFor(position);

        if (std::find(cell.objectIds.begin(), cell.objectIds.end(), objectId) == cell.objectIds.end())
            cell.objectIds.push_back(objectId);
    }

    void removeObject(std::uint64_t objectId, WorldPosition position)
    {
        const auto coordinate = cellFor(position);
        const auto iterator = cells_.find(coordinate);
        if (iterator == cells_.end())
            return;

        auto& objects = iterator->second.objectIds;
        objects.erase(std::remove(objects.begin(), objects.end(), objectId), objects.end());
    }

    void updateStreaming(WorldPosition cameraPosition)
    {
        const auto center = cellFor(cameraPosition);

        for (auto& [coordinate, cell] : cells_)
        {
            const auto dx = std::llabs(coordinate.x - center.x);
            const auto dy = std::llabs(coordinate.y - center.y);
            const auto dz = std::llabs(coordinate.z - center.z);
            const auto distance = std::max({dx, dy, dz});

            if (distance <= settings_.activeRadius)
                cell.state = SpatialCellState::Active;
            else if (distance <= settings_.loadedRadius)
                cell.state = SpatialCellState::Loaded;
            else
                cell.state = SpatialCellState::Unloaded;
        }
    }

    [[nodiscard]] std::vector<const SpatialCell*> activeCells() const
    {
        std::vector<const SpatialCell*> result;
        for (const auto& [coordinate, cell] : cells_)
            if (cell.state == SpatialCellState::Active)
                result.push_back(&cell);
        return result;
    }

    [[nodiscard]] std::vector<const SpatialCell*> loadedCells() const
    {
        std::vector<const SpatialCell*> result;
        for (const auto& [coordinate, cell] : cells_)
            if (cell.state == SpatialCellState::Loaded || cell.state == SpatialCellState::Active)
                result.push_back(&cell);
        return result;
    }

    [[nodiscard]] std::size_t cellCount() const noexcept
    {
        return cells_.size();
    }

    [[nodiscard]] const SpatialPartitionSettings& settings() const noexcept
    {
        return settings_;
    }

private:
    SpatialPartitionSettings settings_{};
    std::unordered_map<SpatialCellCoordinate, SpatialCell, SpatialCellCoordinateHash> cells_;
};

} // namespace mir
