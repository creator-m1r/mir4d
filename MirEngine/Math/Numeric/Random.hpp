// MirEngine/Math/Numeric/Random.hpp
// 🧮 Генератор случайных чисел — основа Монте-Карло и стохастических симуляций.
//
// Детерминированный генератор на базе xorshift64* (быстрый, качественный).
// Сид задаётся явно для воспроизводимости инженерных расчётов.
//
// Распределения:
//   • nextDouble / nextRange  — равномерное на отрезке;
//   • nextInt                  — равномерное целое;
//   • normal                   — нормальное N(mean, std) (Бокс–Мюллер);
//   • exponential              — экспоненциальное Exp(lambda).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Types/Scalar.hpp"

#include <cmath>
#include <cstdint>
#include <utility>

namespace mir::math
{

/// Генератор псевдослучайных чисел с явным сидом (xorshift64*).
class Random
{
public:
    /// Создаёт генератор с заданным сидом (по умолчанию 0x9E3779B97F4A7C15).
    explicit Random(std::uint64_t seed = DefaultSeed) noexcept
        : state_(seed == 0 ? DefaultSeed : seed)
    {
    }

    /// Возвращает 64-битное целое без знака из полного диапазона.
    [[nodiscard]] std::uint64_t nextU64() noexcept
    {
        std::uint64_t x = state_;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        state_ = x;
        return x * Uint64Mult;
    }

    /// Равномерное вещественное в диапазоне [0, 1).
    [[nodiscard]] Scalar nextDouble() noexcept
    {
        return unit();
    }

    /// Равномерное вещественное в диапазоне [a, b).
    [[nodiscard]] Scalar nextRange(Scalar a, Scalar b) noexcept
    {
        return a + (b - a) * unit();
    }

    /// Равномерное целое в диапазоне [lo, hi] включительно.
    [[nodiscard]] std::int64_t nextInt(std::int64_t lo, std::int64_t hi) noexcept
    {
        if (hi <= lo)
            return lo;
        const std::uint64_t range = static_cast<std::uint64_t>(hi - lo) + 1;
        return lo + static_cast<std::int64_t>(nextU64() % range);
    }

    /// Нормальное распределение N(mean, std) методом Бокса–Мюллера.
    [[nodiscard]] Scalar normal(Scalar mean = Scalar(0), Scalar std = Scalar(1)) noexcept
    {
        // Полярный метод Марсальи для устойчивости.
        Scalar u = Scalar(0);
        Scalar v = Scalar(0);
        Scalar s = Scalar(0);
        do
        {
            u = unit() * Scalar(2) - Scalar(1);
            v = unit() * Scalar(2) - Scalar(1);
            s = u * u + v * v;
        } while (s <= Scalar(0) || s >= Scalar(1));

        const Scalar mul = std::sqrt((Scalar(-2) * std::log(s)) / s);
        // Возвращаем одно из двух независимых значений (второе: v*mul).
        return mean + (u * mul) * std;
    }

    /// Экспоненциальное распределение Exp(lambda), lambda > 0.
    [[nodiscard]] Scalar exponential(Scalar lambda = Scalar(1)) noexcept
    {
        const Scalar u = unit() <= Scalar(1e-12) ? Scalar(1e-12) : unit();
        return -std::log(u) / lambda;
    }

    /// Сбрасывает сид (для воспроизводимых повторов).
    void reseed(std::uint64_t seed) noexcept
    {
        state_ = (seed == 0 ? DefaultSeed : seed);
    }

private:
    [[nodiscard]] Scalar unit() noexcept
    {
        // 53-битное равномерное на [0,1).
        const std::uint64_t x = nextU64() >> 11;
        return static_cast<Scalar>(x) * Scale53;
    }

    static constexpr std::uint64_t DefaultSeed = 0x9E3779B97F4A7C15ULL;
    static constexpr std::uint64_t Uint64Mult = 0x2545F4914F6CDD1DULL;
    static constexpr Scalar Scale53 = Scalar(1) / static_cast<Scalar>(std::uint64_t(1) << 53);

    std::uint64_t state_;
};

} // namespace mir::math
