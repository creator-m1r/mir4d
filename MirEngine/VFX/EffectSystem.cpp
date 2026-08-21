#include "EffectSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace MirEngine::VFX
{

EffectSystem& EffectSystem::instance() noexcept
{
    static EffectSystem s_instance;
    return s_instance;
}

float EffectSystem::rand() noexcept
{
    // Deterministic LCG (numerical recipes), good enough for effects.
    m_rngState = std::fmod(m_rngState * 1664525.0F + 1013904223.0F, 4294967296.0F);
    return m_rngState / 4294967296.0F;
}

float EffectSystem::randRange(float lo, float hi) noexcept
{
    return lo + (hi - lo) * rand();
}

void EffectSystem::spawn(const Particle& particle)
{
    if (m_particles.size() < 200000)
        m_particles.push_back(particle);
}

void EffectSystem::update(float dt) noexcept
{
    if (dt <= 0.0F)
        return;

    float clamped = std::min(dt, 0.1F);

    for (Particle& p : m_particles)
    {
        p.vy += p.ay * clamped;
        p.x += p.vx * clamped;
        p.y += p.vy * clamped;
        p.z += p.vz * clamped;
        p.life -= clamped;
    }

    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
                       [](const Particle& p) { return p.life <= 0.0F; }),
        m_particles.end());
}

void EffectSystem::render() noexcept
{
    if (!m_sink)
        return;

    for (const Particle& p : m_particles)
        m_sink->drawParticle(p);
}

void EffectSystem::reset() noexcept
{
    m_particles.clear();
}

void EffectSystem::stats(int* outLive, int* outKind) const noexcept
{
    if (outLive)
        *outLive = static_cast<int>(m_particles.size());
    if (outKind)
        *outKind = m_lastKind;
}

