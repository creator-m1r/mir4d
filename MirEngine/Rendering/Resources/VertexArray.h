// MirEngine/Rendering/Resources/VertexArray.h
// =================================================================================
// Интерфейс вершинного массива (Vertex Array).
//
// Вершинный массив связывает один вершинный буфер и опциональный индексный буфер
// с описанием структуры вершинных атрибутов (позиция, нормаль, текстурные координаты).
// Он хранит состояние привязок и используется рендерером для выполнения draw-команд.
//
// В OpenGL это соответствует VAO (Vertex Array Object). В Metal — MTLVertexDescriptor.
// В Vulkan — VkPipelineVertexInputStateCreateInfo. Во всех случаях задача одна:
//   "Вот буфер с вершинами, вот индексы, вот как читать вершины — рисуй".
//
// Архитектура:
//   - Создаётся после VertexBuffer и IndexBuffer.
//   - Метод setVertexBuffer() привязывает буфер вершин и настраивает атрибуты.
//   - Метод setIndexBuffer() привязывает буфер индексов.
//   - Метод bind() делает массив активным для последующего рисования.
//   - После связывания RenderDevice::draw() использует этот массив.
//
// Правило изоляции:
//   Никаких упоминаний OpenGL/Metal/Vulkan. Конкретная реализация
//   (OpenGLVertexArray) будет в Rendering/OpenGL/.


#pragma once

#include <memory>
#include <cstdint>

#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace MirEngine {
namespace Rendering {

class VertexArray {
public:
    virtual ~VertexArray() = default;

    virtual void bind()   = 0;
    virtual void unbind() = 0;

    virtual void setVertexBuffer(std::shared_ptr<VertexBuffer> vb) = 0;
    virtual void setIndexBuffer (std::shared_ptr<IndexBuffer>  ib) = 0;

    [[nodiscard]] virtual std::shared_ptr<VertexBuffer> getVertexBuffer() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IndexBuffer>  getIndexBuffer()  const = 0;

    // Количество элементов для рисования (индексы или вершины)
    [[nodiscard]] virtual uint32_t getElementCount() const = 0;

    // Есть ли индексный буфер
    [[nodiscard]] virtual bool hasIndexBuffer() const = 0;

    [[nodiscard]] virtual bool isValid() const = 0;

protected:
    VertexArray() = default;
};

} // namespace Rendering
} // namespace MirEngine