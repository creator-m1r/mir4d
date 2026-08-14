#pragma once

#include "../Core/Identity/ObjectRegistry.hpp"
#include "ModelNode.hpp"
#include "../Geometry/Scene/Scene.hpp"

#include <cstddef>
#include <memory>

namespace mir4d
{

/// Canonical document object store.
///
/// Object identity and ModelNode ownership belong to the Document layer.
/// Scene remains only as a transitional spatial compatibility view.
class ObjectStore
{
public:
    using Node = ModelNode;
    using NodePtr = std::shared_ptr<Node>;

    explicit ObjectStore(ObjectRegistry& registry) noexcept
        : registry_(registry)
        , scene_(registry_)
    {
    }

    [[nodiscard]] mir::Scene& scene() noexcept { return scene_; }
    [[nodiscard]] const mir::Scene& scene() const noexcept { return scene_; }

    [[nodiscard]] ObjectRegistry& registry() noexcept { return registry_; }
    [[nodiscard]] const ObjectRegistry& registry() const noexcept { return registry_; }

    [[nodiscard]] std::size_t size() const noexcept { return scene_.size(); }
    [[nodiscard]] bool empty() const noexcept { return scene_.empty(); }

    [[nodiscard]] NodePtr find(ObjectId id) const noexcept
    {
        return scene_.find(id);
    }

    [[nodiscard]] bool contains(ObjectId id) const noexcept
    {
        return scene_.contains(id);
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        return scene_.isValid();
    }

    void clear() noexcept
    {
        scene_.clear();
    }

private:
    ObjectRegistry& registry_;
    mir::Scene scene_;
};

} // namespace mir4d
