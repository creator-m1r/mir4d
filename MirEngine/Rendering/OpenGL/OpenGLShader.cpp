// MirEngine/Rendering/OpenGL/OpenGLShader.cpp
// =================================================================================
// Реализация OpenGLShader без внешних зависимостей (spdlog / glbinding).
// =================================================================================

#include "OpenGLShader.h"

#include <iostream>
#include <vector>
#include <string>

namespace MirEngine {
namespace Rendering {

// --------------------------------------------------------------------------
// Конструктор
// --------------------------------------------------------------------------
OpenGLShader::OpenGLShader()
{
    m_program = glCreateProgram();
    if (m_program == 0) {
        std::cerr << "[OpenGLShader] Failed to create program\n";
    }
}

// --------------------------------------------------------------------------
// Деструктор
// --------------------------------------------------------------------------
OpenGLShader::~OpenGLShader()
{
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    m_uniformCache.clear();
}

// --------------------------------------------------------------------------
// Компиляция
// --------------------------------------------------------------------------
bool OpenGLShader::compile(const std::string& vertexSource,
                           const std::string& fragmentSource)
{
    // Если программа уже существует — пересоздаём
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = glCreateProgram();
        m_uniformCache.clear();
    }

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vs == 0) return false;

    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fs == 0) {
        glDeleteShader(vs);
        return false;
    }

    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    // Промежуточные шейдеры больше не нужны
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!checkLinkStatus()) {
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    return true;
}

// --------------------------------------------------------------------------
// bind / unbind
// --------------------------------------------------------------------------
void OpenGLShader::bind()
{
    glUseProgram(m_program);
}

void OpenGLShader::unbind()
{
    glUseProgram(0);
}

// --------------------------------------------------------------------------
// Uniforms
// --------------------------------------------------------------------------
void OpenGLShader::setInt(std::string_view name, int value)
{
    GLint loc = getUniformLocation(name);
    if (loc != -1) glUniform1i(loc, value);
}

void OpenGLShader::setFloat(std::string_view name, float value)
{
    GLint loc = getUniformLocation(name);
    if (loc != -1) glUniform1f(loc, value);
}

void OpenGLShader::setVec2(std::string_view name, float x, float y)
{
    GLint loc = getUniformLocation(name);
    if (loc != -1) glUniform2f(loc, x, y);
}

void OpenGLShader::setVec3(std::string_view name, float x, float y, float z)
{
    GLint loc = getUniformLocation(name);
    if (loc != -1) glUniform3f(loc, x, y, z);
}

void OpenGLShader::setVec4(std::string_view name, float x, float y, float z, float w)
{
    GLint loc = getUniformLocation(name);
    if (loc != -1) glUniform4f(loc, x, y, z, w);
}

void OpenGLShader::setMatrix(std::string_view name, const Matrix4Raw& matrix)
{
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, matrix.data());
    }
}

// --------------------------------------------------------------------------
// Внутренние методы
// --------------------------------------------------------------------------
GLint OpenGLShader::getUniformLocation(std::string_view name)
{
    std::string key(name);
    auto it = m_uniformCache.find(key);
    if (it != m_uniformCache.end()) {
        return it->second;
    }

    GLint loc = glGetUniformLocation(m_program, key.c_str());
    m_uniformCache[key] = loc;
    return loc;
}

GLuint OpenGLShader::compileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    if (!checkCompileStatus(shader, type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")) {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool OpenGLShader::checkCompileStatus(GLuint shader, const char* typeName)
{
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE) return true;

    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        std::vector<char> log(static_cast<size_t>(logLength));
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::cerr << "[OpenGLShader] " << typeName << " shader compile error:\n"
                  << log.data() << "\n";
    }
    return false;
}

bool OpenGLShader::checkLinkStatus()
{
    GLint status = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status == GL_TRUE) return true;

    GLint logLength = 0;
    glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        std::vector<char> log(static_cast<size_t>(logLength));
        glGetProgramInfoLog(m_program, logLength, nullptr, log.data());
        std::cerr << "[OpenGLShader] Program link error:\n" << log.data() << "\n";
    }
    return false;
}

} // namespace Rendering
} // namespace MirEngine