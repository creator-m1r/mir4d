#include "SelectionInspectorBridge.hpp"

#include "../Widgets/PropertyGrid/SelectionPropertiesPanel.hpp"
#include "../../MirEngine/Render/Selection/RenderSelectionProperties.hpp"

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

struct MirUISelectionInspectorHandle
{
    MirUI::SelectionPropertiesPanel panel{};
    std::vector<MirUI::Property> properties{};
    std::vector<std::string> names{};
    std::vector<std::string> values{};
};

namespace
{

[[nodiscard]] std::string stateValueToString(const MirUI::StateValue& value)
{
    return std::visit([](const auto& item) -> std::string
    {
        using T = std::decay_t<decltype(item)>;

        if constexpr (std::is_same_v<T, bool>)
            return item ? "true" : "false";
        else if constexpr (std::is_same_v<T, std::string>)
            return item;
        else if constexpr (std::is_same_v<T, std::int64_t>)
            return std::to_string(item);
        else if constexpr (std::is_same_v<T, double>)
        {
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(3) << item;
            return stream.str();
        }
    }, value);
}

void rebuildCache(MirUISelectionInspectorHandle* handle)
{
    handle->properties = handle->panel.grid().properties();
    handle->names.clear();
    handle->values.clear();

    handle->names.reserve(handle->properties.size());
    handle->values.reserve(handle->properties.size());

    for (const auto& property : handle->properties)
    {
        handle->names.push_back(property.name);
        handle->values.push_back(stateValueToString(property.value));
    }
}

} // namespace

extern "C" MirUISelectionInspectorHandle*
mirui_selection_inspector_create(void)
{
    return new MirUISelectionInspectorHandle{};
}

extern "C" void
mirui_selection_inspector_destroy(MirUISelectionInspectorHandle* handle)
{
    delete handle;
}

extern "C" void
mirui_selection_inspector_clear(MirUISelectionInspectorHandle* handle)
{
    if (handle == nullptr)
        return;

    handle->panel.clear();
    handle->properties.clear();
    handle->names.clear();
    handle->values.clear();
}

extern "C" void
mirui_selection_inspector_update_face(
    MirUISelectionInspectorHandle* handle,
    std::uint64_t id,
    std::uint64_t triangleCount,
    double area,
    double centerX,
    double centerY,
    double centerZ,
    double normalX,
    double normalY,
    double normalZ)
{
    if (handle == nullptr || id == 0)
        return;

    mir::RenderSelectionProperties properties;
    properties.selection.type = mir::RenderSelectionType::Face;
    properties.selection.id = id;
    properties.triangleCount = static_cast<std::size_t>(triangleCount);
    properties.area = area;
    properties.center = {centerX, centerY, centerZ};
    properties.normal = {normalX, normalY, normalZ};

    handle->panel.update(properties);
    rebuildCache(handle);
}

extern "C" std::size_t
mirui_selection_inspector_property_count(
    const MirUISelectionInspectorHandle* handle)
{
    return handle == nullptr ? 0 : handle->names.size();
}

extern "C" const char*
mirui_selection_inspector_property_name(
    const MirUISelectionInspectorHandle* handle,
    std::size_t index)
{
    if (handle == nullptr || index >= handle->names.size())
        return nullptr;

    return handle->names[index].c_str();
}

extern "C" const char*
mirui_selection_inspector_property_value(
    const MirUISelectionInspectorHandle* handle,
    std::size_t index)
{
    if (handle == nullptr || index >= handle->values.size())
        return nullptr;

    return handle->values[index].c_str();
}
