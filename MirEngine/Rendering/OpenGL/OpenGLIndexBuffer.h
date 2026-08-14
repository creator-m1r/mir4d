// MirEngine/Rendering/OpenGL/OpenGLIndexBuffer.h
// =================================================================================
// Конкретная реализация индексного буфера для OpenGL.
//
// Инкапсулирует нативный объект буфера (GL_ELEMENT_ARRAY_BUFFER) и методы
// загрузки целочисленных индексов (uint32_t). Используется совместно с
// OpenGLVertexBuffer и OpenGLVertexArray для индексированного рендеринга.
//
// Принцип изоляции:
//   Все вызовы gl* скрыты внутри этого класса и его .cpp.
//   Внешний код работает через интерфейс IndexBuffer, не зная о GLuint.
//
// Использование:
//   1. Создать экземпляр (обычно через фабрику в OpenGLDevice).
//   2. Вызвать uploadIndices() с массивом индексов.
//   3. Привязать к VertexArray через setIndexBuffer().
//   4. Рендерер при вызове draw будет использовать индексы из этого буфера.
// =================================================================================

// MirEngine/Rendering/OpenGL/OpenGLIndexBuffer.h
// =================================================================================
// OpenGL-реализация IndexBuffer.
// =================================================================================

#pragma once

#include "../Resources/IndexBuffer.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

class OpenGLIndexBuffer final : public IndexBuffer {
public:
    OpenGLIndexBuffer();
    ~OpenGLIndexBuffer() override;

    OpenGLIndexBuffer(const OpenGLIndexBuffer&) = delete;
    OpenGLIndexBuffer& operator=(const OpenGLIndexBuffer&) = delete;

    void uploadIndices(const std::vector<uint32_t>& indices,
                       BufferUsage usage = BufferUsage::Static) override;

    void uploadIndices(const uint32_t* data, size_t count,
                       BufferUsage usage = BufferUsage::Static) override;

    void bind()   override;
    void unbind() override;

    void upload(const void* data, size_t size,
                BufferUsage usage = BufferUsage::Static) override;

    [[nodiscard]] size_t getIndexCount() const override { return m_indexCount; }
    [[nodiscard]] size_t getSize()       const override;

    [[nodiscard]] bool isValid() const override {
        return m_handle != 0 && m_indexCount > 0;
    }

    // Нативный handle (для внутреннего использования)
    [[nodiscard]] GLuint handle() const noexcept { return m_handle; }

private:
    GLuint m_handle     = 0;
    size_t m_indexCount = 0;

    static GLenum usageToGL(BufferUsage usage);
};

} // namespace Rendering
} // namespace MirEngine