
#include "ShaderLibrary.h"
#include "Shader.h"

#include <iostream>

namespace MirEngine::Rendering {

ShaderLibrary::ShaderLibrary(ShaderFactory factory)
    : m_factory(std::move(factory))
{
    if (!m_factory)
    {
        std::cerr << "[ShaderLibrary] Factory function is null; "
                     "load() calls will fail.\n";
    }
}

ShaderHandle ShaderLibrary::load(const std::string& name,
                                 const std::string& vertexSource,
                                 const std::string& fragmentSource,
                                 bool forceReload)
{
    if (!forceReload)
    {
        const auto it = m_shaders.find(name);
        if (it != m_shaders.end())
        {
            return name;
        }
    }

    if (!m_factory)
    {
        std::cerr << "[ShaderLibrary] Cannot load shader '" << name
                  << "': no factory provided.\n";
        return {};
    }

    auto shader = m_factory();
    if (!shader)
    {
        std::cerr << "[ShaderLibrary] Factory failed to create shader '"
                  << name << "'.\n";
        return {};
    }

    if (!shader->compile(vertexSource, fragmentSource))
    {
        std::cerr << "[ShaderLibrary] Failed to compile shader '" << name
                  << "'.\n";
        return {};
    }

    m_shaders[name] = std::move(shader);
    return name;
}

std::shared_ptr<Shader> ShaderLibrary::get(const ShaderHandle& handle) const
{
    const auto it = m_shaders.find(handle);
    if (it != m_shaders.end())
    {
        return it->second;
    }

    std::cerr << "[ShaderLibrary] Shader '" << handle
              << "' not found; returning nullptr.\n";
    return nullptr;
}

bool ShaderLibrary::contains(const ShaderHandle& handle) const
{
    return m_shaders.find(handle) != m_shaders.end();
}

void ShaderLibrary::remove(const ShaderHandle& handle)
{
    const auto it = m_shaders.find(handle);
    if (it != m_shaders.end())
    {
        m_shaders.erase(it);
    }
    else
    {
        std::cerr << "[ShaderLibrary] Attempt to remove non-existent shader '"
                  << handle << "'.\n";
    }
}

void ShaderLibrary::clear()
{
    m_shaders.clear();
}

}