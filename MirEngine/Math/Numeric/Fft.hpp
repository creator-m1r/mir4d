
#pragma once

#include <complex>
#include <cmath>
#include <numbers>
#include <vector>

namespace mir::math
{

using FftComplex = std::complex<Scalar>;

inline void fftInPlace(std::vector<FftComplex>& a, bool inverse = false)
{
    const std::size_t n = a.size();
    if (n <= 1)
        return;

    for (std::size_t i = 1, j = 0; i < n; ++i)
    {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(a[i], a[j]);
    }

    for (std::size_t len = 2; len <= n; len <<= 1)
    {
        const Scalar ang = (inverse ? Scalar(2) : -Scalar(2)) *
            std::numbers::pi_v<Scalar> / static_cast<Scalar>(len);
        const FftComplex wlen(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len)
        {
            FftComplex w(1, 0);
            for (std::size_t k = 0; k < len / 2; ++k)
            {
                const FftComplex u = a[i + k];
                const FftComplex v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (inverse)
    {
        const Scalar inv = Scalar(1) / static_cast<Scalar>(n);
        for (auto& x : a)
            x *= inv;
    }
}

[[nodiscard]] inline std::vector<FftComplex> fft(const std::vector<FftComplex>& in, bool inverse = false)
{
    std::vector<FftComplex> a = in;
    fftInPlace(a, inverse);
    return a;
}

[[nodiscard]] inline std::vector<FftComplex> fftReal(const std::vector<Scalar>& in, bool inverse = false)
{
    std::vector<FftComplex> a(in.size());
    for (std::size_t i = 0; i < in.size(); ++i)
        a[i] = FftComplex(in[i], Scalar(0));
    fftInPlace(a, inverse);
    return a;
}

[[nodiscard]] inline std::vector<Scalar> convolveFFT(const std::vector<Scalar>& x, const std::vector<Scalar>& y)
{
    const std::size_t n = x.size();
    std::vector<FftComplex> X = fftReal(x);
    std::vector<FftComplex> Y = fftReal(y);
    std::vector<FftComplex> Z(n);
    for (std::size_t i = 0; i < n; ++i)
        Z[i] = X[i] * Y[i];
    fftInPlace(Z, true);
    std::vector<Scalar> out(n, Scalar(0));
    for (std::size_t i = 0; i < n; ++i)
        out[i] = Z[i].real();
    return out;
}

}
