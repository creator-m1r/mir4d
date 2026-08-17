// MirEngine/Math/Numeric/Transforms.hpp
// 🧮 Интегральные преобразования: непрерывное преобразование Фурье (через БПФ)
// и преобразование Лапласа (численное интегрирование).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Fft.hpp"
#include "NumericalAnalysis.hpp"

#include <complex>
#include <cmath>
#include <numbers>
#include <functional>
#include <vector>

namespace mir::math
{

using Cx = std::complex<Scalar>;

/// Результат непрерывного преобразования Фурье: спектр и частоты ωₖ.
struct FourierResult
{
    std::vector<Cx> spectrum;  // F(ωₖ)
    std::vector<Scalar> omegas; // ωₖ (центрированные)
};

/// Численное непрерывное преобразование Фурье F(ω) = ∫ f(t)e^{-iωt} dt,
/// вычисляемое на равномерной сетке [tMin, tMax] из n (степень двойки) узлов.
inline FourierResult continuousFourierTransform(
    std::function<Scalar(Scalar)> f,
    Scalar tMin,
    Scalar tMax,
    int n)
{
    FourierResult res;
    res.spectrum.resize(n);
    res.omegas.resize(n);
    if (n <= 0)
        return res;

    const Scalar dt = (tMax - tMin) / static_cast<Scalar>(n);
    std::vector<Cx> samples(n);
    for (int j = 0; j < n; ++j)
    {
        const Scalar t = tMin + static_cast<Scalar>(j) * dt;
        samples[j] = Cx(f(t) * dt, Scalar(0));
    }
    fftInPlace(samples, false);

    const Scalar dOmega = Scalar(2) * std::numbers::pi_v<Scalar> / (static_cast<Scalar>(n) * dt);
    for (int k = 0; k < n; ++k)
    {
        const int kc = (k < n / 2) ? k : (k - n);
        res.omegas[k] = static_cast<Scalar>(kc) * dOmega;
        // Сдвиг фазы для учёта ненулевого tMin.
        const Scalar phase = -dOmega * static_cast<Scalar>(kc) * tMin;
        // FFT уже дала Σ f_j e^{-i2πjk/n}; домножаем на фазовый множитель.
        samples[k] *= Cx(std::cos(phase), std::sin(phase));
    }
    res.spectrum = std::move(samples);
    return res;
}

/// Обратное непрерывное преобразование Фурье; восстанавливает f на сетке.
inline std::vector<Scalar> inverseFourierTransform(
    const std::vector<Cx>& spectrum,
    Scalar tMin,
    Scalar tMax,
    int n)
{
    std::vector<Scalar> f(n, Scalar(0));
    if (n <= 0 || spectrum.size() != static_cast<std::size_t>(n))
        return f;

    const Scalar dt = (tMax - tMin) / static_cast<Scalar>(n);
    const Scalar dOmega = Scalar(2) * std::numbers::pi_v<Scalar> / (static_cast<Scalar>(n) * dt);
    std::vector<Cx> G = spectrum;
    for (int k = 0; k < n; ++k)
    {
        const int kc = (k < n / 2) ? k : (k - n);
        const Scalar phase = dOmega * static_cast<Scalar>(kc) * tMin;
        G[k] *= Cx(std::cos(phase), std::sin(phase));
    }
    fftInPlace(G, true); // делит на n
    for (int j = 0; j < n; ++j)
        f[j] = G[j].real() / dt;
    return f;
}

namespace detail
{

inline Cx adaptiveSimpsonC(
    const std::function<Cx(Scalar)>& g,
    Scalar a,
    Scalar b,
    Cx fa,
    Cx fm,
    Cx fb,
    Cx whole,
    Scalar tol,
    int depth)
{
    const Scalar h = (b - a) / Scalar(2);
    const Scalar lm = a + h / Scalar(2);
    const Scalar rm = b - h / Scalar(2);
    const Cx flm = g(lm);
    const Cx frm = g(rm);
    const Cx left = h * (fa + Scalar(4) * fm + flm) / Scalar(6);
    const Cx right = h * (fm + Scalar(4) * frm + fb) / Scalar(6);
    const Cx diff = left + right - whole;
    if (depth <= 0 || std::abs(diff) <= Scalar(15) * tol)
        return left + right + diff / Scalar(15);
    return adaptiveSimpsonC(g, a, (a + b) / Scalar(2), fa, flm, fm, h * (fa + Scalar(4) * flm + fm) / Scalar(6), tol / Scalar(2), depth - 1) +
        adaptiveSimpsonC(g, (a + b) / Scalar(2), b, fm, frm, fb, h * (fm + Scalar(4) * frm + fb) / Scalar(6), tol / Scalar(2), depth - 1);
}

} // namespace detail

/// Численное преобразование Лапласа F(s) = ∫₀^∞ e^{-st} f(t) dt (Re(s) > 0).
[[nodiscard]] inline Cx laplaceTransform(std::function<Scalar(Scalar)> f, const Cx& s)
{
    const Scalar re = std::real(s);
    const Scalar T = Scalar(50) / std::max(re, Scalar(1e-3));
    const std::function<Cx(Scalar)> g = [&](Scalar t) {
        return std::exp(-s * t) * Cx(f(t), Scalar(0));
    };
    const Scalar a = Scalar(0);
    const Scalar b = T;
    const Cx fa = g(a);
    const Cx fb = g(b);
    const Cx fm = g((a + b) / Scalar(2));
    const Cx whole = (b - a) * (fa + Scalar(4) * fm + fb) / Scalar(6);
    return detail::adaptiveSimpsonC(g, a, b, fa, fm, fb, whole, Scalar(1e-11), 14);
}

} // namespace mir::math
