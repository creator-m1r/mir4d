#pragma once

#include "MirEngine/Core/Identity/ObjectId.hpp"

#include <algorithm>
#include <vector>

namespace mir
{

/// Transitional interaction state. Identity is canonical mir4d::ObjectId.
class SelectionState
{
public:
    [[nodiscard]] bool empty() const noexcept { return ids_.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return ids_.size(); }

    [[nodiscard]] const std::vector<mir4d::ObjectId>& ids() const noexcept { return ids_; }

    [[nodiscard]] mir4d::ObjectId primary() const noexcept
    {
        return ids_.empty() ? mir4d::InvalidObjectId : ids_.front();
    }

    [[nodiscard]] bool contains(mir4d::ObjectId id) const noexcept
    {
        return std::find(ids_.begin(), ids_.end(), id) != ids_.end();
    }

    void clear() noexcept { ids_.clear(); }

    void select(mir4d::ObjectId id, bool additive = false)
    {
        if (!mir4d::isValidObjectId(id))
            return;

        if (!additive)
            ids_.clear();

        if (!contains(id))
            ids_.push_back(id);
    }

    void deselect(mir4d::ObjectId id)
    {
        ids_.erase(std::remove(ids_.begin(), ids_.end(), id), ids_.end());
    }

    void toggle(mir4d::ObjectId id)
    {
        if (!mir4d::isValidObjectId(id))
            return;

        if (contains(id))
            deselect(id);
        else
            ids_.push_back(id);
    }

    void replace(std::vector<mir4d::ObjectId> ids)
    {
        ids_.clear();
        ids_.reserve(ids.size());
        for (mir4d::ObjectId id : ids)
        {
            if (mir4d::isValidObjectId(id) && !contains(id))
                ids_.push_back(id);
        }
    }

private:
    std::vector<mir4d::ObjectId> ids_;
};

} // namespace mir
