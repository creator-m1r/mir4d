// MirEngine/Rendering/OpenGL/OpenGLShader.h
// =================================================================================
// OpenGL implementation of the Shader interface.
// Uses the system OpenGL headers only (no external dependencies).
// =================================================================================

#pragma once

#include "../Resources/Shader.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine::Rendering {

class OpenGLShader final : public Shader {
public:
    OpenGLShader();
    ~OpenGLShader() override;

    OpenGLShader(const OpenGLShader&) = delete;
    OpenGLShader& operator=(const OpenGLShader&) = delete;

    // Native program handle (for debugging).
    [[nodiscard]] GLuint handle() const noexcept { return m_program; }

    bool compile(const std::string& vertexSource,
                 const std::string& fragmentSource) override;

    void bind() override;
    void unbind() override;

    void setInt(std::string_view name, int value) override;
    void setFloat(std::string_view name, float value) override;
    void setVec2(std::string_view name, float x, float y) override;
    void setVec3(std::string_view name, float x, float y, float z) override;
    void setVec4(std::string_view name, float x, float y, float z, float w) override;
    void setMatrix(std::string_view name, const Matrix4Raw& matrix) override;
    void setMatrix3(std::string_view name, const float* values) override;

private:
    GLuint m_program{0};
    std::unordered_map<std::string, GLint> m_uniformCache;

    [[nodiscard]] GLint getUniformLocation(std::string_view name);
    [[nodiscard]] GLuint compileShader(GLenum type, const std::string& source);
    [[nodiscard]] bool checkCompileStatus(GLuint shader,
                                          const char* typeName,
                                          const std::string& source);
    [[nodiscard]] bool checkLinkStatus();
};

} // namespace MirEngine::Rendering