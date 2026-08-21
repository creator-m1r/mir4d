// MirEngine/Rendering/OpenGL/OpenGLVFXSink.hpp
// =================================================================================
// OpenGL implementation of the MIR 4D VFX draw sink.
//
// Draws each live particle as a screen-space point sprite in NDC space
// (x,y in [-1,1], z toward the viewer), overlaid on top of the CAD scene.
// Owning the renderer registers this sink with the VFX EffectSystem so the
// subsystem (which is renderer-agnostic) can submit particles to the GPU
// without depending on OpenGL directly.
// =================================================================================

#pragma once

#include "MirEngine/VFX/EffectSystem.hpp"

#include <memory>
#include <vector>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine::Rendering
{

class OpenGLVFXSink final : public MirEngine::VFX::VFXDrawSink
{
public:
    OpenGLVFXSink() = default;
    ~OpenGLVFXSink() override = default;

    OpenGLVFXSink(const OpenGLVFXSink&) = delete;
    OpenGLVFXSink& operator=(const OpenGLVFXSink&) = delete;

    /// Accumulates a particle for the next flush() (called by
    /// EffectSystem::render() once per live particle).
    void drawParticle(const MirEngine::VFX::Particle& particle) override;

    /// Uploads and draws all accumulated particles. Must be called with the
    /// GL context current. Safe to call when no particles are pending.
    void flush() noexcept;

private:
    bool ensureProgram() noexcept;

    bool m_ready{false};
    GLuint m_program{0};
    GLuint m_vao{0};
    GLuint m_vbo{0};
    GLint m_uScale{-1};
    std::vector<float> m_batch;   // 7 floats per particle: x,y,z, r,g,b, size
};

} // namespace MirEngine::Rendering
