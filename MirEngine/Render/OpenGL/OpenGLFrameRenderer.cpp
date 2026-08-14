#include "OpenGLFrameRenderer.hpp"

#if defined(__APPLE__)
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

namespace mir
{

bool OpenGLFrameRenderer::initialize() noexcept
{
    static constexpr const char* vertexShader = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
uniform mat4 uViewProjection;
out vec3 vNormal;
void main()
{
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
    vNormal = aNormal;
}
)GLSL";

    static constexpr const char* fragmentShader = R"GLSL(
#version 330 core
in vec3 vNormal;
out vec4 fragColor;
void main()
{
    vec3 n = normalize(vNormal);
    vec3 lightDirection = normalize(vec3(0.35, 0.8, 0.55));
    float diffuse = max(dot(n, lightDirection), 0.0);
    float intensity = 0.18 + 0.82 * diffuse;
    fragColor = vec4(vec3(intensity), 1.0);
}
)GLSL";

    if (!shader_.build(vertexShader, fragmentShader))
        return false;

    viewProjectionLocation_ = shader_.uniformLocation("uViewProjection");
    if (viewProjectionLocation_ < 0)
    {
        shader_.destroy();
        return false;
    }

    initialized_ = true;
    return true;
}

void OpenGLFrameRenderer::destroy() noexcept
{
    shader_.destroy();
    initialized_ = false;
    viewProjectionLocation_ = -1;
}

void OpenGLFrameRenderer::resize(int width, int height) noexcept
{
    width_ = width > 0 ? width : 1;
    height_ = height > 0 ? height : 1;
    glViewport(0, 0, width_, height_);
}

void OpenGLFrameRenderer::beginFrame() const noexcept
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLFrameRenderer::draw(const RenderMeshGPU& mesh,
                               const RenderCamera& camera) noexcept
{
    if (!initialized_ || !mesh.valid())
        return;

    const auto view = camera.viewMatrix();
    const auto projection = camera.projectionMatrix();

    // RenderCamera stores matrices in column-major OpenGL layout.
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

    shader_.bind();
    shader_.setMat4(viewProjectionLocation_, viewProjection);
    meshRenderer_.draw(mesh);
}

void OpenGLFrameRenderer::endFrame() const noexcept
{
    glUseProgram(0);
}

} // namespace mir
