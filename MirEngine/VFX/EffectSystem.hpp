#pragma once

#include "Effect.hpp"

#include <cstddef>
#include <vector>

namespace MirEngine::VFX
{

/// Renderer-agnostic draw sink. The active renderer registers an
/// implementation so the VFX subsystem can submit particles without
/// depending on OpenGL or any concrete rendering backend.
class VFXDrawSink
{
public:
    virtual ~VFXDrawSink() = default;
    virtual void drawParticle(const Particle& particle) = 0;
};

/// Owns and advances all live visual-effect particles.
///
/// The subsystem is a first-class MIR 4D module: it replaces any
/// external/third-party effect noise with deterministic, owned behaviour.
/// Simulation runs on the CPU; rendering is delegated to a registered sink.
class EffectSystem
{
public:
    static EffectSystem& instance() noexcept;

    void setSink(VFXDrawSink* sink) noexcept { m_sink = sink; }

    /// Triggers an effect burst. Returns the number of particles spawned.
    int trigger(EffectKind kind, const EffectParams& params = EffectParams{});

    /// Advances the simulation by dt seconds (does nothing if dt <= 0).
    void update(float dt) noexcept;

    /// Submits all live particles to the registered sink (no-op if none).
    void render() noexcept;

    /// Clears all live particles.
    void reset() noexcept;

    [[nodiscard]] std::size_t liveCount() const noexcept { return m_particles.size(); }

    /// Records a single frame of statistics for diagnostics.
    void stats(int* outLive, int* outKind) const noexcept;

private:
    EffectSystem() = default;

    std::vector<Particle> m_particles;
    VFXDrawSink* m_sink{nullptr};
    int m_lastKind{static_cast<int>(EffectKind::Confetti)};

    float m_rngState{0.123F};

    float rand() noexcept;                       // deterministic LCG in [0,1)
    float randRange(float lo, float hi) noexcept;
    void spawn(const Particle& particle);
};

} // namespace MirEngine::VFX
