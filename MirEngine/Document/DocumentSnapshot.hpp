#pragma once

#include "../Geometry/Scene/Scene.hpp"

#include <memory>
#include <vector>

namespace mir4d
{

/// Immutable description of the spatial state of a MIR 4D document.
/// Time itself belongs to Document; this snapshot stores the scene at one
/// point in that history.
struct DocumentSnapshotNode
{
    ObjectId id{InvalidObjectId};
    std::shared_ptr<mir::Model> model;
    mir::Transform transform{mir::Transform::identity()};
};

class DocumentSnapshot
{
public:
    DocumentSnapshot() = default;

    [[nodiscard]] static DocumentSnapshot capture(const mir::Scene& scene)
    {
        DocumentSnapshot snapshot;
        snapshot.nodes_.reserve(scene.size());

        for (const auto& node : scene.nodes())
        {
            if (!node)
                continue;

            snapshot.nodes_.push_back({
                node->id(),
                node->model(),
                node->transform()
            });
        }

        return snapshot;
    }

    [[nodiscard]] bool restore(mir::Scene& scene) const
    {
        // Validate the complete snapshot before mutating the destination.
        // Restoring through a temporary Scene used to replace the destination's
        // ObjectRegistry pointer with a pointer to a temporary registry. That
        // left a dangling registry after this function returned and could cause
        // undefined behaviour in the next object allocation.
        for (const auto& state : nodes_)
        {
            if (!isValidObjectId(state.id) || !state.model ||
                !state.model->isValid() || !state.transform.isValid())
            {
                return false;
            }
        }

        // Keep the destination Scene (and therefore its Document-owned
        // ObjectRegistry) alive. clear() releases old IDs; add() then reserves
        // the snapshot IDs in the same registry.
        scene.clear();

        for (const auto& state : nodes_)
        {
            auto node = std::make_shared<mir::ModelNode>(state.model);
            node->setId(state.id);
            node->setTransform(state.transform);

            if (!scene.add(std::move(node)))
            {
                scene.clear();
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] const std::vector<DocumentSnapshotNode>& nodes() const noexcept
    {
        return nodes_;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return nodes_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return nodes_.empty();
    }

    void clear() noexcept
    {
        nodes_.clear();
    }

private:
    std::vector<DocumentSnapshotNode> nodes_;
};

} // namespace mir4d