int EffectSystem::trigger(EffectKind kind, const EffectParams& params)
{
    m_lastKind = static_cast<int>(kind);
    const int n = std::max(1, params.count);
    int spawned = 0;

    auto emit = [&](const Particle& p)
    {
        spawn(p);
        ++spawned;
    };

    switch (kind)
    {
        case EffectKind::Confetti:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = randRange(-params.spread, params.spread);
                p.y = randRange(0.6F, 1.0F);
                p.z = randRange(-0.2F, 0.2F);
                p.vx = randRange(-0.4F, 0.4F) * params.speed;
                p.vy = randRange(-0.2F, 0.1F) * params.speed;
                p.ay = -0.6F;
                p.life = p.maxLife = params.duration * randRange(0.7F, 1.0F);
                p.size = randRange(0.4F, 0.9F);
                p.spin = randRange(-6.0F, 6.0F);
                p.r = randRange(0.6F, 1.0F);
                p.g = randRange(0.2F, 1.0F);
                p.b = randRange(0.2F, 1.0F);
                emit(p);
            }
            break;
        }
        case EffectKind::Balloons:
        {
            const int c = std::max(1, n / 3);
            for (int i = 0; i < c; ++i)
            {
                Particle p{};
                p.x = randRange(-params.spread, params.spread);
                p.y = randRange(-0.9F, -0.6F);
                p.z = randRange(-0.1F, 0.1F);
                p.vx = randRange(-0.15F, 0.15F) * params.speed;
                p.vy = randRange(0.5F, 0.9F) * params.speed;
                p.ay = 0.05F;
                p.life = p.maxLife = params.duration * 1.4F;
                p.size = randRange(0.8F, 1.6F);
                const float hue = randRange(0.0F, 1.0F);
                p.r = 0.6F + 0.4F * hue;
                p.g = 0.3F + 0.5F * (1.0F - hue);
                p.b = 0.9F;
                emit(p);
            }
            break;
        }
        case EffectKind::Fireworks:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = params.originX + randRange(-0.1F, 0.1F);
                p.y = params.originY + randRange(-0.1F, 0.1F);
                const float ang = randRange(0.0F, 6.2831853F);
                const float sp = randRange(0.6F, 1.4F) * params.speed;
                p.vx = std::cos(ang) * sp;
                p.vy = std::sin(ang) * sp;
                p.ay = -0.9F;
                p.life = p.maxLife = params.duration * randRange(0.6F, 1.0F);
                p.size = randRange(0.3F, 0.7F);
                p.r = randRange(0.7F, 1.0F);
                p.g = randRange(0.5F, 1.0F);
                p.b = randRange(0.4F, 1.0F);
                emit(p);
            }
            break;
        }
        case EffectKind::Rain:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = randRange(-1.0F, 1.0F);
                p.y = randRange(0.7F, 1.0F);
                p.z = randRange(-0.2F, 0.2F);
                p.vx = randRange(-0.05F, 0.05F);
                p.vy = randRange(-1.6F, -1.0F) * params.speed;
                p.ay = -0.4F;
                p.life = p.maxLife = params.duration * 0.6F;
                p.size = randRange(0.15F, 0.35F);
                p.r = 0.6F;
                p.g = 0.75F;
                p.b = 1.0F;
                emit(p);
            }
            break;
        }
        case EffectKind::Hearts:
        {
            const int c = std::max(1, n / 2);
            for (int i = 0; i < c; ++i)
            {
                Particle p{};
                p.x = randRange(-params.spread, params.spread);
                p.y = randRange(-0.9F, -0.5F);
                p.z = randRange(-0.1F, 0.1F);
                p.vx = randRange(-0.1F, 0.1F) * params.speed;
                p.vy = randRange(0.4F, 0.8F) * params.speed;
                p.ay = 0.1F;
                p.life = p.maxLife = params.duration * 1.2F;
                p.size = randRange(0.6F, 1.1F);
                p.r = 1.0F;
                p.g = randRange(0.2F, 0.5F);
                p.b = randRange(0.4F, 0.7F);
                emit(p);
            }
            break;
        }
        case EffectKind::Lasers:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.y = randRange(-params.spread, params.spread);
                p.x = randRange(-1.0F, -0.7F);
                p.z = randRange(-0.1F, 0.1F);
                p.vx = randRange(1.5F, 2.5F) * params.speed;
                p.vy = randRange(-0.1F, 0.1F);
                p.ay = 0.0F;
                p.life = p.maxLife = params.duration * 0.5F;
                p.size = randRange(0.1F, 0.25F);
                const bool magenta = randRange(0.0F, 1.0F) > 0.5F;
                p.r = magenta ? 1.0F : 0.2F;
                p.g = 0.9F;
                p.b = magenta ? 0.9F : 1.0F;
                emit(p);
            }
            break;
        }
        case EffectKind::Sparks:
        {
            // Короткие яркие искры: старт у точки сварки, гравитация вниз.
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = params.originX + randRange(-0.05F, 0.05F);
                p.y = params.originY + randRange(-0.02F, 0.05F);
                p.z = randRange(-0.05F, 0.05F);
                const float ang = randRange(0.0F, 6.2831853F);
                const float sp = randRange(0.5F, 1.8F) * params.speed;
                p.vx = std::cos(ang) * sp * 0.6F;
                p.vy = std::fabs(std::sin(ang)) * sp * 0.5F + 0.2F;
                p.ay = -2.5F;
                p.life = p.maxLife = params.duration * randRange(0.15F, 0.4F);
                p.size = randRange(0.08F, 0.22F);
                p.r = 1.0F;
                p.g = randRange(0.6F, 1.0F);
                p.b = randRange(0.2F, 0.6F);
                emit(p);
            }
            break;
        }
        case EffectKind::Snow:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = randRange(-1.0F, 1.0F);
                p.y = randRange(0.6F, 1.0F);
                p.z = randRange(-0.2F, 0.2F);
                p.vx = randRange(-0.05F, 0.05F);
                p.vy = randRange(-0.25F, -0.08F) * params.speed;
                p.ay = -0.02F;
                p.life = p.maxLife = params.duration * randRange(0.8F, 1.2F);
                p.size = randRange(0.1F, 0.28F);
                p.r = p.g = p.b = 1.0F;
                emit(p);
            }
            break;
        }
        case EffectKind::Bubbles:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = randRange(-params.spread, params.spread);
                p.y = randRange(-0.95F, -0.6F);
                p.z = randRange(-0.1F, 0.1F);
                p.vx = randRange(-0.1F, 0.1F) * params.speed;
                p.vy = randRange(0.3F, 0.8F) * params.speed;
                p.ay = 0.05F;
                p.life = p.maxLife = params.duration * randRange(0.8F, 1.3F);
                p.size = randRange(0.2F, 0.5F);
                p.r = 0.6F;
                p.g = 0.85F;
                p.b = 1.0F;
                emit(p);
            }
            break;
        }
        case EffectKind::Sparkles:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = randRange(-1.0F, 1.0F);
                p.y = randRange(-1.0F, 1.0F);
                p.z = randRange(-0.2F, 0.2F);
                p.vx = randRange(-0.05F, 0.05F);
                p.vy = randRange(-0.1F, 0.05F);
                p.ay = 0.0F;
                p.life = p.maxLife = params.duration * randRange(0.4F, 0.9F);
                p.size = randRange(0.08F, 0.25F);
                const float t = randRange(0.0F, 1.0F);
                p.r = 1.0F;
                p.g = 0.9F + 0.1F * t;
                p.b = 0.6F + 0.4F * t;
                emit(p);
            }
            break;
        }
        case EffectKind::Petals:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = randRange(-params.spread, params.spread);
                p.y = randRange(0.4F, 1.0F);
                p.z = randRange(-0.2F, 0.2F);
                p.vx = randRange(-0.2F, 0.2F) * params.speed;
                p.vy = randRange(-0.15F, 0.05F) * params.speed;
                p.ay = -0.15F;
                p.life = p.maxLife = params.duration * randRange(0.7F, 1.1F);
                p.size = randRange(0.3F, 0.7F);
                p.r = 1.0F;
                p.g = randRange(0.4F, 0.7F);
                p.b = randRange(0.6F, 0.85F);
                emit(p);
            }
            break;
        }
        case EffectKind::Streamers:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = randRange(-params.spread, params.spread);
                p.y = randRange(0.5F, 1.0F);
                p.z = randRange(-0.2F, 0.2F);
                p.vx = randRange(-0.6F, 0.6F) * params.speed;
                p.vy = randRange(-0.3F, 0.1F) * params.speed;
                p.ay = -0.3F;
                p.life = p.maxLife = params.duration * randRange(1.0F, 1.5F);
                p.size = randRange(0.3F, 0.8F);
                const float h = randRange(0.0F, 1.0F);
                p.r = 0.5F + 0.5F * std::cos(6.2831853F * h);
                p.g = 0.5F + 0.5F * std::cos(6.2831853F * (h - 0.333F));
                p.b = 0.5F + 0.5F * std::cos(6.2831853F * (h + 0.333F));
                emit(p);
            }
            break;
        }
        case EffectKind::ScanGrid:
        {
            // Вспышка сканирующей сетки вокруг точки (cyan).
            const int gx = 16;
            const int gy = 10;
            const float ext = params.spread > 0.0F ? params.spread : 0.8F;
            for (int i = 0; i < n; ++i)
            {
                const int ix = i % gx;
                const int iy = i / gx;
                Particle p{};
                p.x = params.originX + (-1.0F + 2.0F * static_cast<float>(ix) / (gx - 1)) * ext;
                p.y = params.originY + (-1.0F + 2.0F * static_cast<float>(iy) / (gy - 1)) * ext;
                p.z = randRange(-0.05F, 0.05F);
                p.vx = 0.0F;
                p.vy = 0.0F;
                p.ay = 0.0F;
                p.life = p.maxLife = params.duration * randRange(0.4F, 0.8F);
                p.size = randRange(0.15F, 0.3F);
                p.r = 0.2F;
                p.g = 0.9F;
                p.b = 1.0F;
                emit(p);
            }
            break;
        }
        case EffectKind::Assemble:
        {
            // Частицы летят к точке сборки (originX/originY).
            for (int i = 0; i < n; ++i)
            {
                const float ang = randRange(0.0F, 6.2831853F);
                const float rad = randRange(0.5F, 1.0F);
                Particle p{};
                p.x = params.originX + std::cos(ang) * rad;
                p.y = params.originY + std::sin(ang) * rad;
                const float sp = randRange(0.8F, 1.6F) * params.speed;
                p.vx = -std::cos(ang) * sp;
                p.vy = -std::sin(ang) * sp;
                p.ay = 0.0F;
                p.life = p.maxLife = params.duration * randRange(0.6F, 1.0F);
                p.size = randRange(0.2F, 0.4F);
                p.r = 0.4F;
                p.g = 0.8F;
                p.b = 1.0F;
                emit(p);
            }
            break;
        }
        case EffectKind::Smoke:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = params.originX + randRange(-0.2F, 0.2F);
                p.y = params.originY + randRange(-0.1F, 0.1F);
                p.z = randRange(-0.1F, 0.1F);
                p.vx = randRange(-0.05F, 0.05F) * params.speed;
                p.vy = randRange(0.1F, 0.4F) * params.speed;
                p.ay = 0.02F;
                p.life = p.maxLife = params.duration * randRange(0.8F, 1.5F);
                p.size = randRange(0.4F, 0.9F);
                const float g = randRange(0.4F, 0.7F);
                p.r = g;
                p.g = g;
                p.b = g;
                emit(p);
            }
            break;
        }
        case EffectKind::Fire:
        {
            for (int i = 0; i < n; ++i)
            {
                Particle p{};
                p.x = params.originX + randRange(-0.15F, 0.15F);
                p.y = params.originY + randRange(-0.05F, 0.05F);
                p.z = randRange(-0.1F, 0.1F);
                p.vx = randRange(-0.08F, 0.08F) * params.speed;
                p.vy = randRange(0.5F, 1.3F) * params.speed;
                p.ay = 0.05F;
                p.life = p.maxLife = params.duration * randRange(0.2F, 0.5F);
                p.size = randRange(0.2F, 0.5F);
                p.r = 1.0F;
                p.g = randRange(0.3F, 0.8F);
                p.b = 0.05F;
                emit(p);
            }
            break;
        }
        case EffectKind::WarnBurst:
        {
            // Красный радиальный импульс из точки (ошибка / предупреждение).
            for (int i = 0; i < n; ++i)
            {
                const float ang = randRange(0.0F, 6.2831853F);
                const float sp = randRange(0.6F, 1.6F) * params.speed;
                Particle p{};
                p.x = params.originX;
                p.y = params.originY;
                p.z = 0.0F;
                p.vx = std::cos(ang) * sp;
                p.vy = std::sin(ang) * sp;
                p.ay = -0.3F;
                p.life = p.maxLife = params.duration * randRange(0.3F, 0.6F);
                p.size = randRange(0.2F, 0.5F);
                p.r = 1.0F;
                p.g = randRange(0.1F, 0.3F);
                p.b = 0.1F;
                emit(p);
            }
            break;
        }
        case EffectKind::SelectPulse:
        {
            // Расширяющееся кольцо подсветки выбранного.
            const int ring = std::max(1, n);
            for (int i = 0; i < ring; ++i)
            {
                const float ang = 6.2831853F * static_cast<float>(i) / static_cast<float>(ring);
                const float sp = randRange(0.6F, 1.0F) * params.speed;
                Particle p{};
                p.x = params.originX;
                p.y = params.originY;
                p.z = 0.0F;
                p.vx = std::cos(ang) * sp;
                p.vy = std::sin(ang) * sp;
                p.ay = 0.0F;
                p.life = p.maxLife = params.duration * randRange(0.5F, 0.9F);
                p.size = randRange(0.2F, 0.4F);
                p.r = 0.3F;
                p.g = 0.7F;
                p.b = 1.0F;
                emit(p);
            }
            break;
        }
        case EffectKind::Success:
        {
            // Праздничный фейерверк (зелёный / золотой) из точки.
            for (int i = 0; i < n; ++i)
            {
                const float ang = randRange(0.0F, 6.2831853F);
                const float sp = randRange(0.6F, 1.5F) * params.speed;
                Particle p{};
                p.x = params.originX + randRange(-0.1F, 0.1F);
                p.y = params.originY + randRange(-0.1F, 0.1F);
                p.z = 0.0F;
                p.vx = std::cos(ang) * sp;
                p.vy = std::sin(ang) * sp;
                p.ay = -0.9F;
                p.life = p.maxLife = params.duration * randRange(0.5F, 0.9F);
                p.size = randRange(0.2F, 0.5F);
                const bool gold = rand() > 0.5F;
                p.r = gold ? 1.0F : 0.3F;
                p.g = gold ? 0.9F : 1.0F;
                p.b = gold ? 0.3F : 0.4F;
                emit(p);
            }
            break;
        }
        case EffectKind::Count:
        default:
            break;
    }

    std::fprintf(stderr,
                 "[VFX] trigger kind=%d spawned=%d (live=%zu)\n",
                 static_cast<int>(kind),
                 spawned,
                 m_particles.size());
    return spawned;
}

} // namespace MirEngine::VFX
