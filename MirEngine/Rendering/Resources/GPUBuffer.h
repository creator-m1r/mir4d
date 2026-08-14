// MirEngine/Rendering/Resources/GPUBuffer.h
// =================================================================================
// Базовый интерфейс для графического буфера общего назначения.
//
// Определяет минимальный контракт для любого буфера, хранящегося в GPU-памяти.
// Конкретные типы буферов (VertexBuffer, IndexBuffer, UniformBuffer и т.д.)
// наследуются от этого интерфейса и добавляют специфику (тип, layout, частоту обновления).
//
// Основные обязанности:
//   - Предоставлять метод для отправки данных на GPU.
//   - Управлять временем жизни нативного ресурса (автоматически при уничтожении).
//   - Уметь связываться (bind) с конвейером GPU (например, GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER).
//
// Этот интерфейс полностью скрывает OpenGL/Metal/Vulkan.
// Конкретные реализации создаются через фабрики или специализированные классы.
//
// Замечание по использованию:
//   Обычно буфер создаётся один раз (через RenderDevice или ResourceManager),
//   после чего данные загружаются через upload(). Методы bind/unbind
//   вызываются рендерером во время выполнения команд рисования.
// =================================================================================

// MirEngine/Rendering/Resources/GPUBuffer.h
// =================================================================================
// Базовый интерфейс GPU-буфера.
// =================================================================================

#pragma once

#include <cstddef>
#include <cstdint>

namespace MirEngine {
namespace Rendering {

enum class BufferUsage : uint8_t {
    Static  = 0,  // данные задаются один раз
    Dynamic = 1,  // данные меняются время от времени
    Stream  = 2   // данные меняются каждый кадр
};

class GPUBuffer {
public:
    virtual ~GPUBuffer() = default;

    virtual void bind()   = 0;
    virtual void unbind() = 0;

    // Загрузка сырых данных
    virtual void upload(const void* data, size_t size,
                        BufferUsage usage = BufferUsage::Static) = 0;

    [[nodiscard]] virtual size_t getSize()  const = 0;
    [[nodiscard]] virtual bool   isValid()  const = 0;

protected:
    GPUBuffer() = default;
};

} // namespace Rendering
} // namespace MirEngine