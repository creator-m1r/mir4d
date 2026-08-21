#include "TransformAdapter.hpp"

namespace MirUI
{

std::vector<TransformPropertyRow> TransformAdapter::makeProperties(
    const TransformProperties& properties)
{
    const auto& transform = properties.transform;

    return {
        {"Position.X", transform.position.x},
        {"Position.Y", transform.position.y},
        {"Position.Z", transform.position.z},
        {"Rotation.X", transform.rotation.x},
        {"Rotation.Y", transform.rotation.y},
        {"Rotation.Z", transform.rotation.z},
        {"Rotation.W", transform.rotation.w},
        {"Scale.X", transform.scale.x},
        {"Scale.Y", transform.scale.y},
        {"Scale.Z", transform.scale.z}
    };
}

}
