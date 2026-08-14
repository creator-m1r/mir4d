#include "OpenGLMeshRenderer.hpp"

#if defined(__APPLE__)
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

namespace mir
{

void OpenGLMeshRenderer::draw(
    const RenderMeshGPU& mesh,
    const OpenGLDrawParameters& parameters) const noexcept
{
    if (!mesh.valid())
        return;

    const auto vertexArray = static_cast<GLuint>(mesh.vertexArray.value);
    if (vertexArray == 0 || mesh.indexCount == 0)
        return;

    glBindVertexArray(vertexArray);

    if (parameters.wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(mesh.indexCount),
        GL_UNSIGNED_INT,
        nullptr);

    if (parameters.wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
}

} // namespace mir
