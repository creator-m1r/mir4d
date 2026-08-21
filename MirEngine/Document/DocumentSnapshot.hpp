#pragma once

#include "../Geometry/Scene/Scene.hpp"

#include <memory>
#include <vector>

namespace mir4d
{

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

        for (const auto& state : nodes_)
        {
            if (!isValidObjectId(state.id) || !state.model ||
                !state.model->isValid() || !state.transform.isValid())
            {
                return false;
            }
        }

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

}
