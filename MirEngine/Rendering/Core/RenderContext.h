// MirEngine/Rendering/Core/RenderContext.h
// =================================================================================
// Контекст текущего кадра рендеринга.
//
// Содержит все данные, которые передаются от сцены / камеры в рендерер
// и используются RenderPass'ами.
//
// Ключевое правило:
//   RenderContext НЕ зависит от OpenGL, Metal, Vulkan.
//   Он хранит только математические и геометрические величины.
//   Рендерер читает эти значения и транслирует их в вызовы GPU.
//
// Жизненный цикл:
//   1. beginFrame() в начале кадра
//   2. updateMatrices() / setCameraPosition() из камеры
//   3. Использование всеми RenderPass'ами
//   4. endFrame() в конце кадра
// =================================================================================

#pragma once

#include <cstdint>
#include <array>
#include "RenderCommand.h"   // Matrix4Raw и IdentityMatrix4

namespace MirEngine {
namespace Rendering {

// ---------------------------------------------------------------------------------
// Основной класс RenderContext
// ---------------------------------------------------------------------------------
class RenderContext {
public:
    // ==========================================================================
    // Матрицы
    // ==========================================================================

    // Матрица вида (World → Camera)
    Matrix4Raw viewMatrix = IdentityMatrix4();

    // Матрица проекции (Camera → Clip)
    Matrix4Raw projectionMatrix = IdentityMatrix4();

    // Предвычисленное произведение projection * view
    Matrix4Raw viewProjectionMatrix = IdentityMatrix4();

    // ==========================================================================
    // Параметры камеры
    // ==========================================================================

    // Позиция наблюдателя в мировых координатах
    float cameraPosition[3] = { 0.0f, 0.0f, 0.0f };

    // Плоскости отсечения
    float nearPlane = 0.1f;
    float farPlane  = 1000.0f;

    // ==========================================================================
    // Размеры вьюпорта
    // ==========================================================================

    uint32_t viewportWidth  = 0;
    uint32_t viewportHeight = 0;
    float    aspectRatio    = 1.0f;

    // ==========================================================================
    // Временные параметры кадра
    // ==========================================================================

    float    deltaTime   = 0.0f;   // время с предыдущего кадра (сек)
    float    totalTime   = 0.0f;   // общее время работы (сек)
    uint64_t frameNumber = 0;      // номер текущего кадра

    // ==========================================================================
    // Методы
    // ==========================================================================

    // Подготавливает контекст к новому кадру.
    // Обновляет размеры, deltaTime, totalTime и инкрементирует frameNumber.
    void beginFrame(float dt, uint32_t width, uint32_t height) {
        deltaTime      = dt;
        totalTime     += dt;
        viewportWidth  = width;
        viewportHeight = height;
        aspectRatio    = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
        ++frameNumber;
    }

    // Завершает кадр (пока пусто, место для будущей синхронизации).
    void endFrame() noexcept {
        // В будущем: fence, query, очистка временных данных
    }

    // Обновляет матрицы и автоматически пересчитывает viewProjectionMatrix.
    void updateMatrices(const Matrix4Raw& view, const Matrix4Raw& proj) {
        viewMatrix           = view;
        projectionMatrix     = proj;
        viewProjectionMatrix = multiplyMatrices(proj, view);
    }

    // Устанавливает позицию камеры.
    void setCameraPosition(float x, float y, float z) noexcept {
        cameraPosition[0] = x;
        cameraPosition[1] = y;
        cameraPosition[2] = z;
    }

    // Устанавливает плоскости отсечения.
    void setClippingPlanes(float nearVal, float farVal) noexcept {
        nearPlane = nearVal;
        farPlane  = farVal;
    }

    // Сбрасывает все значения к начальным.
    void reset() noexcept {
        viewMatrix           = IdentityMatrix4();
        projectionMatrix     = IdentityMatrix4();
        viewProjectionMatrix = IdentityMatrix4();
        cameraPosition[0] = cameraPosition[1] = cameraPosition[2] = 0.0f;
        nearPlane       = 0.1f;
        farPlane        = 1000.0f;
        viewportWidth   = 0;
        viewportHeight  = 0;
        aspectRatio     = 1.0f;
        deltaTime       = 0.0f;
        totalTime       = 0.0f;
        frameNumber     = 0;
    }

private:
    // Умножение двух матриц 4×4 (column-major).
    // В будущем заменится на Matrix4 из Math.
    static Matrix4Raw multiplyMatrices(const Matrix4Raw& a, const Matrix4Raw& b) noexcept {
        Matrix4Raw result{};
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += a[row + k * 4] * b[k + col * 4];
                }
                result[row + col * 4] = sum;
            }
        }
        return result;
    }
};

} // namespace Rendering
} // namespace MirEngine