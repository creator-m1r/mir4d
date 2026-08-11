// MirEngine/Rendering/Resources/IndexBuffer.h
// =================================================================================
// Интерфейс индексного буфера (Index Buffer).
//
// Индексный буфер хранит целочисленные индексы (uint32_t), которые определяют
// порядок соединения вершин в примитивы. Использование индексов значительно
// сокращает объём данных: одна вершина может быть использована несколькими
// треугольниками.
//
// Архитектура:
//   - Наследует GPUBuffer, реализуя bind/unbind и управление памятью GPU.
//   - Содержит знание о количестве индексов.
//   - Работает в связке с VertexBuffer через VertexArray (будет добавлен позже).
//
// Конкретная реализация (OpenGLIndexBuffer) будет находиться в Rendering/OpenGL/
// и использовать GL_ELEMENT_ARRAY_BUFFER.
//
// Правило изоляции:
//   Никаких GL-вызовов в этом заголовке; только абстрактный контракт.
// =================================================================================

// MirEngine/Rendering/Resources/IndexBuffer.h
// =================================================================================
// Абстрактный интерфейс индексного буфера.
// =================================================================================

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "GPUBuffer.h"

namespace MirEngine {
namespace Rendering {

class IndexBuffer : public GPUBuffer {
public:
    virtual ~IndexBuffer() = default;

    virtual void uploadIndices(const std::vector<uint32_t>& indices,
                               BufferUsage usage = BufferUsage::Static) = 0;

    virtual void uploadIndices(const uint32_t* data, size_t count,
                               BufferUsage usage = BufferUsage::Static) = 0;

    [[nodiscard]] virtual size_t getIndexCount() const = 0;
    [[nodiscard]] virtual size_t getSize()       const override = 0;

protected:
    IndexBuffer() = default;
};

} // namespace Rendering
} // namespace MirEngine