#include "RenderSelectionPropertiesFormatter.hpp"

#include <iomanip>
#include <sstream>

namespace mir
{
namespace
{

[[nodiscard]] std::string number(double value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}

[[nodiscard]] std::string vector3(const RenderVec3& value)
{
    return "X: " + number(value.x) +
           "  Y: " + number(value.y) +
           "  Z: " + number(value.z);
}

} // namespace

std::vector<RenderPropertyRow> RenderSelectionPropertiesFormatter::format(
    const RenderSelectionProperties& properties)
{
    std::vector<RenderPropertyRow> rows;

    if (!properties.selection.valid())
        return rows;

    rows.push_back({"Тип", properties.selection.type == RenderSelectionType::Face ? "Грань" : "Объект"});
    rows.push_back({"ID", std::to_string(properties.selection.id)});
    rows.push_back({"Треугольников", std::to_string(properties.triangleCount)});
    rows.push_back({"Площадь", number(properties.area)});
    rows.push_back({"Центр", vector3(properties.center)});
    rows.push_back({"Нормаль", vector3(properties.normal)});

    return rows;
}

} // namespace mir
