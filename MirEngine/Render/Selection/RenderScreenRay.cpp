#include "RenderScreenRay.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{
namespace
{

struct Vec4
{
    double x{};
    double y{};
    double z{};
    double w{};
};

[[nodiscard]] Vec4 multiply(const RenderMat4& matrix, Vec4 value) noexcept
{
    return {
        matrix.m[0] * value.x + matrix.m[4] * value.y + matrix.m[8] * value.z + matrix.m[12] * value.w,
        matrix.m[1] * value.x + matrix.m[5] * value.y + matrix.m[9] * value.z + matrix.m[13] * value.w,
        matrix.m[2] * value.x + matrix.m[6] * value.y + matrix.m[10] * value.z + matrix.m[14] * value.w,
        matrix.m[3] * value.x + matrix.m[7] * value.y + matrix.m[11] * value.z + matrix.m[15] * value.w};
}

[[nodiscard]] bool inverse(const RenderMat4& matrix, RenderMat4& out) noexcept
{
    double a[4][8]{};

    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 4; ++column)
            a[row][column] = matrix.m[column * 4 + row];
        a[row][row + 4] = 1.0;
    }

    for (int column = 0; column < 4; ++column)
    {
        int pivot = column;
        for (int row = column + 1; row < 4; ++row)
        {
            if (std::abs(a[row][column]) > std::abs(a[pivot][column]))
                pivot = row;
        }

        if (std::abs(a[pivot][column]) <= 1e-12)
            return false;

        if (pivot != column)
        {
            for (int j = 0; j < 8; ++j)
                std::swap(a[pivot][j], a[column][j]);
        }

        const double divisor = a[column][column];
        for (int j = 0; j < 8; ++j)
            a[column][j] /= divisor;

        for (int row = 0; row < 4; ++row)
        {
            if (row == column)
                continue;

            const double factor = a[row][column];
            for (int j = 0; j < 8; ++j)
                a[row][j] -= factor * a[column][j];
        }
    }

    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            out.m[column * 4 + row] = a[row][column + 4];

    return true;
}

[[nodiscard]] RenderVec3 normalize(RenderVec3 value) noexcept
{
    const double length = std::sqrt(
        value.x * value.x + value.y * value.y + value.z * value.z);

    if (length <= 1e-12)
        return {0.0, 0.0, -1.0};

    return {value.x / length, value.y / length, value.z / length};
}

} // namespace

RenderRay RenderScreenRayBuilder::fromScreen(
    double screenX,
    double screenY,
    double viewportWidth,
    double viewportHeight,
    const RenderCamera& camera) noexcept
{
    if (viewportWidth <= 0.0 || viewportHeight <= 0.0)
        return {camera.position(), {0.0, 0.0, -1.0}};

    const double ndcX = 2.0 * screenX / viewportWidth - 1.0;
    const double ndcY = 1.0 - 2.0 * screenY / viewportHeight;

    RenderMat4 viewProjection = RenderMat4::identity();
    const auto view = camera.viewMatrix();
    const auto projection = camera.projectionMatrix();

    for (int column = 0; column < 4; ++column)
    {
        for (int row = 0; row < 4; ++row)
        {
            double value = 0.0;
            for (int k = 0; k < 4; ++k)
                value += projection.m[k * 4 + row] * view.m[column * 4 + k];
            viewProjection.m[column * 4 + row] = value;
        }
    }

    RenderMat4 inverseViewProjection{};
    if (!inverse(viewProjection, inverseViewProjection))
        return {camera.position(), {0.0, 0.0, -1.0}};

    const auto nearPoint = multiply(inverseViewProjection, {ndcX, ndcY, -1.0, 1.0});
    const auto farPoint = multiply(inverseViewProjection, {ndcX, ndcY, 1.0, 1.0});

    if (std::abs(nearPoint.w) <= 1e-12 || std::abs(farPoint.w) <= 1e-12)
        return {camera.position(), {0.0, 0.0, -1.0}};

    const RenderVec3 nearWorld{
        nearPoint.x / nearPoint.w,
        nearPoint.y / nearPoint.w,
        nearPoint.z / nearPoint.w};

    const RenderVec3 farWorld{
        farPoint.x / farPoint.w,
        farPoint.y / farPoint.w,
        farPoint.z / farPoint.w};

    return {
        nearWorld,
        normalize({
            farWorld.x - nearWorld.x,
            farWorld.y - nearWorld.y,
            farWorld.z - nearWorld.z})};
}

} // namespace mir
