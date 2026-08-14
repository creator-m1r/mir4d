// MirEngine/Rendering/OpenGL/OpenGLFrameBuffer.cpp
// =================================================================================
// Реализация FBO с цветовым texture attachment и depth renderbuffer.
// =================================================================================

#include "OpenGLFrameBuffer.h"

#include <iostream>

namespace MirEngine {
namespace Rendering {

OpenGLFrameBuffer::OpenGLFrameBuffer() = default;

OpenGLFrameBuffer::~OpenGLFrameBuffer()
{
    releaseResources();
}

void OpenGLFrameBuffer::releaseResources()
{
    if (m_fbo != 0)
    {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    if (m_depthRbo != 0)
    {
        glDeleteRenderbuffers(1, &m_depthRbo);
        m_depthRbo = 0;
    }
}

bool OpenGLFrameBuffer::create(std::uint32_t width, std::uint32_t height)
{
    if (width == 0 || height == 0)
        return false;

    releaseResources();

    m_width = width;
    m_height = height;

    m_colorTexture = std::make_shared<OpenGLTexture>();
    if (!m_colorTexture->create(width, height, TextureFormat::RGBA8, TextureFilter::Linear))
    {
        std::cerr << "[OpenGLFrameBuffer] Failed to create color texture.\n";
        return false;
    }

    glGenFramebuffers(1, &m_fbo);
    if (m_fbo == 0)
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    const auto* glTexture = dynamic_cast<OpenGLTexture*>(m_colorTexture.get());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           glTexture != nullptr ? glTexture->handle() : 0, 0);
    attachDepthRenderbuffer();

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "[OpenGLFrameBuffer] Framebuffer incomplete (status " << status << ").\n";
        releaseResources();
        return false;
    }

    return true;
}

void OpenGLFrameBuffer::attachDepthRenderbuffer()
{
    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void OpenGLFrameBuffer::attachColorTexture(const std::shared_ptr<Texture>& colorTexture)
{
    auto* glTexture = dynamic_cast<OpenGLTexture*>(colorTexture.get());
    if (glTexture == nullptr || m_fbo == 0)
        return;

    m_width = glTexture->width();
    m_height = glTexture->height();

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           glTexture->handle(), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFrameBuffer::bind()
{
    if (m_fbo == 0)
        return;
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
}

void OpenGLFrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFrameBuffer::resize(std::uint32_t width, std::uint32_t height)
{
    if (width == 0 || height == 0)
        return;

    m_width = width;
    m_height = height;

    if (m_colorTexture)
        m_colorTexture->resize(width, height);

    if (m_fbo == 0)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace Rendering
} // namespace MirEngine