// MirEngine/Math/Numeric/Statistics.hpp
// 🧮 Статистика и специальные функции — математическое ядро MIR 4D.
//
// Статистика:         mean, variance, stdDev, median, minimum, maximum, sum,
//                     dot, covariance, pearson (корреляция Пирсона),
//                     linearRegression (МНК через solveLeastSquares).
// Спецфункции:        factorial, binomialCoefficient, logGamma, gammaFunction,
//                     erf, erfc.
//
// Функции с некорректными входными данными возвращают mir4d::Result.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Result.hpp"
#include "../../Core/Types/Scalar.hpp"
#include "LinearSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace mir::math
{

#ifndef MIR_MATH_NUMERIC_FAIL_DEFINED
#define MIR_MATH_NUMERIC_FAIL_DEFINED
[[nodiscard]] inline auto fail(mir4d::ErrorCode code, std::string_view message)
{
    return std::unexpected(mir4d::Error(code, message));
}
#endif

// ═══════════════════════════════════════════════════════════════
//  Описательная статистика
// ═══════════════════════════════════════════════════════════════

/// Сумма элементов выборки.
[[nodiscard]] inline Scalar sum(const std::vector<Scalar>& values)
{
    Scalar total = Scalar(0);
    for (const Scalar v : values)
        total += v;
    return total;
}

/// Среднее арифметическое. Пустая выборка → ошибку.
[[nodiscard]] inline mir4d::Result<Scalar> mean(const std::vector<Scalar>& values)
{
    if (values.empty())
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая выборка");
    return mir4d::success(sum(values) / static_cast<Scalar>(values.size()));
}

/// Дисперсия. sample=true — выборочная (n−1), иначе генеральная (n).
[[nodiscard]] inline mir4d::Result<Scalar> variance(
    const std::vector<Scalar>& values,
    bool sample = true)
{
    if (values.size() < (sample ? 2 : 1))
        return fail(mir4d::ErrorCode::InvalidArgument, "Недостаточно данных для дисперсии");

    const Scalar m = mean(values).value();
    Scalar acc = Scalar(0);
    for (const Scalar v : values)
    {
        const Scalar d = v - m;
        acc += d * d;
    }

    const Scalar denom = static_cast<Scalar>(values.size() - (sample ? 1 : 0));
    return mir4d::success(acc / denom);
}

/// Среднеквадратичное отклонение.
[[nodiscard]] inline mir4d::Result<Scalar> stdDev(
    const std::vector<Scalar>& values,
    bool sample = true)
{
    auto v = variance(values, sample);
    if (!v)
        return v;
    return mir4d::success(std::sqrt(v.value()));
}

/// Медиана выборки (не разрушает аргумент).
[[nodiscard]] inline mir4d::Result<Scalar> median(std::vector<Scalar> values)
{
    if (values.empty())
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая выборка");

    std::sort(values.begin(), values.end());
    const std::size_t n = values.size();
    if (n % 2 == 1)
        return mir4d::success(values[n / 2]);
    return mir4d::success((values[n / 2 - 1] + values[n / 2]) * Scalar(0.5));
}

/// Минимум выборки.
[[nodiscard]] inline mir4d::Result<Scalar> minimum(const std::vector<Scalar>& values)
{
    if (values.empty())
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая выборка");
    Scalar m = values.front();
    for (const Scalar v : values)
        if (v < m)
            m = v;
    return mir4d::success(m);
}

/// Максимум выборки.
[[nodiscard]] inline mir4d::Result<Scalar> maximum(const std::vector<Scalar>& values)
{
    if (values.empty())
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая выборка");
    Scalar m = values.front();
    for (const Scalar v : values)
        if (v > m)
            m = v;
    return mir4d::success(m);
}

/// Скалярное произведение двух выборок одинаковой длины.
[[nodiscard]] inline mir4d::Result<Scalar> dot(
    const std::vector<Scalar>& a,
    const std::vector<Scalar>& b)
{
    if (a.size() != b.size())
        return fail(mir4d::ErrorCode::InvalidArgument, "Разные длины выборок");
    Scalar acc = Scalar(0);
    for (std::size_t i = 0; i < a.size(); ++i)
        acc += a[i] * b[i];
    return mir4d::success(acc);
}

/// Ковариация двух выборок (sample, деление на n−1).
[[nodiscard]] inline mir4d::Result<Scalar> covariance(
    const std::vector<Scalar>& a,
    const std::vector<Scalar>& b)
{
    if (a.size() != b.size() || a.size() < 2)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные выборки для ковариации");

    const Scalar ma = mean(a).value();
    const Scalar mb = mean(b).value();
    Scalar acc = Scalar(0);
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        const Scalar da = a[i] - ma;
        const Scalar db = b[i] - mb;
        acc += da * db;
    }
    return mir4d::success(acc / static_cast<Scalar>(a.size() - 1));
}

