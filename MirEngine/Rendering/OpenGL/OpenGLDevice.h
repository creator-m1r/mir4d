// MirEngine/Rendering/OpenGL/OpenGLDevice.h
// =================================================================================
// OpenGL implementation of the RenderDevice interface.
//
// Owns a pointer to the OpenGL context and the OpenGL state manager. Executes
// draw commands against the mesh / material caches and translates
// backend-neutral state overrides into GL calls.
// =================================================================================

#pragma once

#include "../Core/RenderDevice.h"
#include "OpenGLContext.h"
#include "OpenGLState.h"
#include "OpenGLVertexBuffer.h"
#include "OpenGLIndexBuffer.h"
#include "OpenGLVertexArray.h"

#include <unordered_map>

namespace MirEngine::Rendering {

class OpenGLDevice final : public RenderDevice {
public:
    // Takes an already-created context (does not own it).
    explicit OpenGLDevice(OpenGLContext* context);

    ~OpenGLDevice() override = default;

    OpenGLDevice(const OpenGLDevice&) = delete;
    OpenGLDevice& operator=(const OpenGLDevice&) = delete;

    // RenderDevice interface.
    bool initialize() override;

    void clear(const ColorRGBA& color,
               float depth = 1.0f,
               int stencil = 0,
               ClearFlags flags = ClearFlags::All) override;

    void draw(const RenderCommand& command) override;

    void present() override;

    void setViewportSize(std::uint32_t width, std::uint32_t height) override;

    void setViewMatrix(const Matrix4Raw& viewMatrix) override;
    void setProjectionMatrix(const Matrix4Raw& projMatrix) override;

    void setDepthTest(bool enabled) override { m_state.setDepthTest(enabled); }
    void setBlend(bool enabled) override { m_state.setBlend(enabled); }
    void setWireframe(bool enabled) override { m_state.setWireframe(enabled); }
    void setLineWidth(float width) override { m_state.setLineWidth(width); }
    void setCullFace(bool enabled) override { m_state.setCullFace(enabled); }
    void setDepthFunc(DepthFunc func) override { m_state.setDepthFunc(func); }

    std::shared_ptr<VertexBuffer> createVertexBuffer() override;
    std::shared_ptr<IndexBuffer> createIndexBuffer() override;
    std::shared_ptr<VertexArray> createVertexArray() override;

    void registerMesh(MeshHandle handle, std::shared_ptr<VertexArray> mesh) override;
    void registerMaterial(MaterialHandle handle, std::shared_ptr<Shader> shader) override;

    // Convenience frame hooks.
    void beginFrame();
    void endFrame();

    [[nodiscard]] OpenGLContext* context() const noexcept { return m_context; }
    [[nodiscard]] OpenGLState& state() noexcept { return m_state; }
    [[nodiscard]] const OpenGLState& state() const noexcept { return m_state; }

private:
    OpenGLContext* m_context{nullptr};
    OpenGLState m_state;

    // GPU resource caches indexed by MeshHandle / MaterialHandle.
    std::unordered_map<MeshHandle, std::shared_ptr<VertexArray>> m_meshes;
    std::unordered_map<MaterialHandle, std::shared_ptr<Shader>> m_materials;

    // Current matrices (loaded into shader uniforms at draw time).
    Matrix4Raw m_viewMatrix{IdentityMatrix4()};
    Matrix4Raw m_projectionMatrix{IdentityMatrix4()};
};

} // namespace MirEngine::Rendering