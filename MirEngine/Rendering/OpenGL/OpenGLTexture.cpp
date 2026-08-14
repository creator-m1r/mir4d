// MirEngine/Rendering/OpenGL/OpenGLTexture.cpp
// =================================================================================
// Реализация 2D-текстуры поверх OpenGL.
// =================================================================================

#include "OpenGLTexture.h"

#include <iostream>

namespace MirEngine {
namespace Rendering {

namespace {

[[nodiscard]] GLenum toInternalFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::RGBA8:  return GL_RGBA8;
    case TextureFormat::RGB8:   return GL_RGB8;
    case TextureFormat::Depth24: return GL_DEPTH_COMPONENT24;
    }
    return GL_RGBA8;
}

[[nodiscard]] GLenum toPixelFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::RGBA8:  return GL_RGBA;
    case TextureFormat::RGB8:   return GL_RGB;
    case TextureFormat::Depth24: return GL_DEPTH_COMPONENT;
    }
    return GL_RGBA;
}

} // namespace

OpenGLTexture::OpenGLTexture() = default;

OpenGLTexture::~OpenGLTexture()
{
    if (m_id != 0)
        glDeleteTextures(1, &m_id);
}

bool OpenGLTexture::create(std::uint32_t width,
                           std::uint32_t height,
                           TextureFormat format,
                           TextureFilter filter)
{
    if (width == 0 || height == 0)
        return false;

    if (m_id == 0)
        glGenTextures(1, &m_id);
    if (m_id == 0)
        return false;

    m_width = width;
    m_height = height;
    m_format = format;
    m_filter = filter;

    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    filter == TextureFilter::Nearest ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    filter == TextureFilter::Nearest ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(toInternalFormat(format)),
                 static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
                 toPixelFormat(format),
                 format == TextureFormat::Depth24 ? GL_FLOAT : GL_UNSIGNED_BYTE,
                 nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool OpenGLTexture::upload(const std::uint8_t* pixels)
{
    if (m_id == 0 || pixels == nullptr || m_format == TextureFormat::Depth24)
        return false;

    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height),
                    toPixelFormat(m_format),
                    m_format == TextureFormat::RGBA8 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_BYTE,
                    pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void OpenGLTexture::bind(std::uint32_t slot)
{
    if (m_id == 0)
        return;
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + slot));
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void OpenGLTexture::unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLTexture::resize(std::uint32_t width, std::uint32_t height)
{
    if (width == 0 || height == 0 || m_id == 0)
        return;

    m_width = width;
    m_height = height;

    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(toInternalFormat(m_format)),
                 static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
                 toPixelFormat(m_format),
                 m_format == TextureFormat::Depth24 ? GL_FLOAT : GL_UNSIGNED_BYTE,
                 nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Rendering
} // namespace MirEngine