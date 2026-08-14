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
// =================================================================================

#include "ShaderLibrary.h"
#include "Shader.h"                     // интерфейс Shader
#include <iostream>

namespace MirEngine {
namespace Rendering {

// ---------------------------------------------------------------------------------
// Конструктор: принимает фабрику для создания экземпляров Shader.
// ---------------------------------------------------------------------------------
ShaderLibrary::ShaderLibrary(ShaderFactory factory)
    : m_factory(std::move(factory))
{
    if (!m_factory) {
        std::cerr << "[ShaderLibrary] Factory function is null.\n";
    } else {
        (void)0;
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
            (void)0;
            return name;
        }
    }

    // Создаём новый шейдер через фабрику
    if (!m_factory) {
        std::cerr << "[ShaderLibrary] Cannot load shader '" << name << "': no factory provided.\n";
        return {}; // возвращаем пустой дескриптор
    }

    auto shader = m_factory();
    if (!shader) {
        std::cerr << "[ShaderLibrary] Factory failed to create shader '" << name << "'.\n";
        return {};
    }

    // Компилируем
    if (!shader->compile(vertexSource, fragmentSource)) {
        std::cerr << "[ShaderLibrary] Failed to compile shader '" << name << "'.\n";
        return {};
    }

    // Сохраняем в кэше
    m_shaders[name] = std::move(shader);
    (void)0;
    return name; // ShaderHandle это просто имя
}

// ---------------------------------------------------------------------------------
// Получение шейдера по дескриптору.
// Если шейдер не найден, возвращает зарегистрированный шейдер с именем "default"
// (fallback), чтобы рендерер продолжал работать вместо nullptr-разыменования.
// ---------------------------------------------------------------------------------
std::shared_ptr<Shader> ShaderLibrary::get(const ShaderHandle& handle) const
{
    auto it = m_shaders.find(handle);
    if (it != m_shaders.end()) {
        return it->second;
    }

    auto fallback = m_shaders.find("default");
    if (fallback != m_shaders.end()) {
        std::cerr << "[ShaderLibrary] Shader '" << handle << "' not found; using 'default' fallback.\n";
        return fallback->second;
    }

    std::cerr << "[ShaderLibrary] Shader '" << handle << "' not found and no 'default' fallback registered.\n";
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
        (void)0;
    } else {
        std::cerr << "[ShaderLibrary] Attempt to remove non-existent shader '" << handle << "'.\n";
    }
}

// ---------------------------------------------------------------------------------
// Очистка всей библиотеки.
// ---------------------------------------------------------------------------------
void ShaderLibrary::clear()
{
    m_shaders.clear();
    (void)0;
}

} // namespace Rendering
} // namespace MirEngine