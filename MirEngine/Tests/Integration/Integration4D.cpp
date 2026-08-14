#include "MirEngine/Document/Document.hpp"
#include "MirEngine/Document/DocumentSnapshot.hpp"
#include "MirEngine/Document/CommandHandler.hpp"
#include "MirEngine/Geometry/Model/ModelNode.hpp"
#include "MirEngine/Geometry/Solid/FacetedSolid.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Time/TimeMachine.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace
{
std::shared_ptr<mir::Model> makeValidModel()
{
    const std::vector<mir::Point3> vertices{
        {0,0,0},{1,0,0},{1,1,0},{0,1,0},
        {0,0,1},{1,0,1},{1,1,1},{0,1,1}};
    const std::vector<mir::Solid3::Triangle> triangles{
        {0,1,2},{0,2,3},{4,6,5},{4,7,6},
        {0,4,5},{0,5,1},{1,5,6},{1,6,2},
        {2,6,7},{2,7,3},{3,7,4},{3,4,0}};

    mir::Solid3 solid(vertices, triangles);
    assert(solid.isValid());

    mir::TriangleMesh3 mesh;
    mesh.vertices = vertices;
    for (const auto& t : triangles)
        mesh.triangles.push_back({t.a, t.b, t.c});
    assert(mesh.isValid());

    auto model = std::make_shared<mir::Model>();
    model->setSolid(std::move(solid));
    model->setMesh(std::move(mesh));
    assert(model->isValid());
    return model;
}

class TestCommandHandler final : public mir4d::CommandHandler
{
public:
    [[nodiscard]] mir4d::CommandResult execute(
        const mir4d::Command& command,
        mir::Scene& scene) override
    {
        if (command.type != mir4d::CommandType::Move)
            return mir4d::CommandResult::failure("Unsupported integration-test command");
        if (command.arguments.size() != 3)
            return mir4d::CommandResult::failure("MOVE requires three coordinates");

        auto node = scene.find(command.target);
        if (!node)
            return mir4d::CommandResult::failure("Target object not found");

        mir::Transform transform = node->transform();
        transform.position = mir::Point3(
            std::stod(command.arguments[0]),
            std::stod(command.arguments[1]),
            std::stod(command.arguments[2]));
        node->setTransform(transform);
        return mir4d::CommandResult::ok("MOVE replayed", command.target);
    }
};
}

int main()
{
    mir4d::Document document("MIR 4D integration test");
    assert(document.time().seconds() == 0.0 && document.scene().empty() && document.history().empty());

    auto node = std::make_shared<mir::ModelNode>(makeValidModel());
    auto added = document.scene().add(node);
    assert(added);
    assert(mir4d::isValidObjectId(added->id()));

    const mir4d::ObjectId objectId = added->id();
    auto duplicate = std::make_shared<mir::ModelNode>(makeValidModel());
    duplicate->setId(objectId);
    assert(document.scene().add(duplicate) == nullptr);

    const auto documentSnapshot = document.snapshot();
    assert(documentSnapshot.size() == 1 && documentSnapshot.nodes().front().id == objectId);

    assert(document.history().append(mir4d::Time(1.0), mir4d::CommandType::CreateBox, objectId) == 1);
    document.setTime(mir4d::Time(1.0));
    const auto snapshotAtOne = document.snapshot();

    TestCommandHandler handler;
    mir4d::TimeMachine timeMachine(document, handler);
    assert(timeMachine.captureSnapshot());

    assert(document.history().append(
        mir4d::Time(2.0),
        mir4d::CommandType::Move,
        objectId,
        {"10.0", "20.0", "30.0"}) == 2);

    document.advanceTime(2.0);
    assert(document.time().seconds() == 3.0);

    assert(timeMachine.seek(mir4d::Time(1.0)));
    auto restored = document.scene().find(objectId);
    assert(restored && restored->transform().position == mir::Point3(0,0,0));

    assert(timeMachine.seek(mir4d::Time(2.0)));
    restored = document.scene().find(objectId);
    assert(restored && restored->transform().position == mir::Point3(10,20,30));

    assert(document.restoreSnapshot(snapshotAtOne));
    restored = document.scene().find(objectId);
    assert(restored && restored->transform().position == mir::Point3(0,0,0));

    assert(timeMachine.seek(mir4d::Time(1.0)));
    assert(document.scene().remove(objectId));
    assert(document.scene().empty());

    auto replacement = std::make_shared<mir::ModelNode>(makeValidModel());
    replacement->setId(objectId);
    assert(document.scene().add(replacement));

    assert(document.history().size() == 2);
    assert(document.history().last()->target == objectId);
    assert(document.history().last()->time.seconds() == 2.0);
    return 0;
}
