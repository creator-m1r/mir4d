
#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <array>

#include "../Core/RenderCommand.h"

namespace MirEngine {
namespace Rendering {

class Shader {
public:
    virtual ~Shader() = default;

    virtual bool compile(const std::string& vertexSource,
                         const std::string& fragmentSource) = 0;

    virtual void bind() = 0;

    virtual void unbind() = 0;

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

}
}