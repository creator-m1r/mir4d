#include "OpenGLRenderMeshUploader.hpp"

#if defined(__APPLE__)
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

namespace mir
{

RenderMeshGPU OpenGLRenderMeshUploader::upload(const RenderMesh& mesh)
{
    RenderMeshGPU result{};
    if (mesh.empty())
        return result;

    GLuint vertexArray = 0;
    GLuint vertexBuffer = 0;
    GLuint indexBuffer = 0;

    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &indexBuffer);

    if (vertexArray == 0 || vertexBuffer == 0 || indexBuffer == 0)
    {
        if (vertexArray != 0)
            glDeleteVertexArrays(1, &vertexArray);
        if (vertexBuffer != 0)
            glDeleteBuffers(1, &vertexBuffer);
        if (indexBuffer != 0)
            glDeleteBuffers(1, &indexBuffer);
        return result;
    }

    glBindVertexArray(vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(RenderVertex)),
        mesh.vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(std::uint32_t)),
        mesh.indices.data(),
        GL_STATIC_DRAW);

    constexpr GLsizei stride = static_cast<GLsizei>(sizeof(RenderVertex));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        reinterpret_cast<const void*>(offsetof(RenderVertex, x)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        reinterpret_cast<const void*>(offsetof(RenderVertex, nx)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    result.vertexArray.value = static_cast<std::uint64_t>(vertexArray);
    result.vertexBuffer.value = static_cast<std::uint64_t>(vertexBuffer);
    result.indexBuffer.value = static_cast<std::uint64_t>(indexBuffer);
    result.indexCount = static_cast<std::uint32_t>(mesh.indices.size());
    return result;
}

void OpenGLRenderMeshUploader::release(RenderMeshGPU& mesh) noexcept
{
    const auto vertexArray = static_cast<GLuint>(mesh.vertexArray.value);
    const auto vertexBuffer = static_cast<GLuint>(mesh.vertexBuffer.value);
    const auto indexBuffer = static_cast<GLuint>(mesh.indexBuffer.value);

    if (vertexArray != 0)
        glDeleteVertexArrays(1, &vertexArray);
    if (vertexBuffer != 0)
        glDeleteBuffers(1, &vertexBuffer);
    if (indexBuffer != 0)
        glDeleteBuffers(1, &indexBuffer);

    mesh = {};
}

} // namespace mir
