#include "OpenGLShader.hpp"

#if defined(__APPLE__)
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

#include <vector>

namespace mir
{
namespace
{

[[nodiscard]] static std::uint32_t compileShader(GLenum type,
                                                  const std::string& source) noexcept
{
    const GLuint shader = glCreateShader(type);
    if (shader == 0)
        return 0;

    const char* sourceText = source.c_str();
    glShaderSource(shader, 1, &sourceText, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

} // namespace

OpenGLShader::~OpenGLShader() noexcept
{
    destroy();
}

bool OpenGLShader::build(const std::string& vertexSource,
                         const std::string& fragmentSource) noexcept
{
    destroy();

    const GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertex == 0)
        return false;

    const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragment == 0)
    {
        glDeleteShader(vertex);
        return false;
    }

    const GLuint program = glCreateProgram();
    if (program == 0)
    {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (status != GL_TRUE)
    {
        glDeleteProgram(program);
        return false;
    }

    program_ = program;
    return true;
}

void OpenGLShader::destroy() noexcept
{
    if (program_ != 0)
    {
        glDeleteProgram(static_cast<GLuint>(program_));
        program_ = 0;
    }
}

void OpenGLShader::bind() const noexcept
{
    if (program_ != 0)
        glUseProgram(static_cast<GLuint>(program_));
}

int OpenGLShader::uniformLocation(const char* name) const noexcept
{
    if (program_ == 0 || name == nullptr)
        return -1;

    return glGetUniformLocation(static_cast<GLuint>(program_), name);
}

void OpenGLShader::setMat4(int location, const double* matrix) const noexcept
{
    if (program_ == 0 || location < 0 || matrix == nullptr)
        return;

    // OpenGL's classic glUniformMatrix4fv accepts float data. The conversion
    // stays inside the rendering boundary; the camera keeps double precision.
    float values[16]{};
    for (int i = 0; i < 16; ++i)
        values[i] = static_cast<float>(matrix[i]);

    glUniformMatrix4fv(location, 1, GL_FALSE, values);
}

} // namespace mir
