#pragma once

#include "../Model/ModelNode.hpp"
#include "../../Core/Identity/ObjectRegistry.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mir
{

/// Transitional spatial scene implementation.
///
/// When constructed with the Document-owned ObjectRegistry, Scene no longer
/// owns document identity. The default constructor remains only for isolated
/// legacy consumers during migration to ObjectStore.
class Scene
{
public:
    using NodePtr = std::shared_ptr<ModelNode>;
    using NodeList = std::vector<NodePtr>;

    Scene() noexcept
        : registry_(&localRegistry_)
    {
    }

    explicit Scene(mir4d::ObjectRegistry& registry) noexcept
        : registry_(&registry)
    {
    }

    [[nodiscard]] NodePtr add(NodePtr node)
    {
        if (!node)
            return nullptr;

        const mir4d::ObjectId existingId = node->id();
        if (mir4d::isValidObjectId(existingId))
        {
            if (index_.contains(existingId))
                return nullptr;

            if (!registry_->reserve(existingId))
                return nullptr;
        }
        else
        {
            node->setId(registry_->allocate());
        }

        const mir4d::ObjectId finalId = node->id();
        if (!mir4d::isValidObjectId(finalId))
            return nullptr;

        nodes_.push_back(node);
        index_.emplace(finalId, node);
        ++revision_;
        return node;
    }

    [[nodiscard]] NodePtr createNode(std::shared_ptr<Model> model)
    {
        return add(std::make_shared<ModelNode>(std::move(model)));
    }

    [[nodiscard]] NodePtr find(mir4d::ObjectId id) const noexcept
    {
        if (!mir4d::isValidObjectId(id))
            return nullptr;

        const auto it = index_.find(id);
        return it == index_.end() ? nullptr : it->second;
    }

    bool remove(mir4d::ObjectId id) noexcept
    {
        if (!mir4d::isValidObjectId(id))
            return false;

        const auto indexIt = index_.find(id);
        if (indexIt == index_.end())
            return false;

        for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        {
            if (*it && (*it)->id() == id)
            {
                nodes_.erase(it);
                break;
            }
        }

        index_.erase(indexIt);
        registry_->release(id);
        ++revision_;
        return true;
    }

    bool remove(const NodePtr& node) noexcept
    {
        return node ? remove(node->id()) : false;
    }

    void clear() noexcept
    {
        for (const auto& node : nodes_)
        {
            if (node)
                registry_->release(node->id());
        }

        nodes_.clear();
        index_.clear();
        ++revision_;
    }

    [[nodiscard]] std::size_t size() const noexcept { return nodes_.size(); }
    [[nodiscard]] bool empty() const noexcept { return nodes_.empty(); }
    [[nodiscard]] const NodeList& nodes() const noexcept { return nodes_; }

    [[nodiscard]] bool contains(mir4d::ObjectId id) const noexcept
    {
        return mir4d::isValidObjectId(id) && index_.find(id) != index_.end();
    }

    [[nodiscard]] std::uint64_t revision() const noexcept { return revision_; }

    [[nodiscard]] std::uint64_t contentRevision() const noexcept
    {
        std::uint64_t value = revision_ + 0x9e3779b97f4a7c15ULL;
        for (const auto& node : nodes_)
        {
            if (!node)
                continue;

            value ^= node->revision() + 0x9e3779b97f4a7c15ULL +
                     (value << 6U) + (value >> 2U);
            value ^= node->id() + 0x517cc1b727220a95ULL +
                     (value << 6U) + (value >> 2U);
        }
        return value;
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        if (nodes_.size() != index_.size())
            return false;

        for (const auto& node : nodes_)
        {
            if (!node || !node->isValid())
                return false;

            const auto it = index_.find(node->id());
            if (it == index_.end() || it->second != node)
                return false;
        }
        return true;
    }

private:
    mir4d::ObjectRegistry localRegistry_;
    mir4d::ObjectRegistry* registry_;
    NodeList nodes_;
    std::unordered_map<mir4d::ObjectId, NodePtr> index_;
    std::uint64_t revision_{0};
};

} // namespace mir
