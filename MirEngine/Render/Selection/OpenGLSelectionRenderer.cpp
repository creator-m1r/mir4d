#include "OpenGLSelectionRenderer.hpp"

#if defined(__APPLE__)
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

#include <vector>

namespace mir
{

bool OpenGLSelectionRenderer::initialize() noexcept
{
    static constexpr const char* vertexSource = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uViewProjection;
void main()
{
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}
)GLSL";

    static constexpr const char* fragmentSource = R"GLSL(
#version 330 core
out vec4 fragColor;
void main()
{
    fragColor = vec4(1.0, 0.65, 0.05, 0.35);
}
)GLSL";

    if (!shader_.build(vertexSource, fragmentSource))
        return false;

    viewProjectionLocation_ = shader_.uniformLocation("uViewProjection");
    if (viewProjectionLocation_ < 0)
    {
        shader_.destroy();
        return false;
    }

    return true;
}

void OpenGLSelectionRenderer::destroy() noexcept
{
    shader_.destroy();
    viewProjectionLocation_ = -1;
}

void OpenGLSelectionRenderer::draw(
    const SelectionOverlay& overlay,
    const RenderMesh& mesh,
    const RenderCamera& camera) const noexcept
{
    if (!overlay.visible() || !shader_.valid() || mesh.vertices.empty())
        return;

    const auto view = camera.viewMatrix();
    const auto projection = camera.projectionMatrix();

    double viewProjection[16]{};
    for (int column = 0; column < 4; ++column)
    {
        for (int row = 0; row < 4; ++row)
        {
            double value = 0.0;
            for (int k = 0; k < 4; ++k)
                value += projection.m[k * 4 + row] * view.m[column * 4 + k];
            viewProjection[column * 4 + row] = value;
        }
    }

    float matrix[16]{};
    for (int i = 0; i < 16; ++i)
        matrix[i] = static_cast<float>(viewProjection[i]);

    std::vector<float> positions;
    positions.reserve(overlay.triangleIndices.size() * 9);

    for (const auto triangleIndex : overlay.triangleIndices)
    {
        if (triangleIndex >= mesh.triangles.size())
            continue;

        const auto& triangle = mesh.triangles[triangleIndex];
        const std::uint32_t ids[3]{triangle.a, triangle.b, triangle.c};

        for (const auto id : ids)
        {
            if (id >= mesh.vertices.size())
                continue;

            const auto& vertex = mesh.vertices[id];
            positions.push_back(vertex.x);
            positions.push_back(vertex.y);
            positions.push_back(vertex.z);
        }
    }

    if (positions.empty())
        return;

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    if (vao == 0 || vbo == 0)
    {
        if (vbo != 0) glDeleteBuffers(1, &vbo);
        if (vao != 0) glDeleteVertexArrays(1, &vao);
        return;
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(positions.size() * sizeof(float)),
        positions.data(),
        GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    shader_.bind();
    glUniformMatrix4fv(viewProjectionLocation_, 1, GL_FALSE, matrix);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0F, -1.0F);

    glDrawArrays(
        GL_TRIANGLES,
        0,
        static_cast<GLsizei>(positions.size() / 3));

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glUseProgram(0);
}

} // namespace mir
