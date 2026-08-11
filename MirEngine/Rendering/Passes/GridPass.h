// MirEngine/Rendering/Passes/GridPass.h
// =================================================================================
// Проход рендеринга координатной сетки и осей (Grid Pass).
//
// Отвечает за отрисовку:
//   - XY-сетки на плоскости Z=0 (основная рабочая плоскость CAD).
//   - Трёх цветных осей (X — красный, Y — зелёный, Z — синий) со стрелками.
//   - Подписей осей (будущее расширение).
//
// Сетка строится из линий с учётом текущей камеры: шаг сетки адаптируется к
// расстоянию, чтобы не перегружать сцену мелкими делениями. Сетка рисуется
// полупрозрачной, чтобы не перебивать геометрию.
//
// Архитектура:
//   - Создаёт собственные VAO/VBO для линий сетки и осей при initialize().
//   - Регистрирует их в устройстве под фиксированными дескрипторами.
//   - В execute() формирует RenderCommand с соответствующим материалом и рисует.
//
// Шейдеры:
//   - Использует простой одноцветный шейдер "SolidColor".
//   - Цвет передаётся через uniform.
//
// Изоляция:
//   - Никаких прямых OpenGL-вызовов, только RenderDevice и его интерфейсы.
// =================================================================================

// MirEngine/Rendering/Passes/GridPass.h
// =================================================================================
// Проход отрисовки координатной сетки и осей.
//
// Рисует:
//   • Основную сетку (крупный шаг)
//   • Мелкую сетку (подразделения)
//   • Цветные оси X (красный), Y (зелёный), Z (синий)
//
// Не зависит от Scene/Camera напрямую — берёт матрицы из RenderContext.
// Геометрия генерируется один раз при initialize() и хранится в GPU-буферах.
// =================================================================================


#pragma once

#include "RenderPass.h"
#include "../Core/RenderCommand.h"
#include "../Resources/Vertex.h"
#include <memory>
#include <vector>
#include <cstdint>

namespace MirEngine {
namespace Rendering {

class Shader;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class OpenGLShader;   // конкретная реализация

class GridPass final : public RenderPass {
public:
    GridPass();
    ~GridPass() override;

    // Создаёт геометрию сетки и компилирует шейдер.
    // Должен быть вызван после того, как OpenGL-контекст стал текущим.
    bool initialize();

    // Основной метод прохода
    void execute(RenderContext& context,
                 Scene& scene,
                 Camera& camera,
                 RenderDevice& device) override;

    // Настройки сетки
    void setGridSize(float size)        noexcept { m_gridSize = size; }
    void setMajorStep(float step)       noexcept { m_majorStep = step; }
    void setMinorDivisions(int divs)    noexcept { m_minorDivisions = divs; }
    void setFadeDistance(float dist)    noexcept { m_fadeDistance = dist; }

    void setShowGrid(bool show)         noexcept { m_showGrid = show; }
    void setShowAxes(bool show)         noexcept { m_showAxes = show; }

    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    bool m_initialized = false;

    // Параметры
    float m_gridSize        = 20.0f;   // половина размера сетки
    float m_majorStep       = 1.0f;    // шаг основных линий
    int   m_minorDivisions  = 10;      // подразделений между major
    float m_fadeDistance    = 25.0f;   // расстояние затухания

    bool  m_showGrid = true;
    bool  m_showAxes = true;

    // GPU-ресурсы
    std::shared_ptr<VertexArray>  m_gridVAO;
    std::shared_ptr<VertexBuffer> m_gridVBO;
    uint32_t m_gridVertexCount = 0;

    std::shared_ptr<VertexArray>  m_axesVAO;
    std::shared_ptr<VertexBuffer> m_axesVBO;
    uint32_t m_axesVertexCount = 0;

    std::unique_ptr<OpenGLShader> m_shader;

    // Генерация геометрии
    void buildGridGeometry();
    void buildAxesGeometry();
    bool createShader();
};

} // namespace Rendering
} // namespace MirEngine