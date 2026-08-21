
#pragma once

#include "../../Core/Types/Scalar.hpp"

#include <cmath>
#include <cstdint>
#include <utility>

namespace mir::math
{

class Random
{
public:

    explicit Random(std::uint64_t seed = DefaultSeed) noexcept
        : state_(seed == 0 ? DefaultSeed : seed)
    {
    }

    [[nodiscard]] std::uint64_t nextU64() noexcept
    {
        std::uint64_t x = state_;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        state_ = x;
        return x * Uint64Mult;
    }

    [[nodiscard]] Scalar nextDouble() noexcept
    {
        return unit();
    }

    [[nodiscard]] Scalar nextRange(Scalar a, Scalar b) noexcept
    {
        return a + (b - a) * unit();
    }

    [[nodiscard]] std::int64_t nextInt(std::int64_t lo, std::int64_t hi) noexcept
    {
        if (hi <= lo)
            return lo;
        const std::uint64_t range = static_cast<std::uint64_t>(hi - lo) + 1;
        return lo + static_cast<std::int64_t>(nextU64() % range);
    }

    [[nodiscard]] Scalar normal(Scalar mean = Scalar(0), Scalar std = Scalar(1)) noexcept
    {

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

        return mean + (u * mul) * std;
    }

    [[nodiscard]] Scalar exponential(Scalar lambda = Scalar(1)) noexcept
    {
        const Scalar u = unit() <= Scalar(1e-12) ? Scalar(1e-12) : unit();
        return -std::log(u) / lambda;
    }

    void reseed(std::uint64_t seed) noexcept
    {
        state_ = (seed == 0 ? DefaultSeed : seed);
    }

private:
    [[nodiscard]] Scalar unit() noexcept
    {

        const std::uint64_t x = nextU64() >> 11;
        return static_cast<Scalar>(x) * Scale53;
    }

    static constexpr std::uint64_t DefaultSeed = 0x9E3779B97F4A7C15ULL;
    static constexpr std::uint64_t Uint64Mult = 0x2545F4914F6CDD1DULL;
    static constexpr Scalar Scale53 = Scalar(1) / static_cast<Scalar>(std::uint64_t(1) << 53);

    std::uint64_t state_;
};

}
