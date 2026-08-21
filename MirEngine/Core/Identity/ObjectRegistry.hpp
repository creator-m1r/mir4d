#pragma once

#include "ObjectId.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace mir4d
{

class ObjectRegistry
{
public:
    [[nodiscard]] ObjectId allocate()
    {
        constexpr ObjectId maxId = std::numeric_limits<ObjectId>::max();

        while (next_ != InvalidObjectId && next_ <= maxId)
        {
            const ObjectId id = next_;
            if (next_ == maxId)
                next_ = InvalidObjectId;
            else
                ++next_;

            if (isValidObjectId(id) && ids_.insert(id).second)
                return id;
        }

        throw std::overflow_error("MIR4D object identifier space exhausted");
    }

    [[nodiscard]] bool reserve(ObjectId id)
    {
        if (!isValidObjectId(id) || ids_.contains(id))
            return false;

        ids_.insert(id);
        if (id >= next_ || next_ == InvalidObjectId)
        {
            constexpr ObjectId maxId = std::numeric_limits<ObjectId>::max();
            next_ = id == maxId ? InvalidObjectId : id + 1;
        }
        return true;
    }

    bool release(ObjectId id) noexcept
    {
        return ids_.erase(id) != 0;
    }

    [[nodiscard]] bool contains(ObjectId id) const noexcept
    {
        return ids_.contains(id);
    }

    void clear() noexcept
    {
        ids_.clear();
        next_ = 1;
    }

private:
    std::unordered_set<ObjectId> ids_;
    ObjectId next_{1};
};

}
