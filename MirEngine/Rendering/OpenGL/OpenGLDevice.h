// MirEngine/Rendering/OpenGL/OpenGLDevice.h
// =================================================================================
// Конкретная реализация RenderDevice для OpenGL.
//
// Владеет указателем на OpenGLContext и OpenGLState.
// Реализует все методы абстрактного интерфейса RenderDevice.
// =================================================================================

#pragma once

#include "../Core/RenderDevice.h"
#include "OpenGLContext.h"
#include "OpenGLState.h"
#include "OpenGLVertexBuffer.h"
#include "OpenGLIndexBuffer.h"
#include "OpenGLVertexArray.h"

#include <unordered_map>

namespace MirEngine {
namespace Rendering {

class OpenGLDevice final : public RenderDevice {
public:
    // Конструктор принимает уже созданный контекст (не владеет им).
    explicit OpenGLDevice(OpenGLContext* context);

    ~OpenGLDevice() override = default;

    // Запрет копирования
    OpenGLDevice(const OpenGLDevice&) = delete;
    OpenGLDevice& operator=(const OpenGLDevice&) = delete;

    // --------------------------------------------------------------------------
    // Реализация RenderDevice
    // --------------------------------------------------------------------------
    bool initialize() override;

    void clear(const ColorRGBA& color,
               float depth = 1.0f,
               int stencil = 0,
               ClearFlags flags = ClearFlags::All) override;

    void draw(const RenderCommand& command) override;

    void present() override;

    void setViewportSize(uint32_t width, uint32_t height) override;

    void setViewMatrix(const Matrix4Raw& viewMatrix) override;
    void setProjectionMatrix(const Matrix4Raw& projMatrix) override;

    std::shared_ptr<VertexBuffer> createVertexBuffer() override;
    std::shared_ptr<IndexBuffer>  createIndexBuffer() override;
    std::shared_ptr<VertexArray>  createVertexArray() override;

    void registerMesh(MeshHandle handle, std::shared_ptr<VertexArray> mesh) override;
    void registerMaterial(MaterialHandle handle, std::shared_ptr<Shader> shader) override;

    // --------------------------------------------------------------------------
    // Дополнительные удобные методы
    // --------------------------------------------------------------------------
    void beginFrame();
    void endFrame();

    [[nodiscard]] OpenGLContext* context() const noexcept { return m_context; }
    [[nodiscard]] OpenGLState&   state() noexcept { return m_state; }
    [[nodiscard]] const OpenGLState& state() const noexcept { return m_state; }

private:
    OpenGLContext* m_context = nullptr;
    OpenGLState    m_state;

    // Кеши GPU-ресурсов, индексируемые MeshHandle / MaterialHandle
    std::unordered_map<MeshHandle, std::shared_ptr<VertexArray>> m_meshes;
    std::unordered_map<MaterialHandle, std::shared_ptr<Shader>>  m_materials;

    // Текущие матрицы (пока просто хранятся, позже уйдут в UBO)
    Matrix4Raw m_viewMatrix       = IdentityMatrix4();
    Matrix4Raw m_projectionMatrix = IdentityMatrix4();
};

} // namespace Rendering
} // namespace MirEngine