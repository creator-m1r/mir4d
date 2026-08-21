
#pragma once

#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Math/Transform.hpp"
#include "MirEngine/Core/Identity/ObjectId.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace mir4d
{

class SceneCommand
{
public:
    virtual ~SceneCommand() = default;

    virtual void execute(mir::Scene& scene) = 0;
    virtual void undo(mir::Scene& scene) = 0;

    [[nodiscard]] virtual const char* description() const noexcept = 0;
};

class MoveObjectCommand final : public SceneCommand
{
public:
    MoveObjectCommand(ObjectId objectId,
                      mir::Transform from,
                      mir::Transform to) noexcept
        : objectId_(objectId)
        , from_(from)
        , to_(to)
    {
    }

    void execute(mir::Scene& scene) override
    {
        apply(scene, to_);
    }

    void undo(mir::Scene& scene) override
    {
        apply(scene, from_);
    }

    [[nodiscard]] const char* description() const noexcept override
    {
        return "MOVE";
    }

private:
    void apply(mir::Scene& scene, const mir::Transform& transform)
    {
        const auto node = scene.find(objectId_);
        if (node)
            node->setTransform(transform);
    }

    ObjectId objectId_;
    mir::Transform from_;
    mir::Transform to_;
};

using GrabTransformCommand = MoveObjectCommand;

class DeleteObjectCommand final : public SceneCommand
{
public:
    explicit DeleteObjectCommand(std::shared_ptr<mir::ModelNode> node) noexcept
        : node_(std::move(node))
    {
    }

    void execute(mir::Scene& scene) override
    {
        if (node_)
            scene.remove(node_->id());
    }

    void undo(mir::Scene& scene) override
    {
        if (node_)
            (void)scene.add(node_);
    }

    [[nodiscard]] const char* description() const noexcept override
    {
        return "DELETE";
    }

private:
    std::shared_ptr<mir::ModelNode> node_;
};

class DeformObjectCommand final : public SceneCommand
{
public:
    DeformObjectCommand(ObjectId objectId,
                        std::vector<mir::Point3> from,
                        std::vector<mir::Point3> to) noexcept
        : objectId_(objectId)
        , from_(std::move(from))
        , to_(std::move(to))
    {
    }

    void execute(mir::Scene& scene) override { apply(scene, to_); }
    void undo(mir::Scene& scene) override { apply(scene, from_); }

    [[nodiscard]] const char* description() const noexcept override
    {
        return "DEFORM";
    }

private:
    void apply(mir::Scene& scene, const std::vector<mir::Point3>& verts)
    {
        const auto node = scene.find(objectId_);
        if (!node || !node->model() || !node->model()->hasMesh())
            return;
        auto* model = const_cast<mir::Model*>(node->model().get());
        auto& mesh = model->mesh();
        if (mesh.vertices.size() != verts.size())
            return;
        mesh.vertices = verts;

        if (mesh.normals.size() != mesh.vertices.size())
            mesh.normals.resize(mesh.vertices.size());
        std::vector<mir::Vector3> acc(mesh.vertices.size(), mir::Vector3(0, 0, 0));
        for (const auto& t : mesh.triangles)
        {
            const mir::Vector3 fn = mir::Vector3::cross(
                mesh.vertices[t.b] - mesh.vertices[t.a],
                mesh.vertices[t.c] - mesh.vertices[t.a]);
            acc[t.a] += fn;
            acc[t.b] += fn;
            acc[t.c] += fn;
        }
        for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            const double l = acc[i].length();
            mesh.normals[i] = l > 1e-9 ? acc[i].normalized() : mir::Vector3(0, 0, 1);
        }

        node->touch();
    }

    ObjectId objectId_;
    std::vector<mir::Point3> from_;
    std::vector<mir::Point3> to_;
};

class SceneCommandHistory
{
public:
    void execute(std::unique_ptr<SceneCommand> command, mir::Scene& scene)
    {
        if (!command)
            return;

        command->execute(scene);

        redo_.clear();
        undo_.push_back(std::move(command));
    }

    bool undo(mir::Scene& scene)
    {
        if (undo_.empty())
            return false;

        auto command = std::move(undo_.back());
        undo_.pop_back();
        command->undo(scene);
        redo_.push_back(std::move(command));
        return true;
    }

    bool redo(mir::Scene& scene)
    {
        if (redo_.empty())
            return false;

        auto command = std::move(redo_.back());
        redo_.pop_back();
        command->execute(scene);
        undo_.push_back(std::move(command));
        return true;
    }

    void clear() noexcept
    {
        undo_.clear();
        redo_.clear();
    }

    [[nodiscard]] bool canUndo() const noexcept { return !undo_.empty(); }
    [[nodiscard]] bool canRedo() const noexcept { return !redo_.empty(); }

private:
    std::vector<std::unique_ptr<SceneCommand>> undo_;
    std::vector<std::unique_ptr<SceneCommand>> redo_;
};

}