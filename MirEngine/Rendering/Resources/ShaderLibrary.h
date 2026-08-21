
#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace MirEngine {
namespace Rendering {

class Shader;

using ShaderHandle = std::string;

using ShaderFactory = std::function<std::shared_ptr<Shader>()>;

class ShaderLibrary {
public:

    explicit ShaderLibrary(ShaderFactory factory);

    ShaderHandle load(const std::string& name,
                      const std::string& vertexSource,
                      const std::string& fragmentSource,
                      bool forceReload = false);

    std::shared_ptr<Shader> get(const ShaderHandle& handle) const;

    bool contains(const ShaderHandle& handle) const;

    void remove(const ShaderHandle& handle);

    void clear();

private:
    ShaderFactory m_factory;
    std::unordered_map<ShaderHandle, std::shared_ptr<Shader>> m_shaders;
};

}
}