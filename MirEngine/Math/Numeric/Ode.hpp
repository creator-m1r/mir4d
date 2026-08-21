
#pragma once

#include "LinearSystem.hpp"

#include <cstddef>
#include <functional>
#include <vector>

namespace mir::math
{

using OdeRhs = std::function<VectorN(Scalar, const VectorN&)>;

[[nodiscard]] inline VectorN axpy(Scalar a, const VectorN& x, const VectorN& y)
{
    VectorN r(x.size(), Scalar(0));
    for (std::size_t i = 0; i < x.size(); ++i)
        r[i] = a * x[i] + y[i];
    return r;
}

[[nodiscard]] inline VectorN odeEulerStep(const OdeRhs& f, Scalar t, const VectorN& y, Scalar h)
{
    return axpy(h, f(t, y), y);
}

[[nodiscard]] inline VectorN odeMidpointStep(const OdeRhs& f, Scalar t, const VectorN& y, Scalar h)
{
    const VectorN k1 = f(t, y);
    const VectorN ym = axpy(h / Scalar(2), k1, y);
    return axpy(h, f(t + h / Scalar(2), ym), y);
}

[[nodiscard]] inline VectorN odeRK4Step(const OdeRhs& f, Scalar t, const VectorN& y, Scalar h)
{
    const VectorN k1 = f(t, y);
    const VectorN k2 = f(t + h / Scalar(2), axpy(h / Scalar(2), k1, y));
    const VectorN k3 = f(t + h / Scalar(2), axpy(h / Scalar(2), k2, y));
    const VectorN k4 = f(t + h, axpy(h, k3, y));

    VectorN out(y.size(), Scalar(0));
    for (std::size_t i = 0; i < y.size(); ++i)
        out[i] = y[i] + (h / Scalar(6)) * (k1[i] + Scalar(2) * k2[i] + Scalar(2) * k3[i] + k4[i]);
    return out;
}

inline VectorN integrateRK4(
    const OdeRhs& f,
    Scalar t0,
    const VectorN& y0,
    Scalar tEnd,
    std::size_t steps,
    std::vector<VectorN>* trajectory = nullptr,
    std::vector<Scalar>* times = nullptr)
{
    const Scalar h = (tEnd - t0) / static_cast<Scalar>(steps);
    VectorN y = y0;
    Scalar t = t0;
    if (trajectory)
    {
        trajectory->clear();
        trajectory->push_back(y);
    }
    if (times)
    {
        times->clear();
        times->push_back(t);
    }
    for (std::size_t s = 0; s < steps; ++s)
    {
        y = odeRK4Step(f, t, y, h);
        t += h;
        if (trajectory)
            trajectory->push_back(y);
        if (times)
            times->push_back(t);
    }
    return y;
}

inline VectorN rkf45Step(
    const OdeRhs& f,
    Scalar t,
    const VectorN& y,
    Scalar h,
    Scalar* errorEstimate = nullptr)
{
    const VectorN k1 = f(t, y);
    const VectorN k2 = f(t + h * Scalar(0.25), axpy(h * Scalar(0.25), k1, y));
    const VectorN k3 = f(t + h * Scalar(3) / Scalar(8),
        axpy(h * Scalar(3) / Scalar(32), k1, axpy(h * Scalar(9) / Scalar(32), k2, y)));
    const VectorN k4 = f(t + h * Scalar(12) / Scalar(13),
        axpy(h * Scalar(1932) / Scalar(2197), k1,
            axpy(-h * Scalar(7200) / Scalar(2197), k2, axpy(h * Scalar(7296) / Scalar(2197), k3, y))));
    const VectorN k5 = f(t + h,
        axpy(h * Scalar(439) / Scalar(216), k1,
            axpy(-h * Scalar(8), k2, axpy(h * Scalar(3680) / Scalar(513), k3, axpy(-h * Scalar(845) / Scalar(4104), k4, y)))));
    const VectorN k6 = f(t + h * Scalar(0.5),
        axpy(-h * Scalar(8) / Scalar(27), k1,
            axpy(h * Scalar(2), k2,
                axpy(-h * Scalar(3544) / Scalar(2565), k3,
                    axpy(h * Scalar(1859) / Scalar(4104), k4, axpy(-h * Scalar(11) / Scalar(40), k5, y))))));

    VectorN yNew(y.size(), Scalar(0));
    VectorN y4(y.size(), Scalar(0));
    Scalar err = Scalar(0);
    for (std::size_t i = 0; i < y.size(); ++i)
    {
        yNew[i] = y[i] + h * (Scalar(16) / Scalar(135) * k1[i]
            + Scalar(6656) / Scalar(12825) * k3[i]
            + Scalar(28561) / Scalar(56430) * k4[i]
            - Scalar(9) / Scalar(50) * k5[i]
            + Scalar(2) / Scalar(55) * k6[i]);
        y4[i] = y[i] + h * (Scalar(25) / Scalar(216) * k1[i]
            + Scalar(1408) / Scalar(2565) * k3[i]
            + Scalar(2197) / Scalar(4104) * k4[i]
            - Scalar(1) / Scalar(5) * k5[i]);
        err = std::max(err, std::abs(yNew[i] - y4[i]));
    }
    if (errorEstimate)
        *errorEstimate = err;
    return yNew;
}

inline VectorN integrateAdaptiveRKF45(
    const OdeRhs& f,
    Scalar t0,
    const VectorN& y0,
    Scalar tEnd,
    Scalar tol = Scalar(1e-6),
    std::size_t maxSteps = 100000,
    std::vector<VectorN>* trajectory = nullptr,
    std::vector<Scalar>* times = nullptr)
{
    Scalar h = (tEnd - t0) / static_cast<Scalar>(100);
    if (h == Scalar(0))
        return y0;
    Scalar t = t0;
    VectorN y = y0;
    const Scalar safety = Scalar(0.9);
    const Scalar minScale = Scalar(0.2);
    const Scalar maxScale = Scalar(5.0);

    if (trajectory)
    {
        trajectory->clear();
        trajectory->push_back(y);
    }
    if (times)
    {
        times->clear();
        times->push_back(t);
    }

    for (std::size_t step = 0; step < maxSteps && t < tEnd; ++step)
    {
        if (t + h > tEnd)
            h = tEnd - t;

        Scalar err = Scalar(0);
        VectorN yNew = rkf45Step(f, t, y, h, &err);

        const Scalar scale = (err > Scalar(0))
            ? std::max(minScale, std::min(maxScale, safety * std::pow(tol / err, Scalar(1) / Scalar(5))))
            : maxScale;

        if (err <= tol || h <= (tEnd - t) * Scalar(1e-12))
        {
            y = yNew;
            t += h;
            if (trajectory)
                trajectory->push_back(y);
            if (times)
                times->push_back(t);
            if (t >= tEnd)
                break;
        }
        h *= scale;
    }
    return y;
}

}
