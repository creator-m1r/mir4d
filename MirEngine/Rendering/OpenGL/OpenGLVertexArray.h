// MirEngine/Rendering/OpenGL/OpenGLVertexArray.h
// =================================================================================
// Конкретная реализация вершинного массива для OpenGL.
//
// Инкапсулирует объект Vertex Array Object (VAO) и связывает вершинный буфер
// (OpenGLVertexBuffer) и индексный буфер (OpenGLIndexBuffer) с описанием атрибутов
// вершин, соответствующих структуре MirEngine::Rendering::Vertex.
//
// Принцип изоляции:
//   Все вызовы gl* скрыты внутри этого класса и его .cpp. Внешний код работает
//   через интерфейс VertexArray, не зная о GLuint и внутренних состояниях OpenGL.
//   Исключением является метод getHandle(), необходимый для низкоуровневых операций
//   в OpenGLDevice, где требуется прямой доступ к нативному объекту VAO.
//
// Использование:
//   1. Создать экземпляр (обычно через фабрику в OpenGLDevice).
//   2. Привязать вершинный буфер через setVertexBuffer().
//   3. Привязать индексный буфер через setIndexBuffer() (опционально).
//   4. Вызвать bind() перед рисованием.
//   5. Передать массив в RenderDevice::draw().


#pragma once

#include "../Resources/VertexArray.h"
#include <memory>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

class OpenGLVertexArray final : public VertexArray {
public:
    OpenGLVertexArray();
    ~OpenGLVertexArray() override;

    OpenGLVertexArray(const OpenGLVertexArray&) = delete;
    OpenGLVertexArray& operator=(const OpenGLVertexArray&) = delete;

    void bind()   override;
    void unbind() override;

    void setVertexBuffer(std::shared_ptr<VertexBuffer> vb) override;
    void setIndexBuffer (std::shared_ptr<IndexBuffer>  ib) override;

    [[nodiscard]] std::shared_ptr<VertexBuffer> getVertexBuffer() const override {
        return m_vertexBuffer;
    }
    [[nodiscard]] std::shared_ptr<IndexBuffer> getIndexBuffer() const override {
        return m_indexBuffer;
    }

    [[nodiscard]] uint32_t getElementCount() const override;
    [[nodiscard]] bool     hasIndexBuffer()  const override {
        return m_indexBuffer != nullptr;
    }

    [[nodiscard]] bool isValid() const override {
        return m_vao != 0 && m_vertexBuffer != nullptr;
    }

    // Нативный handle (только для внутреннего использования)
    [[nodiscard]] GLuint handle() const noexcept { return m_vao; }

private:
    GLuint m_vao = 0;
    std::shared_ptr<VertexBuffer> m_vertexBuffer;
    std::shared_ptr<IndexBuffer>  m_indexBuffer;

    void setupAttributes();
};

} // namespace Rendering
} // namespace MirEngine