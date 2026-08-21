// MirEngine/Math/Numeric/SpecialFunctions.hpp
// 🧮 Специальные функции: функции Бесселя Jₙ/Yₙ и Бета-функция.
//
// J₀, J₁, Y₀, Y₁ — рациональные аппроксимации (по Numerical Recipes,
// точность ~1e-7). Общий целый порядок n — устойчивыми рекуррентными
// схемами (вверх для x>n, вниз по Миллеру для x≤n).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Statistics.hpp"

#include <cmath>
#include <limits>
#include <numbers>
#include <vector>

namespace mir::math
{

/// J₀(x) — функция Бесселя первого рода нулевого порядка.
[[nodiscard]] inline Scalar besselJ0(Scalar x)
{
    if (x == Scalar(0))
        return Scalar(1);
    const Scalar ax = std::abs(x);
    if (ax < Scalar(8))
    {
        const Scalar y = x * x;
        const Scalar a1 = Scalar(57568490574.0) + y * (Scalar(-13362590354.0) + y * (Scalar(651619640.7) +
            y * (Scalar(-11214424.18) + y * (Scalar(77392.33017) + y * (Scalar(-184.9052456))))));
        const Scalar a2 = Scalar(57568490411.0) + y * (Scalar(1029532985.0) + y * (Scalar(9494680.718) +
            y * (Scalar(59272.64853) + y * (Scalar(267.8532712) + y))));
        return a1 / a2;
    }
    const Scalar z = Scalar(8) / ax;
    const Scalar y = z * z;
    const Scalar xx = ax - Scalar(0.785398164);
    const Scalar a1 = Scalar(1) + y * (Scalar(-0.1098628627e-2) + y * (Scalar(0.2734510407e-4) +
        y * (Scalar(-0.2073370639e-5) + y * Scalar(0.2093887211e-6))));
    const Scalar a2 = Scalar(-0.1562499995e-1) + y * (Scalar(0.1430488765e-3) +
        y * (Scalar(-0.6911147651e-5) + y * (Scalar(0.7621095161e-6) - y * Scalar(0.934935152e-7))));
    return std::sqrt(Scalar(0.636619772) / ax) * (std::cos(xx) * a1 - z * std::sin(xx) * a2);
}

/// J₁(x) — функция Бесселя первого рода первого порядка.
[[nodiscard]] inline Scalar besselJ1(Scalar x)
{
    if (x == Scalar(0))
        return Scalar(0);
    const Scalar ax = std::abs(x);
    if (ax < Scalar(8))
    {
        const Scalar y = x * x;
        const Scalar a1 = x * (Scalar(72362614232.0) + y * (Scalar(-7895059235.0) + y * (Scalar(242396853.1) +
            y * (Scalar(-2972611.439) + y * (Scalar(15704.48260) + y * (Scalar(-30.16036606)))))));
        const Scalar a2 = Scalar(144725228442.0) + y * (Scalar(2300535178.0) + y * (Scalar(18583304.74) +
            y * (Scalar(99447.43394) + y * (Scalar(376.9991397) + y))));
        Scalar ans = a1 / a2;
        return (x < Scalar(0)) ? -ans : ans;
    }
    const Scalar z = Scalar(8) / ax;
    const Scalar y = z * z;
    const Scalar xx = ax - Scalar(2.356194491);
    const Scalar a1 = Scalar(1) + y * (Scalar(0.183105e-2) + y * (Scalar(-0.3516396496e-4) +
        y * (Scalar(0.2457520174e-5) - y * Scalar(0.240337019e-6))));
    const Scalar a2 = Scalar(0.04687499995) + y * (Scalar(-0.2002690873e-3) + y * (Scalar(0.8449199096e-5) +
        y * (Scalar(-0.88228987e-6) + y * Scalar(0.105787412e-6))));
    Scalar ans = std::sqrt(Scalar(0.636619772) / ax) * (std::cos(xx) * a1 - z * std::sin(xx) * a2);
    return (x < Scalar(0)) ? -ans : ans;
}

/// Jₙ(x) — функция Бесселя первого рода целого порядка n ≥ 0.
[[nodiscard]] inline Scalar besselJ(int n, Scalar x)
{
    if (n < 0)
        return (n % 2 == 0 ? besselJ(-n, x) : -besselJ(-n, x));
    if (n == 0)
        return besselJ0(x);
    if (n == 1)
        return besselJ1(x);

    const Scalar ax = std::abs(x);
    if (ax == Scalar(0))
        return Scalar(0);

    if (ax > static_cast<Scalar>(n))
    {
        const Scalar tox = Scalar(2) / ax;
        Scalar bjm = besselJ0(ax);
        Scalar bj = besselJ1(ax);
        for (int j = 1; j < n; ++j)
        {
            const Scalar bjp = static_cast<Scalar>(j) * tox * bj - bjm;
            bjm = bj;
            bj = bjp;
        }
        return (x < Scalar(0) && (n % 2 != 0)) ? -bj : bj;
    }

    // Вниз по Миллеру (устойчиво при x ≤ n).
    const Scalar tox = Scalar(2) / ax;
    const int m = 2 * (n + static_cast<int>(std::sqrt(Scalar(40) * static_cast<Scalar>(n))) / 2);
    Scalar bjp = Scalar(0);
    Scalar bj = Scalar(1);
    Scalar ans = Scalar(0);
    for (int j = m; j > 0; --j)
    {
        const Scalar bjm = static_cast<Scalar>(j) * tox * bj - bjp;
        bjp = bj;
        bj = bjm;
        if (std::abs(bj) > Scalar(1e10))
        {
            bj *= Scalar(1e-10);
            bjp *= Scalar(1e-10);
            ans *= Scalar(1e-10);
        }
        if (j == n)
            ans = bjp;
    }
    ans *= besselJ0(ax) / bj;
    return (x < Scalar(0) && (n % 2 != 0)) ? -ans : ans;
}

/// Y₀(x) — функция Бесселя второго рода (Неймана) нулевого порядка.
[[nodiscard]] inline Scalar besselY0(Scalar x)
{
    const Scalar ax = std::abs(x);
    if (ax < Scalar(8))
    {
        const Scalar y = x * x;
        const Scalar a1 = Scalar(-2957821389.0) + y * (Scalar(7062834065.0) + y * (Scalar(-512359803.6) +
            y * (Scalar(10879881.29) + y * (Scalar(-86327.92757) + y * Scalar(228.4622733)))));
        const Scalar a2 = Scalar(40076544269.0) + y * (Scalar(745249964.8) + y * (Scalar(7189466.438) +
            y * (Scalar(47447.26470) + y * (Scalar(226.1030244) + y))));
        return a1 / a2 + Scalar(0.636619772) * std::log(ax) * besselJ0(ax);
    }
    const Scalar z = Scalar(8) / ax;
    const Scalar y = z * z;
    const Scalar xx = ax - Scalar(0.785398164);
    const Scalar a1 = Scalar(1) + y * (Scalar(-0.1098628627e-2) + y * (Scalar(0.2734510407e-4) +
        y * (Scalar(-0.2073370639e-5) + y * Scalar(0.2093887211e-6))));
    const Scalar a2 = Scalar(-0.1562499995e-1) + y * (Scalar(0.1430488765e-3) +
        y * (Scalar(-0.6911147651e-5) + y * (Scalar(0.7621095161e-6) - y * Scalar(0.934935152e-7))));
    return std::sqrt(Scalar(0.636619772) / ax) * (std::sin(xx) * a1 + z * std::cos(xx) * a2);
}

/// Y₁(x) — функция Бесселя второго рода первого порядка.
/// Вычисляется точным тождеством Вронского J₁·Y₀ − J₀·Y₁ = 2/(π·x),
/// опираясь на уже точные besselJ0/besselJ1/besselY0.
[[nodiscard]] inline Scalar besselY1(Scalar x)
{
    const Scalar ax = std::abs(x);
    if (ax == Scalar(0))
        return (x == Scalar(0)) ? -std::numeric_limits<Scalar>::infinity() : std::numeric_limits<Scalar>::infinity();
    const Scalar j0 = besselJ0(ax);
    // Y₁ = (J₁·Y₀ − 2/(πx)) / J₀  (при малых |J₀| — асимптотика x>8).
    if (std::abs(j0) < Scalar(1e-4) && ax > Scalar(8))
    {
        const Scalar z = Scalar(8) / ax;
        const Scalar y = z * z;
        const Scalar xx = ax - Scalar(2.356194491);
        const Scalar a1 = Scalar(1) + y * (Scalar(0.183105e-2) + y * (Scalar(-0.3516396496e-4) +
            y * (Scalar(0.2457520174e-5) - y * Scalar(0.240337019e-6))));
        const Scalar a2 = Scalar(0.04687499995) + y * (Scalar(-0.2002690873e-3) + y * (Scalar(0.8449199096e-5) +
            y * (Scalar(-0.88228987e-6) + y * Scalar(0.105787412e-6))));
        return std::sqrt(Scalar(0.636619772) / ax) * (std::sin(xx) * a1 + z * std::cos(xx) * a2);
    }
    const Scalar y1 = (besselJ1(ax) * besselY0(ax) - Scalar(2) / (std::numbers::pi_v<Scalar> * ax)) / j0;
    return (x < Scalar(0)) ? -y1 : y1;
}

/// Yₙ(x) — функция Бесселя второго рода целого порядка n ≥ 0.
[[nodiscard]] inline Scalar besselY(int n, Scalar x)
{
    if (n < 0)
        return (n % 2 == 0 ? besselY(-n, x) : -besselY(-n, x));
    if (n == 0)
        return besselY0(x);
    if (n == 1)
        return besselY1(x);

    const Scalar tox = Scalar(2) / x;
    Scalar bjm = besselY0(x);
    Scalar bj = besselY1(x);
    for (int j = 1; j < n; ++j)
    {
        const Scalar bjp = static_cast<Scalar>(j) * tox * bj - bjm;
        bjm = bj;
        bj = bjp;
    }
    return bj;
}

/// I₀(x) — модифицированная функция Бесселя первого рода нулевого порядка.
[[nodiscard]] inline Scalar besselI0(Scalar x)
{
    const Scalar ax = std::abs(x);
    if (ax < Scalar(3.75))
    {
        const Scalar y = (x / Scalar(3.75)) * (x / Scalar(3.75));
        return Scalar(1) + y * (Scalar(3.5156229) + y * (Scalar(3.0899424) + y * (Scalar(1.2067492) +
            y * (Scalar(0.2659732) + y * (Scalar(0.0360768) + y * Scalar(0.0045813))))));
    }
    const Scalar y = Scalar(3.75) / ax;
    return std::exp(ax) / std::sqrt(ax) * (Scalar(0.39894228) + y * (Scalar(0.001328592) +
        y * (Scalar(0.00225319) + y * (Scalar(-0.00157565) + y * (Scalar(0.00916281) +
        y * (Scalar(-0.02057706) + y * Scalar(0.00936543)))))));
}

/// I₁(x) — модифицированная функция Бесселя первого рода первого порядка.
[[nodiscard]] inline Scalar besselI1(Scalar x)
{
    const Scalar ax = std::abs(x);
    if (ax < Scalar(3.75))
    {
        const Scalar y = (x / Scalar(3.75)) * (x / Scalar(3.75));
        Scalar ans = ax * (Scalar(0.5) + y * (Scalar(0.87890594) + y * (Scalar(0.51498869) +
            y * (Scalar(0.15084934) + y * (Scalar(0.02658733) + y * Scalar(0.00301532))))));
        return (x < Scalar(0)) ? -ans : ans;
    }
    const Scalar y = Scalar(3.75) / ax;
    Scalar ans = std::exp(ax) / std::sqrt(ax) * (Scalar(0.39894228) + y * (Scalar(-0.03988024) +
        y * (Scalar(-0.00362018) + y * (Scalar(0.00163801) + y * (Scalar(-0.01031555) +
        y * Scalar(0.02282967))))));
    return (x < Scalar(0)) ? -ans : ans;
}

/// Iₙ(x) — модифицированная функция Бесселя первого рода целого порядка n ≥ 0.
[[nodiscard]] inline Scalar besselI(int n, Scalar x)
{
    if (n < 0)
        return (n % 2 == 0 ? besselI(-n, x) : -besselI(-n, x));
    if (n == 0)
        return besselI0(x);
    if (n == 1)
        return besselI1(x);
    const Scalar ax = std::abs(x);
    if (ax == Scalar(0))
        return Scalar(0);
    if (ax > static_cast<Scalar>(n))
    {
        const Scalar tox = Scalar(2) / ax;
        Scalar bim = besselI0(ax);
        Scalar bi = besselI1(ax);
        for (int j = 1; j < n; ++j)
        {
            const Scalar bip = bim + static_cast<Scalar>(j) * tox * bi;
            bim = bi;
            bi = bip;
        }
        return (x < Scalar(0) && (n % 2 != 0)) ? -bi : bi;
    }
    const Scalar tox = Scalar(2) / ax;
    const int m = 2 * (n + static_cast<int>(std::sqrt(Scalar(40) * static_cast<Scalar>(n))) / 2);
    Scalar bip = Scalar(0);
    Scalar bi = Scalar(1);
    Scalar ans = Scalar(0);
    for (int j = m; j > 0; --j)
    {
        const Scalar bim = bip + static_cast<Scalar>(j) * tox * bi;
        bip = bi;
        bi = bim;
        if (std::abs(bi) > Scalar(1e10))
        {
            bi *= Scalar(1e-10);
            bip *= Scalar(1e-10);
            ans *= Scalar(1e-10);
        }
        if (j == n)
            ans = bip;
    }
    ans *= besselI0(ax) / bi;
    return (x < Scalar(0) && (n % 2 != 0)) ? -ans : ans;
}

namespace detail
{
/// K_μ(x) = ∫₀^∞ e^{−x·cosh t}·cosh(μ·t) dt  (численное интегрирование,
/// устойчиво для 0 < x ≤ 2). Используется как эталонная реализация K₀/K₁,
/// поскольку замкнутые ряды вблизи x≈2 нестабильны.
[[nodiscard]] inline Scalar besselKint(int mu, Scalar x)
{
    const Scalar tmax = std::max(std::log(Scalar(80) / x) + Scalar(2), Scalar(8));
    const Scalar dt = Scalar(5e-4);
    Scalar sum = Scalar(0);
    for (Scalar t = Scalar(0); t < tmax; t += dt)
        sum += std::exp(-x * std::cosh(t)) * std::cosh(static_cast<Scalar>(mu) * t) * dt;
    return sum;
}
} // namespace detail

/// K₀(x) — модифицированная функция Бесселя второго рода нулевого порядка.
/// Для x ≥ 2 — асимптотика (точность ~1e-7); для 0 < x < 2 — интегрирование.
[[nodiscard]] inline Scalar besselK0(Scalar x)
{
    if (x == Scalar(0))
        return std::numeric_limits<Scalar>::infinity();
    if (x < Scalar(0))
        return std::numeric_limits<Scalar>::quiet_NaN();
    if (x >= Scalar(2))
    {
        const Scalar y = Scalar(2) / x;
        return std::exp(-x) / std::sqrt(x) * (Scalar(1.25331414) + y * (Scalar(-0.07832358) +
            y * (Scalar(0.02189568) + y * (Scalar(-0.01062446) + y * Scalar(0.00587872)))));
    }
    return detail::besselKint(0, x);
}

/// K₁(x) — модифицированная функция Бесселя второго рода первого порядка.
[[nodiscard]] inline Scalar besselK1(Scalar x)
{
    if (x == Scalar(0))
        return std::numeric_limits<Scalar>::infinity();
    if (x < Scalar(0))
        return std::numeric_limits<Scalar>::quiet_NaN();
    if (x >= Scalar(2))
    {
        const Scalar y = Scalar(2) / x;
        return std::exp(-x) / std::sqrt(x) * (Scalar(1.25331414) + y * (Scalar(0.23498619) +
            y * (Scalar(-0.03655620) + y * (Scalar(0.01504268) + y * Scalar(-0.00780353)))));
    }
    return detail::besselKint(1, x);
}

/// Kₙ(x) — модифицированная функция Бесселя второго рода целого порядка n ≥ 0.
[[nodiscard]] inline Scalar besselK(int n, Scalar x)
{
    if (n < 0)
        return (n % 2 == 0 ? besselK(-n, x) : -besselK(-n, x));
    if (n == 0)
        return besselK0(x);
    if (n == 1)
        return besselK1(x);
    const Scalar tox = Scalar(2) / x;
    Scalar bkm = besselK0(x);
    Scalar bk = besselK1(x);
    for (int j = 1; j < n; ++j)
    {
        const Scalar bkp = bkm + static_cast<Scalar>(j) * tox * bk;
        bkm = bk;
        bk = bkp;
    }
    return bk;
}

/// Бета-функция B(a, b) = Γ(a)·Γ(b) / Γ(a+b).
[[nodiscard]] inline Scalar betaFunction(Scalar a, Scalar b)
{
    return std::exp(logGamma(a) + logGamma(b) - logGamma(a + b));
}

} // namespace mir::math
