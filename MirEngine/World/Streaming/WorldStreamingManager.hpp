#pragma once

#include "../Spatial/SpatialWorldPartition.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace mir
{

enum class StreamingOperation : std::uint8_t
{
    None,
    Load,
    Activate,
    Deactivate,
    Unload
};

struct WorldStreamingOperation
{
    SpatialCellCoordinate cell{};
    StreamingOperation operation{StreamingOperation::None};
};

struct WorldStreamingBudget
{
    std::size_t maxLoadsPerUpdate{2};
    std::size_t maxUnloadsPerUpdate{4};
    std::size_t maxActiveCells{32};
};

struct WorldStreamingResult
{
    std::vector<WorldStreamingOperation> operations;
    std::size_t loadedCells{0};
    std::size_t activeCells{0};
};

class WorldStreamingManager
{
public:
    explicit WorldStreamingManager(WorldStreamingBudget budget = {})
        : budget_(budget)
    {
    }

    [[nodiscard]] WorldStreamingResult update(
        SpatialWorldPartition& partition,
        WorldPosition cameraPosition)
    {
        WorldStreamingResult result{};
        partition.updateStreaming(cameraPosition);

        const auto active = partition.activeCells();
        const auto loaded = partition.loadedCells();

        result.activeCells = std::min(active.size(), budget_.maxActiveCells);
        result.loadedCells = loaded.size();

        std::unordered_set<SpatialCellCoordinate, SpatialCellCoordinateHash> desiredLoaded;
        desiredLoaded.reserve(loaded.size());
        for (const auto* cell : loaded)
            desiredLoaded.insert(cell->coordinate);

        std::size_t loads = 0;
        std::size_t unloads = 0;

        for (const auto* cell : loaded)
        {
            if (!knownLoaded_.contains(cell->coordinate) && loads < budget_.maxLoadsPerUpdate)
            {
                result.operations.push_back({cell->coordinate, StreamingOperation::Load});
                knownLoaded_.insert(cell->coordinate);
                ++loads;
            }
        }

        for (const auto& coordinate : knownLoaded_)
        {
            if (!desiredLoaded.contains(coordinate) && unloads < budget_.maxUnloadsPerUpdate)
            {
                result.operations.push_back({coordinate, StreamingOperation::Unload});
                ++unloads;
            }
        }

        for (const auto* cell : active)
        {
            if (active_.contains(cell->coordinate))
                continue;

            if (active_.size() >= budget_.maxActiveCells)
                break;

            result.operations.push_back({cell->coordinate, StreamingOperation::Activate});
            active_.insert(cell->coordinate);
        }

        for (const auto& coordinate : active_)
        {
            const auto loadedIterator = desiredLoaded.find(coordinate);
            if (loadedIterator == desiredLoaded.end())
            {
                result.operations.push_back({coordinate, StreamingOperation::Deactivate});
            }
        }

        for (const auto& operation : result.operations)
        {
            if (operation.operation == StreamingOperation::Unload)
                knownLoaded_.erase(operation.cell);
            else if (operation.operation == StreamingOperation::Deactivate)
                active_.erase(operation.cell);
        }

        return result;
    }

    [[nodiscard]] const WorldStreamingBudget& budget() const noexcept
    {
        return budget_;
    }

private:
    WorldStreamingBudget budget_{};
    std::unordered_set<SpatialCellCoordinate, SpatialCellCoordinateHash> knownLoaded_;
    std::unordered_set<SpatialCellCoordinate, SpatialCellCoordinateHash> active_;
};

} // namespace mir