/// Коэффициент корреляции Пирсона ∈ [−1, 1].
[[nodiscard]] inline mir4d::Result<Scalar> pearson(
    const std::vector<Scalar>& a,
    const std::vector<Scalar>& b)
{
    auto cov = covariance(a, b);
    auto sa = stdDev(a);
    auto sb = stdDev(b);
    if (!cov || !sa || !sb)
        return cov ? cov : sa;

    const Scalar denom = sa.value() * sb.value();
    if (std::abs(denom) <= Scalar(1e-14))
        return fail(mir4d::ErrorCode::ValidationFailed, "Нулевое СКО");

    return mir4d::success(cov.value() / denom);
}

/// Линейная регрессия y = slope·x + intercept методом наименьших квадратов.
/// Возвращает пару (slope, intercept).
[[nodiscard]] inline mir4d::Result<std::pair<Scalar, Scalar>> linearRegression(
    const std::vector<std::pair<Scalar, Scalar>>& points)
{
    if (points.size() < 2)
        return fail(mir4d::ErrorCode::InvalidArgument, "Нужно не менее двух точек");

    std::vector<std::vector<Scalar>> A(points.size(), std::vector<Scalar>(2));
    std::vector<Scalar> b(points.size());

    for (std::size_t i = 0; i < points.size(); ++i)
    {
        A[i][0] = Scalar(1);   // свободный член (intercept)
        A[i][1] = points[i].first;
        b[i] = points[i].second;
    }

    auto sol = solveLeastSquares(A, b);
    if (!sol)
        return std::unexpected(sol.error());

    return mir4d::success(std::make_pair(sol.value()[1], sol.value()[0]));
}

// ═══════════════════════════════════════════════════════════════
//  Специальные функции
// ═══════════════════════════════════════════════════════════════

/// Натуральный логарифм гамма-функции (аппроксимация Ланцоша, g=7).
[[nodiscard]] inline Scalar logGamma(Scalar x)
{
    // Коэффициенты Ланцоша для g = 7, n = 9.
    static constexpr double c[9] = {
        0.99999999999980993,
        676.5203681218851,
        -1259.1392167224028,
        771.32342877765313,
        -176.61502916214059,
        12.507343278686905,
        -0.13857109526572012,
        9.9843695780195716e-6,
        1.5056327351493116e-7};

    if (x < Scalar(0.5))
    {
        // Формула дополнения Эйлера.
        const Scalar pi = Scalar(3.14159265358979323846);
        return std::log(pi / std::sin(pi * x)) - logGamma(Scalar(1) - x);
    }

    x -= Scalar(1);
    double ag = c[0];
    for (int k = 1; k < 9; ++k)
        ag += c[k] / static_cast<double>(x + k);

    const double t = static_cast<double>(x) + Scalar(7) + Scalar(0.5);
    const double log2pi = Scalar(0.91893853320467274178); // 0.5*ln(2π)

    return static_cast<Scalar>(
        log2pi + (static_cast<double>(x) + Scalar(0.5)) * std::log(t) - t + std::log(ag));
}

/// Гамма-функция Γ(x) (для x > 0).
[[nodiscard]] inline Scalar gammaFunction(Scalar x)
{
    return std::exp(logGamma(x));
}

