
#pragma once

#include <array>
#include <cstdint>

namespace MirEngine::Rendering {

enum class PrimitiveType : std::uint8_t {
    Points        = 0,
    Lines         = 1,
    LineStrip     = 2,
    LineLoop      = 3,
    Triangles     = 4,
    TriangleStrip = 5,
    TriangleFan   = 6
};

using MeshHandle = std::uint32_t;

using MaterialHandle = std::uint32_t;

using Matrix4Raw = std::array<float, 16>;

inline constexpr Matrix4Raw IdentityMatrix4() noexcept
{
    return {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
}

struct RenderStateFlags {
    bool depthTest{true};
    bool blend{false};
    bool cullBackFaces{true};
    bool wireframe{false};
    float lineWidth{1.0f};

    constexpr RenderStateFlags() noexcept = default;
};

struct RenderCommand {
    MeshHandle mesh{0};
    MaterialHandle material{0};
    Matrix4Raw modelMatrix{IdentityMatrix4()};
    PrimitiveType primitive{PrimitiveType::Triangles};
    RenderStateFlags state{};
    std::uint32_t firstIndex{0};
    std::uint32_t indexCount{0};
};

}