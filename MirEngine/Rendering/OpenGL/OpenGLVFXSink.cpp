// MirEngine/Rendering/OpenGL/OpenGLVFXSink.cpp
// =================================================================================
// OpenGL implementation of the MIR 4D VFX draw sink.
// =================================================================================

#include "OpenGLVFXSink.hpp"

#include <cstdio>

namespace MirEngine::Rendering
{

namespace
{

const char* kVertexSource = R"GLSL(#version 150 core
in vec3 a_pos;
in vec3 a_color;
in float a_size;
uniform float u_scale;
out vec3 v_color;
void main()
{
    gl_Position = vec4(a_pos, 1.0);
    gl_PointSize = max(1.0, a_size * u_scale);
    v_color = a_color;
}
)GLSL";

const char* kFragmentSource = R"GLSL(#version 150 core
in vec3 v_color;
out vec4 frag;
void main()
{
    vec2 d = gl_PointCoord - vec2(0.5);
    float r = length(d);
    if (r > 0.5) discard;
    float alpha = smoothstep(0.5, 0.15, r);
    frag = vec4(v_color, alpha);
}
)GLSL";

GLuint compileShader(GLenum type, const char* source)
{
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        GLchar log[512]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[OpenGLVFXSink] shader compile failed: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

} // namespace

bool OpenGLVFXSink::ensureProgram() noexcept
{
    if (m_ready)
        return true;

    const GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexSource);
    const GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentSource);
    if (vs == 0 || fs == 0)
        return false;

    const GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (status == 0)
    {
        GLchar log[512]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[OpenGLVFXSink] program link failed: %s\n", log);
        glDeleteProgram(program);
        return false;
    }

    m_program = program;
    m_uScale = glGetUniformLocation(program, "u_scale");

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    const GLsizei stride = static_cast<GLsizei>(7 * sizeof(float));

    const GLint aPos = glGetAttribLocation(program, "a_pos");
    const GLint aCol = glGetAttribLocation(program, "a_color");
    const GLint aSize = glGetAttribLocation(program, "a_size");

    if (aPos >= 0)
    {
        glEnableVertexAttribArray(aPos);
        glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, stride,
                              static_cast<void*>(nullptr));
    }
    if (aCol >= 0)
    {
        glEnableVertexAttribArray(aCol);
        glVertexAttribPointer(aCol, 3, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(3 * sizeof(float)));
    }
    if (aSize >= 0)
    {
        glEnableVertexAttribArray(aSize);
        glVertexAttribPointer(aSize, 1, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(6 * sizeof(float)));
    }

    glBindVertexArray(0);

    m_ready = true;
    std::fprintf(stderr,
                 "[OpenGLVFXSink] program ready (u_scale location=%d)\n",
                 static_cast<int>(m_uScale));
    return true;
}

void OpenGLVFXSink::drawParticle(const MirEngine::VFX::Particle& particle)
{
    m_batch.push_back(particle.x);
    m_batch.push_back(particle.y);
    m_batch.push_back(particle.z);
    m_batch.push_back(particle.r);
    m_batch.push_back(particle.g);
    m_batch.push_back(particle.b);
    m_batch.push_back(particle.size);
}

void OpenGLVFXSink::flush() noexcept
{
    if (m_batch.empty())
        return;

    if (!ensureProgram())
    {
        // ensureProgram already logs the specific shader/link failure.
        static bool s_reported = false;
        if (!s_reported)
        {
            std::fprintf(stderr,
                         "[OpenGLVFXSink] flush skipped: program not ready\n");
            s_reported = true;
        }
        m_batch.clear();
        return;
    }

    const GLsizei vertexCount = static_cast<GLsizei>(m_batch.size() / 7);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_batch.size() * sizeof(float)),
                 m_batch.data(),
                 GL_DYNAMIC_DRAW);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_program);
    if (m_uScale >= 0)
        glUniform1f(m_uScale, 26.0F);

    glDrawArrays(GL_POINTS, 0, vertexCount);

    glUseProgram(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);

    // Diagnostic: report drawn particle count, throttled to ~1 line/sec so
    // an active effect does not flood the console.
    static int s_frame = 0;
    if ((++s_frame % 60) == 0)
    {
        std::fprintf(stderr,
                     "[OpenGLVFXSink] drew %d VFX particles\n",
                     static_cast<int>(vertexCount));
    }

    m_batch.clear();
}

} // namespace MirEngine::Rendering
