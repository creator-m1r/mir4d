// MirEngine/Rendering/Resources/Shader.h
// =================================================================================
// Абстрактный интерфейс шейдерной программы.
//
// Инкапсулирует этапы компиляции вершинного и фрагментного шейдера, линковки
// программы и установки uniform-переменных. Конкретные реализации (OpenGLShader,
// MetalShader, VulkanShader) будут скрывать вызовы соответствующего графического API.
//
// Архитектура:
//   Материал (Material) ссылается на Shader через дескриптор.
//   Рендерер перед выполнением draw-команды вызывает shader->bind(),
//   устанавливает uniform'ы (матрицы, параметры материала) и затем рисует геометрию.
//
// Правило изоляции:
//   Никаких GL/MTL/Vk-типов или вызовов. Внешний код видит только этот интерфейс.
//
// Использование:
//   1. Создать конкретный экземпляр (OpenGLShader).
//   2. Вызвать compile() с исходным кодом шейдеров.
//   3. При рисовании: bind() -> setMatrix/setVec3/setInt -> draw -> unbind().
// =================================================================================

#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <array>

// Импортируем Matrix4Raw, определённый в RenderCommand.h
#include "../Core/RenderCommand.h"

namespace MirEngine {
namespace Rendering {

class Shader {
public:
    virtual ~Shader() = default;

    // --------------------------------------------------------------------------
    // Компилирует вершинный и фрагментный шейдеры из исходного кода GLSL/HLSL/etc.
    // В случае ошибки компиляции/линковки возвращает false; подробности
    // должны выводиться через систему логирования (spdlog) в реализации.
    // --------------------------------------------------------------------------
    virtual bool compile(const std::string& vertexSource,
                         const std::string& fragmentSource) = 0;

    // --------------------------------------------------------------------------
    // Делает шейдерную программу активной.
    // --------------------------------------------------------------------------
    virtual void bind() = 0;

    // --------------------------------------------------------------------------
    // Отключает шейдерную программу.
    // --------------------------------------------------------------------------
    virtual void unbind() = 0;

    // --------------------------------------------------------------------------
    // Методы установки uniform-переменных.
    // Имена параметров соответствуют названиям в шейдерном коде.
    // Если переменная не найдена, реализация может проигнорировать вызов
    // или выдать предупреждение (но не крашиться).
    // --------------------------------------------------------------------------
    virtual void setInt(std::string_view name, int value) = 0;
    virtual void setFloat(std::string_view name, float value) = 0;
    virtual void setVec2(std::string_view name, float x, float y) = 0;
    virtual void setVec3(std::string_view name, float x, float y, float z) = 0;
    virtual void setVec4(std::string_view name, float x, float y, float z, float w) = 0;
    virtual void setMatrix(std::string_view name, const Matrix4Raw& matrix) = 0;
    virtual void setMatrix3(std::string_view name, const float* values) = 0;

protected:
    Shader() = default;
};

} // namespace Rendering
} // namespace MirEngine