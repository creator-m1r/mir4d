// MirEngine/Rendering/OpenGL/OpenGLVertexBuffer.h
// =================================================================================
// Конкретная реализация вершинного буфера для OpenGL.
//
// Инкапсулирует нативный объект буфера (GL_ARRAY_BUFFER) и предоставляет методы
// для загрузки вершин, соответствующих структуре MirEngine::Rendering::Vertex.
//
// Принцип изоляции:
//   Это единственное место (вместе с .cpp), где появляются вызовы gl*.
//   Внешний код работает через интерфейс VertexBuffer и не видит GLuint.
//   Метод getHandle() предоставляется для внутреннего использования движком
//   (OpenGLVertexArray, отладочные инструменты) и не является частью интерфейса.
//
// Использование (через интерфейс VertexBuffer):
//   1. Создать через OpenGLDevice или фабрику.
//   2. Вызвать uploadVertices() с массивом Vertex.
//   3. Привязать к VertexArray через setVertexBuffer().
//   4. Рендерер использует VertexArray, внутри которого буфер автоматически
//      подключается к GL.
// =================================================================================
// MirEngine/Rendering/OpenGL/OpenGLVertexBuffer.h
// =================================================================================
// OpenGL-реализация VertexBuffer.
// =================================================================================

#pragma once

#include "../Resources/VertexBuffer.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

class OpenGLVertexBuffer final : public VertexBuffer {
public:
    OpenGLVertexBuffer();
    ~OpenGLVertexBuffer() override;

    OpenGLVertexBuffer(const OpenGLVertexBuffer&) = delete;
    OpenGLVertexBuffer& operator=(const OpenGLVertexBuffer&) = delete;

    void uploadVertices(const std::vector<Vertex>& vertices,
                        BufferUsage usage = BufferUsage::Static) override;

    void uploadVertices(const Vertex* data, size_t count,
                        BufferUsage usage = BufferUsage::Static) override;

    void bind()   override;
    void unbind() override;

    [[nodiscard]] size_t getVertexCount() const override { return m_vertexCount; }
    [[nodiscard]] size_t getVertexSize()  const override { return sizeof(Vertex); }
    [[nodiscard]] size_t getSize()        const override;

    [[nodiscard]] bool isValid() const override {
        return m_handle != 0 && m_vertexCount > 0;
    }

    // Нативный handle (только для внутреннего использования)
    [[nodiscard]] GLuint handle() const noexcept { return m_handle; }

private:
    GLuint m_handle      = 0;
    size_t m_vertexCount = 0;

    static GLenum usageToGL(BufferUsage usage);
};

} // namespace Rendering
} // namespace MirEngine