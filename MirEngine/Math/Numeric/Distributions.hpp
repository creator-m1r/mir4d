
#pragma once

#include "../../Core/Types/Scalar.hpp"
#include "Statistics.hpp"

#include <cmath>
#include <utility>
#include <vector>

namespace mir::math
{

static constexpr Scalar kPi = Scalar(3.14159265358979323846);

[[nodiscard]] inline Scalar normalPdf(Scalar x, Scalar mu = Scalar(0), Scalar sigma = Scalar(1))
{
    const Scalar z = (x - mu) / sigma;
    return std::exp(Scalar(-0.5) * z * z) / (sigma * std::sqrt(Scalar(2) * kPi));
}

[[nodiscard]] inline Scalar normalCdf(Scalar x, Scalar mu = Scalar(0), Scalar sigma = Scalar(1))
{
    return Scalar(0.5) * (Scalar(1) + erf((x - mu) / (sigma * std::sqrt(Scalar(2)))));
}

[[nodiscard]] inline Scalar exponentialPdf(Scalar x, Scalar lambda = Scalar(1))
{
    if (x < Scalar(0))
        return Scalar(0);
    return lambda * std::exp(-lambda * x);
}

[[nodiscard]] inline Scalar exponentialCdf(Scalar x, Scalar lambda = Scalar(1))
{
    if (x < Scalar(0))
        return Scalar(0);
    return Scalar(1) - std::exp(-lambda * x);
}

[[nodiscard]] inline Scalar logNormalPdf(Scalar x, Scalar mu = Scalar(0), Scalar sigma = Scalar(1))
{
    if (x <= Scalar(0))
        return Scalar(0);
    const Scalar z = (std::log(x) - mu) / sigma;
    return std::exp(Scalar(-0.5) * z * z) / (x * sigma * std::sqrt(Scalar(2) * kPi));
}

[[nodiscard]] inline Scalar logNormalCdf(Scalar x, Scalar mu = Scalar(0), Scalar sigma = Scalar(1))
{
    if (x <= Scalar(0))
        return Scalar(0);
    return normalCdf(std::log(x), mu, sigma);
}

[[nodiscard]] inline Scalar chiSquarePdf(Scalar x, Scalar k)
{
    if (x <= Scalar(0))
        return Scalar(0);
    const Scalar a = k / Scalar(2);
    return std::pow(x, a - Scalar(1)) * std::exp(-x / Scalar(2)) /
        (std::pow(Scalar(2), a) * gammaFunction(a));
}

[[nodiscard]] inline Scalar chiSquareCdf(Scalar x, Scalar k)
{
    if (x <= Scalar(0))
        return Scalar(0);
    return lowerRegularizedGamma(k / Scalar(2), x / Scalar(2));
}

[[nodiscard]] inline Scalar studentTPdf(Scalar t, Scalar nu)
{
    const Scalar a = (nu + Scalar(1)) / Scalar(2);
    const Scalar b = nu / Scalar(2);
    const Scalar coeff = std::exp(logGamma(a) - logGamma(b)) / std::sqrt(nu * kPi);
    return coeff * std::pow(Scalar(1) + t * t / nu, -a);
}

[[nodiscard]] inline Scalar studentTCdf(Scalar t, Scalar nu)
{
    if (t == Scalar(0))
        return Scalar(0.5);

    const Scalar x = nu / (nu + t * t);
    const Scalar ib = regularizedIncompleteBeta(x, nu / Scalar(2), Scalar(0.5));
    return (t > Scalar(0)) ? (Scalar(1) - Scalar(0.5) * ib) : (Scalar(0.5) * ib);
}

}
