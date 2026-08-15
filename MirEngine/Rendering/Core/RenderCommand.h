// MirEngine/Rendering/Core/RenderCommand.h
// =================================================================================
// Command-based draw contract of the MirEngine rendering pipeline.
//
// Architecture:
//   - MirEngine forms RenderCommand objects from the engineering scene and
//     CAD logic. The renderer backend (OpenGL / Metal / Vulkan) interprets
//     each command and executes it on the GPU.
//   - The upper layer never touches GL/Metal calls; the lower layer never
//     depends on CAD types.
//
// A command carries:
//   - geometry descriptor (mesh / GPU buffers);
//   - material descriptor (shader + parameters);
//   - per-instance transform (model matrix);
//   - primitive type;
//   - pipeline state overrides (wireframe, blending, line width, depth test).
//
// MeshHandle and MaterialHandle are lightweight ids resolved by the renderer
// inside its resource caches.
// =================================================================================

#pragma once

#include <array>
#include <cstdint>

namespace MirEngine::Rendering {

// -----------------------------------------------------------------------------
// Primitive types supported by the renderer.
// They map to OpenGL / Metal / Vulkan primitive constants.
// -----------------------------------------------------------------------------
enum class PrimitiveType : std::uint8_t {
    Points        = 0,  // point cloud
    Lines         = 1,  // GL_LINES
    LineStrip     = 2,  // GL_LINE_STRIP
    LineLoop      = 3,  // GL_LINE_LOOP
    Triangles     = 4,  // GL_TRIANGLES
    TriangleStrip = 5,  // GL_TRIANGLE_STRIP
    TriangleFan   = 6   // GL_TRIANGLE_FAN
};

// -----------------------------------------------------------------------------
// Geometry handle: unique id of a VertexArray / buffer set in the renderer cache.
// -----------------------------------------------------------------------------
using MeshHandle = std::uint32_t;

// -----------------------------------------------------------------------------
// Material handle: id of a shader / material state in the renderer cache.
// -----------------------------------------------------------------------------
using MaterialHandle = std::uint32_t;

// -----------------------------------------------------------------------------
// 4x4 matrix stored as a 16-float array in column-major order (OpenGL layout,
// compatible with GLSL mat4). Will be replaced by the canonical Matrix4 from
// MirEngine/Math once the rendering layer fully migrates to it.
// -----------------------------------------------------------------------------
using Matrix4Raw = std::array<float, 16>;

// -----------------------------------------------------------------------------
// Identity matrix (constexpr).
// -----------------------------------------------------------------------------
inline constexpr Matrix4Raw IdentityMatrix4() noexcept
{
    return {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
}

// -----------------------------------------------------------------------------
// Per-command pipeline state overrides.
// Defaults match the standard CAD look: depth test on, blending off.
// -----------------------------------------------------------------------------
struct RenderStateFlags {
    bool depthTest{true};
    bool blend{false};
    bool cullBackFaces{true};
    bool wireframe{false};
    float lineWidth{1.0f};

    constexpr RenderStateFlags() noexcept = default;
};

// -----------------------------------------------------------------------------
// A single draw command: minimal information needed to draw one object.
// The renderer may sort, batch and merge commands.
// -----------------------------------------------------------------------------
struct RenderCommand {
    MeshHandle mesh{0};                                  // geometry
    MaterialHandle material{0};                          // shader / material
    Matrix4Raw modelMatrix{IdentityMatrix4()};           // model -> world
    PrimitiveType primitive{PrimitiveType::Triangles};
    RenderStateFlags state{};
    std::uint32_t firstIndex{0};                         // index buffer offset
    std::uint32_t indexCount{0};                         // 0 = whole mesh
};

} // namespace MirEngine::Rendering