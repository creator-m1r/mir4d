#pragma once

#include <cstdint>

namespace MirEngine::VFX
{

/// Visual effect kinds owned by the MIR 4D VFX subsystem.
/// Values are stable and mirrored in the C ABI (MirEngineVFXTrigger).
enum class EffectKind : int
{
    Confetti = 0,
    Balloons = 1,
    Fireworks = 2,
    Rain = 3,
    Hearts = 4,
    Lasers = 5,
    // ── Лёгкие (точечные) эффекты ──
    Sparks = 6,        // искры при сварке/резке
    Snow = 7,          // снег
    Bubbles = 8,       // пузыри
    Sparkles = 9,      // блёстки / звёздочки
    Petals = 10,       // лепестки
    Streamers = 11,    // серпантин / стриммеры
    ScanGrid = 12,     // сканирующая сетка / лазерный скан
    Assemble = 13,     // частицы сборки узла (летят к точке)
    // ── Промышленные / CAD ──
    Smoke = 14,        // дым / пар
    Fire = 15,         // огонь
    WarnBurst = 16,    // всплеск ошибки / предупреждения
    SelectPulse = 17,  // пульс подсветки выбранного
    Success = 18,      // успешная проверка / расчёт (галочка-фейерверк)
    Count
};

/// CPU-side particle. Positions are in normalized viewport space
/// [-1, 1] (x right, y up, z toward viewer) so the subsystem stays
/// renderer-agnostic; the active sink maps them to clip/pixel space.
struct Particle
{
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};

    float vx{0.0F};
    float vy{0.0F};
    float vz{0.0F};

    float ay{0.0F};        // per-particle acceleration on y (world up positive)

    float life{0.0F};      // remaining lifetime, seconds
    float maxLife{1.0F};

    float size{1.0F};      // relative size, 1 = nominal
    float spin{0.0F};      // angular velocity, radians/sec

    float r{1.0F};         // color components, 0..1
    float g{1.0F};
    float b{1.0F};
};

/// Spawn parameters for a single effect burst.
struct EffectParams
{
    int count{120};        // particles to spawn
    float originX{0.0F};   // normalized [-1,1]
    float originY{0.0F};   // normalized [-1,1]
    float spread{0.6F};    // spawn region half-size
    float speed{1.0F};     // base speed multiplier
    float duration{2.5F};  // particle lifetime, seconds
};

} // namespace MirEngine::VFX