/// Факториал n! (возвращает Scalar; при переполнении → ∞).
[[nodiscard]] inline Scalar factorial(int n)
{
    if (n < 0)
        return std::numeric_limits<Scalar>::quiet_NaN();

    Scalar result = Scalar(1);
    for (int i = 2; i <= n; ++i)
    {
        result *= static_cast<Scalar>(i);
        if (!std::isfinite(result))
            break;
    }
    return result;
}

/// Биномиальный коэффициент C(n, k) = n! / (k!(n−k)!).
[[nodiscard]] inline Scalar binomialCoefficient(int n, int k)
{
    if (n < 0 || k < 0 || k > n)
        return Scalar(0);
    if (k == 0 || k == n)
        return Scalar(1);

    return std::exp(
        logGamma(static_cast<Scalar>(n + 1)) -
        logGamma(static_cast<Scalar>(k + 1)) -
        logGamma(static_cast<Scalar>(n - k + 1)));
}

/// Функция ошибок erf(x) (аппроксимация Абрамовица–Стигена, точность ~1.5e-7).
[[nodiscard]] inline Scalar erf(Scalar x)
{
    if (x < Scalar(0))
        return -erf(-x);
    if (x == Scalar(0))
        return Scalar(0);

    const Scalar p = Scalar(0.3275911);
    const Scalar t = Scalar(1) / (Scalar(1) + p * x);

    const Scalar a1 = Scalar(0.254829592);
    const Scalar a2 = Scalar(-0.284496736);
    const Scalar a3 = Scalar(1.421413741);
    const Scalar a4 = Scalar(-1.453152027);
    const Scalar a5 = Scalar(1.061405429);

    const Scalar poly = (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t;
    return Scalar(1) - poly * std::exp(-x * x);
}

/// Дополнение функции ошибок erfc(x) = 1 − erf(x).
[[nodiscard]] inline Scalar erfc(Scalar x)
{
    return Scalar(1) - erf(x);
}

// ═══════════════════════════════════════════════════════════════
//  Неполные специальные функции (для распределений вероятностей)
// ═══════════════════════════════════════════════════════════════

/// Регуляризованная нижняя неполная гамма P(a, x) = γ(a,x)/Γ(a).
/// Ряд для x < a+1, непрерывная дробь (Ленца) для x ≥ a+1.
[[nodiscard]] inline Scalar lowerRegularizedGamma(Scalar a, Scalar x)
{
    if (x <= Scalar(0))
        return Scalar(0);
    if (a <= Scalar(0))
        return Scalar(1);

    const Scalar gln = logGamma(a);

    if (x < a + Scalar(1))
    {
        Scalar ap = a;
        Scalar sum = Scalar(1) / a;
        Scalar del = sum;
        for (int n = 0; n < 200; ++n)
        {
            ap += Scalar(1);
            del *= x / ap;
            sum += del;
            if (std::abs(del) < std::abs(sum) * Scalar(1e-15))
                break;
        }
        return sum * std::exp(-x + a * std::log(x) - gln);
    }

    // Непрерывная дробь для Q(a,x) = 1 - P(a,x).
    const Scalar tiny = Scalar(1e-300);
    Scalar b = x + Scalar(1) - a;
    Scalar c = Scalar(1) / tiny;
    Scalar d = Scalar(1) / b;
    Scalar h = d;
    for (int i = 1; i < 200; ++i)
    {
        const Scalar an = -static_cast<Scalar>(i) * (static_cast<Scalar>(i) - a);
        b += Scalar(2);
        d = Scalar(1) / (b + an * d);
        c = b + an / c;
        const Scalar del = c * d;
        h *= del;
        if (std::abs(del - Scalar(1)) < Scalar(1e-15))
            break;
    }
    const Scalar q = std::exp(-x + a * std::log(x) - gln) * h;
    return Scalar(1) - q;
}

/// Регуляризованная неполная бета I_x(a, b) = B(x;a,b)/B(a,b).
/// Через непрерывную дробь (Ленца) с ветвлением по x.
[[nodiscard]] inline Scalar regularizedIncompleteBeta(Scalar x, Scalar a, Scalar b)
{
    if (x <= Scalar(0))
        return Scalar(0);
    if (x >= Scalar(1))
        return Scalar(1);
    if (a <= Scalar(0) || b <= Scalar(0))
        return Scalar(0);

    const Scalar lbeta = logGamma(a + b) - logGamma(a) - logGamma(b);

    if (x < (a + Scalar(1)) / (a + b + Scalar(2)))
    {
        Scalar bt = std::exp(a * std::log(x) + b * std::log(Scalar(1) - x) + lbeta) / a;

        const Scalar tiny = Scalar(1e-300);
        Scalar c = Scalar(1) / tiny;
        Scalar d = Scalar(1) - (a + b) * x / (a + Scalar(1));
        if (std::abs(d) < tiny)
            d = tiny;
        d = Scalar(1) / d;
        Scalar h = d;
        for (int m = 1; m < 200; ++m)
        {
            const Scalar m2 = static_cast<Scalar>(m);
            Scalar aa = m2 * (b - m2) * x / ((a + m2) * (a + m2 - Scalar(1)));
            d = Scalar(1) + aa * d;
            if (std::abs(d) < tiny)
                d = tiny;
            c = Scalar(1) + aa / c;
            if (std::abs(c) < tiny)
                c = tiny;
            d = Scalar(1) / d;
            const Scalar del = d * c;
            h *= del;
            if (std::abs(del - Scalar(1)) < Scalar(1e-15))
                break;

            const Scalar a2 = -(a + m2) * (a + b + m2) * x /
                ((a + Scalar(2) * m2) * (a + Scalar(2) * m2 + Scalar(1)));
            d = Scalar(1) + a2 * d;
            if (std::abs(d) < tiny)
                d = tiny;
            c = Scalar(1) + a2 / c;
            if (std::abs(c) < tiny)
                c = tiny;
            d = Scalar(1) / d;
            const Scalar del2 = d * c;
            h *= del2;
            if (std::abs(del2 - Scalar(1)) < Scalar(1e-15))
                break;
        }
        return bt * h;
    }

    // Ветвь x >= порога: 1 - I_{1-x}(b, a).
    Scalar bt = std::exp(b * std::log(x) + a * std::log(Scalar(1) - x) + lbeta) / b;

    const Scalar tiny = Scalar(1e-300);
    Scalar c = Scalar(1) / tiny;
    Scalar d = Scalar(1) - (a + b) * (Scalar(1) - x) / (b + Scalar(1));
    if (std::abs(d) < tiny)
        d = tiny;
    d = Scalar(1) / d;
    Scalar h = d;
    for (int m = 1; m < 200; ++m)
    {
        const Scalar m2 = static_cast<Scalar>(m);
        Scalar aa = m2 * (a - m2) * (Scalar(1) - x) / ((b + m2) * (b + m2 - Scalar(1)));
        d = Scalar(1) + aa * d;
        if (std::abs(d) < tiny)
            d = tiny;
        c = Scalar(1) + aa / c;
        if (std::abs(c) < tiny)
            c = tiny;
        d = Scalar(1) / d;
        const Scalar del = d * c;
        h *= del;
        if (std::abs(del - Scalar(1)) < Scalar(1e-15))
            break;

        const Scalar a2 = -(b + m2) * (a + b + m2) * (Scalar(1) - x) /
            ((b + Scalar(2) * m2) * (b + Scalar(2) * m2 + Scalar(1)));
        d = Scalar(1) + a2 * d;
        if (std::abs(d) < tiny)
            d = tiny;
        c = Scalar(1) + a2 / c;
        if (std::abs(c) < tiny)
            c = tiny;
        d = Scalar(1) / d;
        const Scalar del2 = d * c;
        h *= del2;
        if (std::abs(del2 - Scalar(1)) < Scalar(1e-15))
            break;
    }
    return Scalar(1) - bt * h;
}

} // namespace mir::math
