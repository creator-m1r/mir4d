#pragma once

// MirEngine/BRep/Commands/BRepSceneBridge.hpp

#include "MirEngine/BRep/Converters/BRepToModel.hpp"
#include "MirEngine/BRep/Builders/BRepPrimAPI_MakeBox.hpp"
#include "MirEngine/BRep/Builders/BRepMakeRectangleProfile.hpp"
#include "MirEngine/BRep/Builders/BRepExtrudeBuilder.hpp"

#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Model/ModelNode.hpp"
#include "MirEngine/Math/Transform.hpp"
#include "MirEngine/Core/Identity/ObjectId.hpp"

#include <memory>
#include <utility>

namespace mir4d
{

struct BRepSceneInsertResult
{
    bool success{false};
    ObjectId objectId{InvalidObjectId};
    std::shared_ptr<mir::Model> model{};
    mir::BRepValidationReport report{};
};

class BRepSceneBridge
{
public:
    [[nodiscard]] static BRepSceneInsertResult insertModel(
        mir::Scene& scene,
        std::shared_ptr<mir::Model> model,
        const mir::Transform& transform = mir::Transform::identity())
    {
        BRepSceneInsertResult result{};
        if (!model || !model->isValid())
        {
            result.report.add(mir::BRepValidationSeverity::Error, mir::BRepShapeType::Solid,
                              mir::InvalidBRepIndex, "BRepSceneBridge: model is null or invalid");
            return result;
        }

        auto node = std::make_shared<mir::ModelNode>(std::move(model));
        node->setTransform(transform);
        auto added = scene.add(std::move(node));
        if (!added)
        {
            result.report.add(mir::BRepValidationSeverity::Error, mir::BRepShapeType::Solid,
                              mir::InvalidBRepIndex, "BRepSceneBridge: Scene::add failed");
            return result;
        }

        result.objectId = added->id();
        result.model = added->model();
        result.success = isValidObjectId(result.objectId);
        return result;
    }

    [[nodiscard]] static BRepSceneInsertResult createBox(
        mir::Scene& scene,
        mir::BRepModel& brep,
        mir::Scalar sizeX,
        mir::Scalar sizeY,
        mir::Scalar sizeZ,
        const mir::Vector3& origin = mir::Vector3::zero(),
        const mir::Transform& transform = mir::Transform::identity(),
        mir::BRepTolerance tolerance = mir::DefaultBRepTolerance)
    {
        const mir::BRepMakeBoxResult box = mir::BRepPrimAPI_MakeBox::build(
            brep, sizeX, sizeY, sizeZ, origin, tolerance);
        if (!box.success)
            return BRepSceneInsertResult{false, InvalidObjectId, {}, box.report};

        mir::BRepToModelOptions options{};
        options.tolerance = tolerance;
        options.validateBeforeConvert = true;
        const mir::BRepToModelResult converted = mir::BRepToModel::convertSolid(brep, box.solid, options);
        if (!converted.success)
            return BRepSceneInsertResult{false, InvalidObjectId, {}, converted.report};

        return insertModel(scene, converted.model, transform);
    }

    [[nodiscard]] static BRepSceneInsertResult createExtrudedRectangle(
        mir::Scene& scene,
        mir::BRepModel& brep,
        mir::Scalar width,
        mir::Scalar height,
        mir::Scalar depth,
        const mir::Vector3& origin = mir::Vector3::zero(),
        const mir::Vector3& direction = mir::Vector3::unitZ(),
        const mir::Transform& transform = mir::Transform::identity(),
        mir::BRepTolerance tolerance = mir::DefaultBRepTolerance)
    {
        const auto profile = mir::BRepMakeRectangleProfile::buildXY(
            brep, width, height, origin, tolerance);
        if (!profile.success)
            return BRepSceneInsertResult{false, InvalidObjectId, {}, profile.report};

        const auto extruded = mir::BRepExtrudeBuilder::extrudeWire(
            brep, profile.wire, direction, depth, tolerance);
        if (!extruded.success)
            return BRepSceneInsertResult{false, InvalidObjectId, {}, extruded.report};

        mir::BRepToModelOptions options{};
        options.tolerance = tolerance;
        const auto converted = mir::BRepToModel::convertSolid(brep, extruded.solid, options);
        if (!converted.success)
            return BRepSceneInsertResult{false, InvalidObjectId, {}, converted.report};

        return insertModel(scene, converted.model, transform);
    }
};

} // namespace mir4d
