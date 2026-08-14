// MirEngine/Rendering/OpenGL/OpenGLShader.h
// =================================================================================
// Конкретная реализация Shader для OpenGL.
// Использует стандартные заголовки OpenGL (без glbinding/spdlog).
// =================================================================================

#pragma once

#include "../Resources/Shader.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstdint>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>   // или glew / epoxy
#endif

namespace MirEngine {
namespace Rendering {

class OpenGLShader final : public Shader {
public:
    OpenGLShader();
    ~OpenGLShader() override;

    OpenGLShader(const OpenGLShader&) = delete;
    OpenGLShader& operator=(const OpenGLShader&) = delete;

    // Нативный handle программы (для отладки)
    [[nodiscard]] GLuint handle() const noexcept { return m_program; }

    bool compile(const std::string& vertexSource,
                 const std::string& fragmentSource) override;

    void bind() override;
    void unbind() override;

    void setInt   (std::string_view name, int value) override;
    void setFloat (std::string_view name, float value) override;
    void setVec2  (std::string_view name, float x, float y) override;
    void setVec3  (std::string_view name, float x, float y, float z) override;
    void setVec4  (std::string_view name, float x, float y, float z, float w) override;
    void setMatrix(std::string_view name, const Matrix4Raw& matrix) override;

private:
    GLuint m_program = 0;
    std::unordered_map<std::string, GLint> m_uniformCache;

    GLint  getUniformLocation(std::string_view name);
    GLuint compileShader(GLenum type, const std::string& source);
    bool   checkCompileStatus(GLuint shader, const char* typeName);
    bool   checkLinkStatus();
};

} // namespace Rendering
} // namespace MirEngine