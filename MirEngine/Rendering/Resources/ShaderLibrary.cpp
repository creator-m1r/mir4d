// MirEngine/Rendering/Resources/ShaderLibrary.cpp
// =================================================================================
// Реализация библиотеки шейдеров.
// 
// Хранит скомпилированные шейдеры, предоставляя к ним доступ по строковому
// дескриптору (имени). Создание экземпляров Shader делегируется фабричной
// функции, переданной в конструктор, что позволяет использовать OpenGL, Metal
// или другой бэкенд без изменения этого файла.
//
// Зависимости:
//   - Shader.h (абстрактный интерфейс)
//   - spdlog (логирование)
// =================================================================================

#include "ShaderLibrary.h"
#include "Shader.h"                     // интерфейс Shader
#include <spdlog/spdlog.h>

namespace MirEngine {
namespace Rendering {

// ---------------------------------------------------------------------------------
// Конструктор: принимает фабрику для создания экземпляров Shader.
// ---------------------------------------------------------------------------------
ShaderLibrary::ShaderLibrary(ShaderFactory factory)
    : m_factory(std::move(factory))
{
    if (!m_factory) {
        spdlog::error("[ShaderLibrary] Factory function is null.");
    } else {
        spdlog::debug("[ShaderLibrary] Initialized with a valid factory.");
    }
}

// ---------------------------------------------------------------------------------
// Загрузка (компиляция) шейдера.
// Если шейдер с таким именем уже есть, не перекомпилирует (если не forceReload).
// Возвращает имя шейдера как дескриптор.
// ---------------------------------------------------------------------------------
ShaderHandle ShaderLibrary::load(const std::string& name,
                                 const std::string& vertexSource,
                                 const std::string& fragmentSource,
                                 bool forceReload)
{
    // Проверяем, существует ли уже шейдер
    if (!forceReload) {
        auto it = m_shaders.find(name);
        if (it != m_shaders.end()) {
            spdlog::info("[ShaderLibrary] Shader '{}' already loaded, skipping.", name);
            return name;
        }
    }

    // Создаём новый шейдер через фабрику
    if (!m_factory) {
        spdlog::error("[ShaderLibrary] Cannot load shader '{}': no factory provided.", name);
        return {}; // возвращаем пустой дескриптор
    }

    auto shader = m_factory();
    if (!shader) {
        spdlog::error("[ShaderLibrary] Factory failed to create shader '{}'.", name);
        return {};
    }

    // Компилируем
    if (!shader->compile(vertexSource, fragmentSource)) {
        spdlog::error("[ShaderLibrary] Failed to compile shader '{}'.", name);
        return {};
    }

    // Сохраняем в кэше
    m_shaders[name] = std::move(shader);
    spdlog::info("[ShaderLibrary] Shader '{}' loaded and cached.", name);
    return name; // ShaderHandle это просто имя
}

// ---------------------------------------------------------------------------------
// Получение шейдера по дескриптору.
// ---------------------------------------------------------------------------------
std::shared_ptr<Shader> ShaderLibrary::get(const ShaderHandle& handle) const
{
    auto it = m_shaders.find(handle);
    if (it != m_shaders.end()) {
        return it->second;
    }
    spdlog::warn("[ShaderLibrary] Shader '{}' not found.", handle);
    return nullptr;
}

// ---------------------------------------------------------------------------------
// Проверка наличия шейдера.
// ---------------------------------------------------------------------------------
bool ShaderLibrary::contains(const ShaderHandle& handle) const
{
    return m_shaders.find(handle) != m_shaders.end();
}

// ---------------------------------------------------------------------------------
// Удаление шейдера из кэша (приведёт к освобождению ресурсов).
// ---------------------------------------------------------------------------------
void ShaderLibrary::remove(const ShaderHandle& handle)
{
    auto it = m_shaders.find(handle);
    if (it != m_shaders.end()) {
        m_shaders.erase(it);
        spdlog::debug("[ShaderLibrary] Shader '{}' removed.", handle);
    } else {
        spdlog::warn("[ShaderLibrary] Attempt to remove non-existent shader '{}'.", handle);
    }
}

// ---------------------------------------------------------------------------------
// Очистка всей библиотеки.
// ---------------------------------------------------------------------------------
void ShaderLibrary::clear()
{
    m_shaders.clear();
    spdlog::debug("[ShaderLibrary] All shaders cleared.");
}

} // namespace Rendering
} // namespace MirEngine