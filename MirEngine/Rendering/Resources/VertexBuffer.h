// MirEngine/Rendering/Resources/VertexBuffer.h
// =================================================================================
// Интерфейс вершинного буфера (Vertex Buffer).
//
// Этот класс расширяет базовый GPUBuffer и добавляет знание о типе хранимых вершин 
// (MirEngine::Rendering::Vertex) и их количестве. Он является ключевым элементом 
// при передаче геометрии на GPU.
//
// Архитектура:
//   MirEngine строит геометрию в оперативной памяти (например, из CAD-модели),
//   затем передаёт массив Vertex через VertexBuffer::upload().
//   Renderer использует VertexBuffer совместно с IndexBuffer и VertexArray
//   для выполнения команд рисования.
//
// Конкретная реализация (OpenGLVertexBuffer) будет находиться в Rendering/OpenGL/
// и займётся вызовами glGenBuffers, glBindBuffer, glBufferData и т.п.
//
// Абстракция соблюдает правило:
//   "Никаких GL-функций в заголовках Rendering/Resources/".
// =================================================================================
// MirEngine/Rendering/Resources/VertexBuffer.h
// =================================================================================
// Абстрактный интерфейс вершинного буфера.
// =================================================================================

#pragma once

#include <cstddef>
#include <vector>

#include "GPUBuffer.h"
#include "Vertex.h"

namespace MirEngine {
namespace Rendering {

class VertexBuffer : public GPUBuffer {
public:
    virtual ~VertexBuffer() = default;

    // Загрузка из вектора
    virtual void uploadVertices(const std::vector<Vertex>& vertices,
                                BufferUsage usage = BufferUsage::Static) = 0;

    // Загрузка из сырого указателя
    virtual void uploadVertices(const Vertex* data, size_t count,
                                BufferUsage usage = BufferUsage::Static) = 0;

    [[nodiscard]] virtual size_t getVertexCount() const = 0;
    [[nodiscard]] virtual size_t getVertexSize()  const = 0;
    [[nodiscard]] virtual size_t getSize()        const override = 0;

protected:
    VertexBuffer() = default;
};

} // namespace Rendering
} // namespace MirEngine