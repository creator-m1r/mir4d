// MirEngine/Math/Numeric/NumericalAnalysis.hpp
// 🧮 Численный анализ — основа инженерных расчётов MIR 4D.
//
// Содержит классические численные методы, не привязанные к конкретной
// геометрии:
//   • Поиск корней:   биссекция, Ньютон, метод секущих.
//   • Интегрирование:  метод трапеций, метод Симпсона.
//   • Дифференцирование: центральная/правосторонняя/левосторонняя разности.
//   • Одномерная минимизация: поиск золотым сечением.
//
// Все методы принимают целевую функцию как std::function<Scalar(Scalar)>,
// что позволяет передавать лямбды, свободные функции и функторы.
//
// Итерационные методы возвращают mir4d::Result: при отсутствии сходимости
// или некорректных входных данных возвращается описание ошибки.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Result.hpp"
#include "../../Core/Types/Scalar.hpp"

#include <cmath>
#include <functional>
#include <utility>

namespace mir::math
{

#ifndef MIR_MATH_NUMERIC_FAIL_DEFINED
#define MIR_MATH_NUMERIC_FAIL_DEFINED
/// Возвращает «неудачу» в виде std::unexpected<Error> для функций Result.
[[nodiscard]] inline auto fail(mir4d::ErrorCode code, std::string_view message)
{
    return std::unexpected(mir4d::Error(code, message));
}
#endif

// ═══════════════════════════════════════════════════════════════
//  Поиск корней уравнения f(x) = 0
// ═══════════════════════════════════════════════════════════════

/// Находит корень f(x) = 0 на отрезке [a, b] методом бисекции.
/// Требует, чтобы f(a) и f(b) имели разные знаки (корень «зажат»).
///
/// \param f        непрерывная целевая функция.
/// \param a, b     границы интервала поиска.
/// \param tol      допуск по значению/ширине интервала.
/// \param maxIter  максимальное число итераций.
[[nodiscard]] inline mir4d::Result<Scalar> findRootBisection(
    std::function<Scalar(Scalar)> f,
    Scalar a,
    Scalar b,
    Scalar tol = Scalar(1e-9),
    int maxIter = 100)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    Scalar fa = f(a);
    Scalar fb = f(b);

    if (fa == Scalar(0))
        return mir4d::success(a);
    if (fb == Scalar(0))
        return mir4d::success(b);

    if (fa * fb > Scalar(0))
        return fail(mir4d::ErrorCode::ValidationFailed,
            "Корень не зажат: f(a) и f(b) одного знака");

    Scalar lo = a;
    Scalar hi = b;

    for (int i = 0; i < maxIter; ++i)
    {
        const Scalar mid = (lo + hi) * Scalar(0.5);
        const Scalar fm = f(mid);

        if (std::abs(fm) <= tol || (hi - lo) * Scalar(0.5) <= tol)
            return mir4d::success(mid);

        if (fa * fm <= Scalar(0))
        {
            hi = mid;
            fb = fm;
        }
        else
        {
            lo = mid;
            fa = fm;
        }
    }

    return fail(mir4d::ErrorCode::Internal, "Бисекция не сошлась за maxIter");
}

/// Находит корень f(x) = 0 методом Ньютона (касательных).
/// Требует аналитическую производную df.
///
/// \param f, df    функция и её производная.
/// \param x0       начальное приближение.
[[nodiscard]] inline mir4d::Result<Scalar> findRootNewton(
    std::function<Scalar(Scalar)> f,
    std::function<Scalar(Scalar)> df,
    Scalar x0,
    Scalar tol = Scalar(1e-9),
    int maxIter = 100)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    Scalar x = x0;

    for (int i = 0; i < maxIter; ++i)
    {
        const Scalar fx = f(x);
        if (std::abs(fx) <= tol)
            return mir4d::success(x);

        const Scalar d = df(x);
        if (std::abs(d) <= Scalar(1e-14))
            return fail(mir4d::ErrorCode::ValidationFailed,
                "Производная близка к нулю (сингулярность)");

        const Scalar next = x - fx / d;
        if (std::abs(next - x) <= tol)
            return mir4d::success(next);

        x = next;
    }

    return fail(mir4d::ErrorCode::Internal, "Метод Ньютона не сошёлся за maxIter");
}

