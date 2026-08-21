#pragma once

#include "MirEngine/Core/Identity/ObjectId.hpp"
#include "MirEngine/Interaction/PickTypes.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace mir
{

/// Transitional interaction state. Identity is canonical mir4d::ObjectId.
///
/// The state optionally carries the hierarchical selection kind (Body/Face/
/// Edge/Vertex) and the corresponding element id of the primary selection.
/// The renderer uses it to highlight exactly the picked element instead of
/// the whole object.
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

    /// Hierarchical kind of the primary selection.
    [[nodiscard]] PickKind kind() const noexcept { return kind_; }

    /// Element id of the primary selection (vertex/edge index or face id).
    [[nodiscard]] std::uint64_t elementId() const noexcept { return elementId_; }

    /// Source B-Rep face id of the primary selection (0 = whole object).
    /// Mirrors `elementId` when `kind == PickKind::Face`.
    [[nodiscard]] std::uint64_t faceId() const noexcept
    {
        return kind_ == PickKind::Face ? elementId_ : 0;
    }

    [[nodiscard]] bool contains(mir4d::ObjectId id) const noexcept
    {
        return std::find(ids_.begin(), ids_.end(), id) != ids_.end();
    }

    void clear() noexcept
    {
        ids_.clear();
        kind_ = PickKind::Body;
        elementId_ = 0;
    }

    void select(mir4d::ObjectId id, bool additive = false)
    {
        selectElement(PickKind::Body, id, 0, additive);
    }

    /// Selects one object together with the source face of the hit point.
    void selectFace(mir4d::ObjectId id, std::uint64_t faceId)
    {
        selectElement(PickKind::Face, id, faceId);
    }

    /// Selects an element of the given hierarchical kind, replacing the set
    /// unless `additive` is set.
    void selectElement(PickKind kind, mir4d::ObjectId id, std::uint64_t elementId, bool additive = false)
    {
        if (!mir4d::isValidObjectId(id))
            return;

        if (!additive)
            ids_.clear();

        kind_ = kind;
        elementId_ = elementId;

        if (!contains(id))
            ids_.push_back(id);
    }

    void deselect(mir4d::ObjectId id)
    {
        ids_.erase(std::remove(ids_.begin(), ids_.end(), id), ids_.end());
        if (ids_.empty())
        {
            kind_ = PickKind::Body;
            elementId_ = 0;
        }
    }

    void toggle(mir4d::ObjectId id)
    {
        if (!mir4d::isValidObjectId(id))
            return;

        if (contains(id))
            deselect(id);
        else
        {
            ids_.push_back(id);
            kind_ = PickKind::Body;
            elementId_ = 0;
        }
    }

    /// Toggles an element of the given kind. If the exact (object, kind,
    /// element) is already the primary selection it is removed; otherwise it
    /// becomes the new selection.
    void toggleElement(PickKind kind, mir4d::ObjectId id, std::uint64_t elementId)
    {
        if (!mir4d::isValidObjectId(id))
            return;

        if (primary() == id && kind_ == kind && elementId_ == elementId)
        {
            deselect(id);
            return;
        }

        ids_.clear();
        ids_.push_back(id);
        kind_ = kind;
        elementId_ = elementId;
    }

    void replace(std::vector<mir4d::ObjectId> ids)
    {
        ids_.clear();
        kind_ = PickKind::Body;
        elementId_ = 0;
        ids_.reserve(ids.size());
        for (mir4d::ObjectId id : ids)
        {
            if (mir4d::isValidObjectId(id) && !contains(id))
                ids_.push_back(id);
        }
    }

private:
    std::vector<mir4d::ObjectId> ids_;
    PickKind kind_{PickKind::Body};
    std::uint64_t elementId_{0};
};

} // namespace mir
