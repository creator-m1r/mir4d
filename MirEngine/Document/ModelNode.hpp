#pragma once

#include "../Geometry/Model/Model.hpp"
#include "../Core/Identity/ObjectId.hpp"
#include "../Math/Transform.hpp"

#include <cstdint>
#include <memory>
#include <utility>

namespace mir4d
{

/// Document-owned engineering object instance.
///
/// The implementation is owned by the Document layer. Geometry remains a
/// lower-level dependency; the old Geometry/Model/ModelNode.hpp path is only
/// a compatibility alias during migration.
class ModelNode
{
public:
    ModelNode() = default;

    explicit ModelNode(std::shared_ptr<mir::Model> model)
        : model_(std::move(model))
    {
    }

    [[nodiscard]] ObjectId id() const noexcept { return id_; }

    void setId(ObjectId id) noexcept
    {
        if (id_ == id)
            return;
        id_ = id;
        touch();
    }

    [[nodiscard]] const std::shared_ptr<mir::Model>& model() const noexcept
    {
        return model_;
    }

    void setModel(std::shared_ptr<mir::Model> model) noexcept
    {
        model_ = std::move(model);
        touch();
    }

    [[nodiscard]] const Transform& transform() const noexcept
    {
        return transform_;
    }

    void setTransform(const Transform& transform) noexcept
    {
        transform_ = transform;
        touch();
    }

    [[nodiscard]] std::uint64_t revision() const noexcept { return revision_; }

    void touch() noexcept { ++revision_; }

    [[nodiscard]] bool isValid() const noexcept
    {
        return isValidObjectId(id_) &&
               model_ != nullptr &&
               model_->isValid() &&
               transform_.isValid();
    }

private:
    ObjectId id_{InvalidObjectId};
    std::shared_ptr<mir::Model> model_;
    Transform transform_{Transform::identity()};
    std::uint64_t revision_{0};
};

} // namespace mir4d
