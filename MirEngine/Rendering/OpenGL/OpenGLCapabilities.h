// MirEngine/Rendering/OpenGL/OpenGLCapabilities.h
// =================================================================================
// Сбор информации о возможностях текущего OpenGL-контекста.
//
// Предоставляет статические методы для получения:
//   - Версии OpenGL и GLSL
//   - Производителя GPU
//   - Максимальных размеров текстур, количества сэмплов (MSAA)
//   - Поддержки анизотропной фильтрации
//   - Доступных расширений (список)
//
// Эти данные используются для валидации и выбора оптимальных путей в рендерере.
// Например, если GPU поддерживает только OpenGL 4.1, нельзя использовать функции 4.5.
//
// Архитектура:
//   - Не зависит от платформы, только от OpenGL API.
//   - Вызывается после инициализации контекста (в OpenGLDevice::initialize).
// =================================================================================

// MirEngine/Rendering/OpenGL/OpenGLCapabilities.h
// =================================================================================
// Сбор информации о возможностях текущего OpenGL-контекста.
// =================================================================================

#pragma once

#include <string>
#include <vector>

namespace MirEngine {
namespace Rendering {

struct CapabilitiesInfo {
    std::string vendor;
    std::string renderer;
    std::string version;
    std::string shadingLanguageVersion;

    int  maxTextureSize     = 0;
    int  maxSamples         = 0;
    int  maxAnisotropy      = 1;
    bool anisotropySupported = false;

    std::vector<std::string> extensions;
};

class OpenGLCapabilities {
public:
    // Собирает информацию о текущем активном контексте.
    static CapabilitiesInfo query();

    // Проверяет наличие расширения.
    static bool isExtensionSupported(const std::string& name);
};

} // namespace Rendering
} // namespace MirEngine