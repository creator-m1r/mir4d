
#include "OpenGLShader.h"

#include <iostream>
#include <string>
#include <vector>

namespace MirEngine::Rendering {

OpenGLShader::OpenGLShader()
{
    m_program = glCreateProgram();
    if (m_program == 0)
    {
        std::cerr << "[OpenGLShader] Failed to create program\n";
    }
}

OpenGLShader::~OpenGLShader()
{
    if (m_program != 0)
    {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    m_uniformCache.clear();
}

bool OpenGLShader::compile(const std::string& vertexSource,
                           const std::string& fragmentSource)
{
    if (m_program != 0)
    {
        glDeleteProgram(m_program);
        m_program = glCreateProgram();
        m_uniformCache.clear();
    }

    const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0)
    {
        return false;
    }

    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        return false;
    }

    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (!checkLinkStatus())
    {
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }
    return true;
}

void OpenGLShader::bind()
{
    glUseProgram(m_program);
}

void OpenGLShader::unbind()
{
    glUseProgram(0);
}

void OpenGLShader::setInt(std::string_view name, int value)
{
    const GLint location = getUniformLocation(name);
    if (location != -1)
        glUniform1i(location, value);
}

void OpenGLShader::setFloat(std::string_view name, float value)
{
    const GLint location = getUniformLocation(name);
    if (location != -1)
        glUniform1f(location, value);
}

void OpenGLShader::setVec2(std::string_view name, float x, float y)
{
    const GLint location = getUniformLocation(name);
    if (location != -1)
        glUniform2f(location, x, y);
}

void OpenGLShader::setVec3(std::string_view name, float x, float y, float z)
{
    const GLint location = getUniformLocation(name);
    if (location != -1)
        glUniform3f(location, x, y, z);
}

void OpenGLShader::setVec4(std::string_view name, float x, float y, float z, float w)
{
    const GLint location = getUniformLocation(name);
    if (location != -1)
        glUniform4f(location, x, y, z, w);
}

void OpenGLShader::setMatrix(std::string_view name, const Matrix4Raw& matrix)
{
    const GLint location = getUniformLocation(name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());
    }
}

void OpenGLShader::setMatrix3(std::string_view name, const float* values)
{
    const GLint location = getUniformLocation(name);
    if (location != -1)
    {
        glUniformMatrix3fv(location, 1, GL_FALSE, values);
    }
}

GLint OpenGLShader::getUniformLocation(std::string_view name)
{
    const std::string key(name);
    const auto it = m_uniformCache.find(key);
    if (it != m_uniformCache.end())
    {
        return it->second;
    }

    const GLint location = glGetUniformLocation(m_program, key.c_str());
    m_uniformCache.emplace(key, location);
    return location;
}

GLuint OpenGLShader::compileShader(GLenum type, const std::string& source)
{
    const GLuint shader = glCreateShader(type);
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    if (!checkCompileStatus(shader,
                            type == GL_VERTEX_SHADER ? "Vertex" : "Fragment",
                            source))
    {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool OpenGLShader::checkCompileStatus(GLuint shader,
                                      const char* typeName,
                                      const std::string& source)
{
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE)
    {
        return true;
    }

    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        std::vector<char> log(static_cast<std::size_t>(logLength));
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::cerr << "[OpenGLShader] " << typeName << " shader compile error:\n"
                  << log.data() << "\n";
        std::cerr << "[OpenGLShader] Source (" << source.size() << " bytes):\n"
                  << source << "\n";
    }
    return false;
}

bool OpenGLShader::checkLinkStatus()
{
    GLint status = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status == GL_TRUE)
    {
        return true;
    }

    GLint logLength = 0;
    glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        std::vector<char> log(static_cast<std::size_t>(logLength));
        glGetProgramInfoLog(m_program, logLength, nullptr, log.data());
        std::cerr << "[OpenGLShader] Program link error:\n"
                  << log.data() << "\n";
    }
    return false;
}

}