/// Находит корень f(x) = 0 методом секущих (без производной).
///
/// \param x0, x1   две начальные точки.
[[nodiscard]] inline mir4d::Result<Scalar> findRootSecant(
    std::function<Scalar(Scalar)> f,
    Scalar x0,
    Scalar x1,
    Scalar tol = Scalar(1e-9),
    int maxIter = 100)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    Scalar xPrev = x0;
    Scalar fPrev = f(x0);

    if (std::abs(fPrev) <= tol)
        return mir4d::success(x0);

    Scalar x = x1;
    Scalar fx = f(x1);

    for (int i = 0; i < maxIter; ++i)
    {
        if (std::abs(fx) <= tol)
            return mir4d::success(x);

        const Scalar denom = fx - fPrev;
        if (std::abs(denom) <= Scalar(1e-14))
            return fail(mir4d::ErrorCode::ValidationFailed,
                "Знаменатель метода секущих близок к нулю");

        const Scalar next = x - fx * (x - xPrev) / denom;
        if (std::abs(next - x) <= tol)
            return mir4d::success(next);

        xPrev = x;
        fPrev = fx;
        x = next;
        fx = f(x);
    }

    return fail(mir4d::ErrorCode::Internal, "Метод секущих не сошёлся за maxIter");
}

// ═══════════════════════════════════════════════════════════════
//  Численное интегрирование
// ═══════════════════════════════════════════════════════════════

/// Определяет ∫ₐᵇ f(x) dx методом трапеций на n отрезках.
[[nodiscard]] inline Scalar integrateTrapezoidal(
    std::function<Scalar(Scalar)> f,
    Scalar a,
    Scalar b,
    int n)
{
    if (n <= 0)
        return Scalar(0);

    const Scalar h = (b - a) / static_cast<Scalar>(n);
    Scalar sum = (f(a) + f(b)) * Scalar(0.5);

    for (int i = 1; i < n; ++i)
        sum += f(a + static_cast<Scalar>(i) * h);

    return sum * h;
}

/// Определяет ∫ₐᵇ f(x) dx методом Симпсона (требует чётное n,
/// при нечётном n автоматически увеличивается).
[[nodiscard]] inline Scalar integrateSimpson(
    std::function<Scalar(Scalar)> f,
    Scalar a,
    Scalar b,
    int n)
{
    if (n <= 0)
        return Scalar(0);

    if (n % 2 != 0)
        ++n;

    const Scalar h = (b - a) / static_cast<Scalar>(n);
    Scalar sum = f(a) + f(b);

    for (int i = 1; i < n; ++i)
    {
        const Scalar x = a + static_cast<Scalar>(i) * h;
        sum += f(x) * (i % 2 == 0 ? Scalar(2) : Scalar(4));
    }

    return sum * h / Scalar(3);
}

// ═══════════════════════════════════════════════════════════════
//  Численное дифференцирование
// ═══════════════════════════════════════════════════════════════

/// Производная f'(x) центральной разностью: (f(x+h) − f(x−h)) / 2h.
[[nodiscard]] inline Scalar derivativeCentral(
    std::function<Scalar(Scalar)> f,
    Scalar x,
    Scalar h = Scalar(1e-5))
{
    return (f(x + h) - f(x - h)) / (Scalar(2) * h);
}

/// Производная f'(x) правосторонней (прогрессивной) разностью.
[[nodiscard]] inline Scalar derivativeForward(
    std::function<Scalar(Scalar)> f,
    Scalar x,
    Scalar h = Scalar(1e-5))
{
    return (f(x + h) - f(x)) / h;
}

/// Производная f'(x) левосторонней (регрессивной) разностью.
[[nodiscard]] inline Scalar derivativeBackward(
    std::function<Scalar(Scalar)> f,
    Scalar x,
    Scalar h = Scalar(1e-5))
{
    return (f(x) - f(x - h)) / h;
}

// ═══════════════════════════════════════════════════════════════
//  Одномерная минимизация
// ═══════════════════════════════════════════════════════════════

/// Находит точку минимума унимодальной функции f на [a, b]
/// поиском золотым сечением.
///
/// \return приближённая координата минимума или ошибку при несходимости.
[[nodiscard]] inline mir4d::Result<Scalar> minimizeGoldenSection(
    std::function<Scalar(Scalar)> f,
    Scalar a,
    Scalar b,
    Scalar tol = Scalar(1e-7),
    int maxIter = 100)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    if (b < a)
        std::swap(a, b);

    // resphi = 2 - phi ≈ 0.381966, где phi = (1+√5)/2.
    const Scalar resphi = Scalar(2) - (Scalar(1) + std::sqrt(Scalar(5))) / Scalar(2);

    Scalar x1 = a + resphi * (b - a);
    Scalar x2 = b - resphi * (b - a);
    Scalar f1 = f(x1);
    Scalar f2 = f(x2);

    for (int i = 0; i < maxIter; ++i)
    {
        if (std::abs(b - a) <= tol)
            return mir4d::success((a + b) * Scalar(0.5));

        if (f1 < f2)
        {
            b = x2;
            x2 = x1;
            f2 = f1;
            x1 = a + resphi * (b - a);
            f1 = f(x1);
        }
        else
        {
            a = x1;
            x1 = x2;
            f1 = f2;
            x2 = b - resphi * (b - a);
            f2 = f(x2);
        }
    }

    return fail(mir4d::ErrorCode::Internal,
        "Поиск золотым сечением не сошёлся за maxIter");
}

} // namespace mir::math
