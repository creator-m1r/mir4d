// MirEngine/Tests/Math/NumericTest.cpp
// 🧪 Постоянный набор проверок математического ядра MIR 4D (模块 Numeric).
//
// Запускается через ctest как MIR4D_MathNumeric. Возвращает 0 при успехе,
// abort() при расхождении с эталоном. Охватывает все подмодули Numeric:
// линейные системы, численный анализ, интерполяцию, статистику,
// ГПСЧ, собственные значения, оптимизацию, разложения и производные.

#include "MirEngine/Math/Numeric/Numeric.hpp"

#include <cassert>
#include <cmath>
#include <complex>
#include <cstdio>
#include <vector>

using namespace mir;
using namespace mir::math;

#define CHECK_NEAR(a, b, tol) \
    do { \
        if (std::abs((a) - (b)) > (tol)) { \
            std::fprintf(stderr, "FAIL: %s (%g vs %g)\n", #a, double(a), double(b)); \
            std::abort(); \
        } \
    } while (false)

int main()
{
    // ── LinearSystem ─────────────────────────────────────────────
    {
        std::vector<std::vector<Scalar>> A{{2, 1}, {1, 3}};
        std::vector<Scalar> b{5, 10};
        auto r = solveLinearSystem(A, b);
        assert(r.has_value());
        CHECK_NEAR(r.value()[0], 1.0, 1e-9);
        CHECK_NEAR(r.value()[1], 3.0, 1e-9);

        std::vector<std::vector<Scalar>> A2{{1, 0}, {1, 1}, {1, 2}, {1, 3}};
        std::vector<Scalar> b2{1, 3, 5, 7}; // y = 2x + 1
        auto ls = solveLeastSquares(A2, b2);
        assert(ls.has_value());
        CHECK_NEAR(ls.value()[1], 2.0, 1e-6);  // slope
        CHECK_NEAR(ls.value()[0], 1.0, 1e-6);  // intercept

        std::vector<std::vector<Scalar>> As{{1, 1}, {2, 2}};
        std::vector<Scalar> bs{1, 2};
        auto sing = solveLinearSystem(As, bs);
        assert(!sing.has_value());
    }

    // ── NumericalAnalysis ───────────────────────────────────────
    {
        auto rb = findRootBisection([](Scalar x) { return std::cos(x); }, Scalar(0), Scalar(2));
        assert(rb.has_value());
        CHECK_NEAR(rb.value(), 1.5707963267948966, 1e-7);

        auto rn = findRootNewton([](Scalar x) { return x * x - 2; },
                                 [](Scalar x) { return 2 * x; }, Scalar(1));
        assert(rn.has_value());
        CHECK_NEAR(rn.value(), std::sqrt(Scalar(2)), 1e-9);

        Scalar simp = integrateSimpson([](Scalar x) { return x * x; }, Scalar(0), Scalar(1), 100);
        CHECK_NEAR(simp, 1.0 / 3.0, 1e-6);

        CHECK_NEAR(derivativeCentral([](Scalar x) { return std::sin(x); }, Scalar(0)), 1.0, 1e-4);

        auto rm = minimizeGoldenSection([](Scalar x) { return x * x - 3 * x + 2; },
                                        Scalar(-5), Scalar(5));
        assert(rm.has_value());
        CHECK_NEAR(rm.value(), 1.5, 1e-5);
    }

    // ── Interpolation ───────────────────────────────────────────
    {
        std::vector<Scalar> P{1, 3, 2}; // 2x^2 + 3x + 1
        CHECK_NEAR(evaluatePolynomial(P, 2), 15.0, 1e-9);
        auto dP = derivativePolynomial(P);
        CHECK_NEAR(evaluatePolynomial(dP, 2), 11.0, 1e-9); // 4x + 3

        std::vector<std::pair<Scalar, Scalar>> pts{{0, 1}, {1, 3}, {2, 7}}; // x^2 + x + 1
        auto lg = lagrangeInterpolate(pts, 1.5);
        assert(lg.has_value());
        CHECK_NEAR(lg.value(), 4.75, 1e-9);

        Vector3 bez = cubicBezier({0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, 0.5);
        CHECK_NEAR(bez.x, 0.5, 1e-9);
        CHECK_NEAR(bez.y, 0.75, 1e-9);
        CHECK_NEAR(bez.z, 0.0, 1e-9);
    }

    // ── Statistics & special functions ─────────────────────────
    {
        std::vector<Scalar> v{1, 2, 3, 4, 5};
        CHECK_NEAR(mean(v).value(), 3.0, 1e-9);
        CHECK_NEAR(variance(v, true).value(), 2.5, 1e-9);
        CHECK_NEAR(median(v).value(), 3.0, 1e-9);

        std::vector<std::pair<Scalar, Scalar>> rp{{0, 1}, {1, 3}, {2, 5}, {3, 7}};
        auto reg = linearRegression(rp);
        assert(reg.has_value());
        CHECK_NEAR(reg.value().first, 2.0, 1e-6);   // slope
        CHECK_NEAR(reg.value().second, 1.0, 1e-6);  // intercept

        CHECK_NEAR(pearson({1, 2, 3}, {2, 4, 6}).value(), 1.0, 1e-9);
        CHECK_NEAR(gammaFunction(5), 24.0, 1e-9);
        CHECK_NEAR(gammaFunction(0.5), std::sqrt(Scalar(3.14159265358979323846)), 1e-9);
        CHECK_NEAR(factorial(10), 3628800.0, 1e-6);
        CHECK_NEAR(binomialCoefficient(10, 3), 120.0, 1e-6);
        CHECK_NEAR(erf(1), 0.8427007929497149, 1e-6);
    }

    // ── Random ─────────────────────────────────────────────────
    {
        Random r1(12345), r2(12345);
        assert(r1.nextU64() == r2.nextU64());
        Random r3(7);
        double acc = 0;
        for (int i = 0; i < 200000; ++i)
            acc += r3.normal(0, 1);
        CHECK_NEAR(acc / 200000.0, 0.0, 0.05);
    }

    // ── Eigen ──────────────────────────────────────────────────
    {
        std::vector<std::vector<Scalar>> A{{2, 1}, {1, 2}};
        auto e = symmetricEigen(A);
        assert(e.has_value());
        CHECK_NEAR(e.value().first[0], 3.0, 1e-7);
        CHECK_NEAR(e.value().first[1], 1.0, 1e-7);
        const auto& v0 = e.value().second[0];
        // A v0 = lambda0 v0
        Scalar l0 = e.value().first[0];
        CHECK_NEAR(A[0][0] * v0[0] + A[0][1] * v0[1], l0 * v0[0], 1e-7);
        CHECK_NEAR(A[1][0] * v0[0] + A[1][1] * v0[1], l0 * v0[1], 1e-7);
    }

    // ── Optimization ───────────────────────────────────────────
    {
        FunctionN F = [](const VectorN& x) {
            return VectorN{x[0] * x[0] + x[1] - 1, x[0] + x[1] * x[1] - 1};
        };
        JacobianN J = [](const VectorN& x) {
            return MatrixN{{2 * x[0], 1}, {1, 2 * x[1]}};
        };
        auto s = solveNonlinearSystem(F, J, {0.0, 0.0});
        assert(s.has_value());
        CHECK_NEAR(vectorNorm(F(s.value())), 0.0, 1e-9);

        GradientN g = [](const VectorN& x) { return VectorN{2 * x[0], 4 * x[1]}; };
        HessianN H = [](const VectorN&) { return MatrixN{{2, 0}, {0, 4}}; };
        auto m = minimizeNewton(g, H, {3.0, -1.0});
        assert(m.has_value());
        CHECK_NEAR(m.value()[0], 0.0, 1e-7);
        CHECK_NEAR(m.value()[1], 0.0, 1e-7);

        // BFGS (квазиньютоновский, только градиент) — квадратик и Розенброк.
        ObjectiveN quad = [](const VectorN& v) {
            Scalar a = v[0] - 3, b = v[1] + 2;
            return a * a + b * b;
        };
        GradientN grad = [](const VectorN& v) { return VectorN{2 * (v[0] - 3), 2 * (v[1] + 2)}; };
        auto bf = minimizeBFGS(quad, grad, {0.0, 0.0});
        assert(bf.has_value());
        CHECK_NEAR(bf.value()[0], 3.0, 1e-6);
        CHECK_NEAR(bf.value()[1], -2.0, 1e-6);

        ObjectiveN ros = [](const VectorN& v) {
            Scalar a = 1 - v[0], b = v[1] - v[0] * v[0];
            return a * a + 100 * b * b;
        };
        GradientN rosG = [](const VectorN& v) {
            Scalar x = v[0], y = v[1];
            return VectorN{-2 * (1 - x) - 400 * x * (y - x * x), 200 * (y - x * x)};
        };
        auto br = minimizeBFGS(ros, rosG, {-1.2, 1.0}, 1e-8, 5000);
        assert(br.has_value());
        CHECK_NEAR(br.value()[0], 1.0, 1e-5);
        CHECK_NEAR(br.value()[1], 1.0, 1e-5);

        // Нелинейный МНК (Левенберг–Марквардт): линейная и экспоненциальная аппроксимация.
        std::vector<Scalar> ts{0, 1, 2, 3, 4};
        std::vector<Scalar> ys{1, 3, 5, 7, 9};
        ResidualN rlin = [&](const VectorN& p) {
            VectorN r(5);
            for (std::size_t i = 0; i < 5; ++i)
                r[i] = p[0] + p[1] * ts[i] - ys[i];
            return r;
        };
        JacobianN jlin = [&](const VectorN&) {
            MatrixN J(5, VectorN(2));
            for (std::size_t i = 0; i < 5; ++i)
            {
                J[i][0] = 1;
                J[i][1] = ts[i];
            }
            return J;
        };
        auto lf = solveNonlinearLeastSquares(rlin, jlin, {0.0, 0.0});
        assert(lf.has_value());
        CHECK_NEAR(lf.value()[0], 1.0, 1e-4);
        CHECK_NEAR(lf.value()[1], 2.0, 1e-4);

        std::vector<Scalar> us{0, 0.5, 1, 1.5, 2};
        std::vector<Scalar> ws;
        for (Scalar t : us)
            ws.push_back(2.0 * std::exp(-1.0 * t));
        ResidualN rexp = [&](const VectorN& p) {
            VectorN r(us.size());
            for (std::size_t i = 0; i < us.size(); ++i)
                r[i] = p[0] * std::exp(p[1] * us[i]) - ws[i];
            return r;
        };
        JacobianN jexp = [&](const VectorN& p) {
            MatrixN J(us.size(), VectorN(2));
            for (std::size_t i = 0; i < us.size(); ++i)
            {
                const Scalar e = std::exp(p[1] * us[i]);
                J[i][0] = e;
                J[i][1] = p[0] * us[i] * e;
            }
            return J;
        };
        auto ef = solveNonlinearLeastSquares(rexp, jexp, {1.0, -0.5});
        assert(ef.has_value());
        CHECK_NEAR(ef.value()[0], 2.0, 1e-3);
        CHECK_NEAR(ef.value()[1], -1.0, 1e-3);
    }

    // ── Decomposition ──────────────────────────────────────────
    {
        MatrixN A{{2, 1}, {1, 3}};
        auto lu = luDecompose(A);
        assert(lu.has_value());
        auto x = luSolve(lu.value(), {5, 10});
        assert(x.has_value());
        CHECK_NEAR(x.value()[0], 1.0, 1e-9);
        CHECK_NEAR(x.value()[1], 3.0, 1e-9);

        MatrixN C{{4, 2}, {2, 5}};
        auto L = cholesky(C);
        assert(L.has_value());
        CHECK_NEAR(L.value()[0][0], 2.0, 1e-9);
        CHECK_NEAR(L.value()[1][0], 1.0, 1e-9);
        CHECK_NEAR(L.value()[1][1], 2.0, 1e-9);

        MatrixN M{{12, -51, 4}, {6, 167, -68}, {-4, 24, -41}};
        auto qr = qrDecompose(M);
        assert(qr.has_value());
        // Q^T Q = I
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
            {
                Scalar s = 0;
                for (std::size_t r = 0; r < 3; ++r)
                    s += qr.value().first[r][i] * qr.value().first[r][j];
                CHECK_NEAR(s, (i == j ? 1.0 : 0.0), 1e-9);
            }
    }

    // ── Derivatives ────────────────────────────────────────────
    {
        ScalarFunction f = [](const VectorN& x) {
            return x[0] * x[0] + 3 * x[0] * x[1] + 2 * x[1] * x[1];
        };
        VectorN p{1.0, 2.0};
        auto g = numericalGradient(f, p);
        CHECK_NEAR(g[0], 8.0, 1e-4);
        CHECK_NEAR(g[1], 11.0, 1e-4);
        auto H = numericalHessian(f, p);
        CHECK_NEAR(H[0][0], 2.0, 1e-3);
        CHECK_NEAR(H[0][1], 3.0, 1e-3);
        CHECK_NEAR(H[1][1], 4.0, 1e-3);
    }

    // ── SVD ──────────────────────────────────────────────────────
    {
        auto recons = [](const SVD& s) {
            std::size_t m = s.U.size(), n = s.V.size(), k = s.sigma.size();
            MatrixN R(m, VectorN(n, 0));
            for (std::size_t i = 0; i < m; ++i)
                for (std::size_t j = 0; j < n; ++j)
                {
                    Scalar v = 0;
                    for (std::size_t t = 0; t < k; ++t)
                        v += s.U[i][t] * s.sigma[t] * s.V[j][t];
                    R[i][j] = v;
                }
            return R;
        };
        auto orth = [](const std::vector<std::vector<Scalar>>& M, std::size_t cols) {
            Scalar mx = 0;
            for (std::size_t i = 0; i < cols; ++i)
                for (std::size_t j = 0; j < cols; ++j)
                {
                    Scalar d = 0;
                    for (std::size_t r = 0; r < M.size(); ++r)
                        d += M[r][i] * M[r][j];
                    mx = std::max(mx, std::abs(d - (i == j ? 1.0 : 0.0)));
                }
            return mx;
        };

        MatrixN A1{{1, 1}, {1, -1}};
        auto s1 = svdDecompose(A1);
        assert(s1.has_value());
        CHECK_NEAR(s1.value().sigma[0], 1.41421356, 1e-6);
        CHECK_NEAR(recons(s1.value())[0][0], A1[0][0], 1e-9);

        MatrixN A2{{3, 0}, {0, 4}};
        auto s2 = svdDecompose(A2);
        assert(s2.has_value());
        CHECK_NEAR(s2.value().sigma[0], 4.0, 1e-9);
        CHECK_NEAR(s2.value().sigma[1], 3.0, 1e-9);

        MatrixN A3{{1, 2}, {3, 4}, {5, 6}};
        auto s3 = svdDecompose(A3);
        assert(s3.has_value());
        CHECK_NEAR(s3.value().sigma[0], 9.525518, 1e-4);
        CHECK_NEAR(s3.value().sigma[1], 0.514301, 1e-4);
        CHECK_NEAR(recons(s3.value())[2][1], A3[2][1], 1e-9);
        CHECK_NEAR(orth(s3.value().U, 2), 0.0, 1e-10);
        CHECK_NEAR(orth(s3.value().V, 2), 0.0, 1e-10);

        MatrixN A4{{1, 2, 3}, {4, 5, 6}}; // m < n
        auto s4 = svdDecompose(A4);
        assert(s4.has_value());
        CHECK_NEAR(recons(s4.value())[1][2], A4[1][2], 1e-9);
        CHECK_NEAR(orth(s4.value().U, 2), 0.0, 1e-10);
        CHECK_NEAR(orth(s4.value().V, 2), 0.0, 1e-10);

        // Линейные МНК через SVD: переопределённая и вырожденная системы.
        MatrixN Aover{{1, 1}, {1, 2}, {1, 3}};
        auto xo = solveLeastSquaresSVD(Aover, {2, 3, 5});
        assert(xo.has_value());
        CHECK_NEAR(xo.value()[0], 1.0 / 3.0, 1e-6);
        CHECK_NEAR(xo.value()[1], 1.5, 1e-6);

        MatrixN Arank{{1, 1}, {2, 2}, {3, 3}};
        auto xr = solveLeastSquaresSVD(Arank, {1, 2, 3});
        assert(xr.has_value());
        CHECK_NEAR(xr.value()[0], 0.5, 1e-6);  // минимально-нормальное: x0 = x1
        CHECK_NEAR(xr.value()[1], 0.5, 1e-6);
    }

    // ── Distributions ────────────────────────────────────────────
    {
        CHECK_NEAR(normalCdf(0.0), 0.5, 1e-12);
        CHECK_NEAR(normalCdf(1.959963985, 0.0, 1.0), 0.975, 1e-4);
        CHECK_NEAR(normalPdf(0.0), 0.3989422804, 1e-9);
        CHECK_NEAR(chiSquareCdf(2.0, 2.0), 0.6321205588, 1e-9);
        CHECK_NEAR(exponentialCdf(1.0, 1.0), 0.6321205588, 1e-9);
        CHECK_NEAR(studentTCdf(0.0, 10.0), 0.5, 1e-12);
        CHECK_NEAR(studentTCdf(2.228138852, 10.0), 0.975, 1e-3);
        CHECK_NEAR(logNormalCdf(1.0, 0.0, 1.0), 0.5, 1e-12);
        CHECK_NEAR(lowerRegularizedGamma(1.0, 1.0), 0.6321205588, 1e-9);
        CHECK_NEAR(regularizedIncompleteBeta(0.5, 1.0, 1.0), 0.5, 1e-12);
    }

    // ── ODE ──────────────────────────────────────────────────────
    {
        OdeRhs expRhs = [](Scalar, const VectorN& y) { return VectorN{y[0]}; };
        auto ya = integrateRK4(expRhs, 0.0, VectorN{1.0}, 1.0, 40);
        CHECK_NEAR(ya[0], std::exp(1.0), 1e-8);
        auto yb = integrateAdaptiveRKF45(expRhs, 0.0, VectorN{1.0}, 1.0, 1e-9);
        CHECK_NEAR(yb[0], std::exp(1.0), 1e-7);

        OdeRhs oscRhs = [](Scalar, const VectorN& y) { return VectorN{y[1], -y[0]}; };
        auto yc = integrateRK4(oscRhs, 0.0, VectorN{0.0, 1.0}, std::acos(-1.0) / 2, 40);
        CHECK_NEAR(yc[0], 1.0, 1e-5);
        CHECK_NEAR(yc[1], 0.0, 1e-4);
    }

    // ── Polynomials ──────────────────────────────────────────────
    {
        std::vector<Scalar> c{-6, 11, -6, 1}; // (x-1)(x-2)(x-3)
        auto r = polynomialRoots(c);
        int found = 0;
        for (const auto& z : r)
            if (std::abs(z.imag()) < 1e-6 &&
                (std::abs(z.real() - 1.0) < 1e-4 ||
                 std::abs(z.real() - 2.0) < 1e-4 ||
                 std::abs(z.real() - 3.0) < 1e-4))
                ++found;
        assert(found == 3);

        auto d = polynomialDerivative(std::vector<Scalar>{0, 0, 1});
        CHECK_NEAR(d[0], 0.0, 1e-12);
        CHECK_NEAR(d[1], 2.0, 1e-12);

        std::vector<Scalar> xs{0, 1, 2, 3, 4}, ys{1, 3, 5, 7, 9};
        auto fit = polynomialFit(xs, ys, 2);
        assert(fit.has_value());
        CHECK_NEAR(fit.value()[0], 1.0, 1e-6);
        CHECK_NEAR(fit.value()[1], 2.0, 1e-6);
    }

    // ── FFT ──────────────────────────────────────────────────────
    {
        std::vector<FftComplex> a{Cx(1), Cx(1), Cx(1), Cx(1)};
        auto F = fft(a);
        CHECK_NEAR(F[0].real(), 4.0, 1e-9);
        CHECK_NEAR(std::abs(F[1]), 0.0, 1e-9);

        std::vector<Scalar> sig{1, 2, 3, 4, 5, 6, 7, 8};
        auto S = fftReal(sig);
        fftInPlace(S, true);
        for (std::size_t i = 0; i < sig.size(); ++i)
            CHECK_NEAR(S[i].real(), sig[i], 1e-9);
    }

    // ── Transforms ──────────────────────────────────────────────
    {
        Cx L1 = laplaceTransform([](Scalar) { return 1.0; }, Cx(3.0));
        CHECK_NEAR(L1.real(), 1.0 / 3.0, 1e-4);
        Cx L2 = laplaceTransform([](Scalar t) { return std::exp(-2.0 * t); }, Cx(3.0));
        CHECK_NEAR(L2.real(), 0.2, 1e-4);
        Cx L3 = laplaceTransform([](Scalar t) { return t; }, Cx(2.0));
        CHECK_NEAR(L3.real(), 0.25, 1e-4);

        const int n = 512;
        const double T = 6.0;
        auto fr = continuousFourierTransform([](Scalar t) { return std::exp(-t * t); }, -T, T, n);
        auto rec = inverseFourierTransform(fr.spectrum, -T, T, n);
        for (int i = 0; i < n; ++i)
        {
            const double t = -T + 2.0 * T * i / n;
            CHECK_NEAR(rec[i], std::exp(-t * t), 1e-2);
        }
    }

    // ── SpecialFunctions ────────────────────────────────────────
    {
        CHECK_NEAR(besselJ0(0.0), 1.0, 1e-12);
        CHECK_NEAR(besselJ0(1.0), 0.7651976866, 1e-7);
        CHECK_NEAR(besselJ1(1.0), 0.4400505857, 1e-7);
        CHECK_NEAR(besselJ(2, 1.0), 0.1149034849, 1e-7);
        CHECK_NEAR(besselJ(0, 2.5), -0.0483839979, 1e-6);
        CHECK_NEAR(besselY0(1.0), 0.0882569642, 1e-7);
        CHECK_NEAR(besselY1(1.0), -0.7812128213, 1e-7);
        CHECK_NEAR(betaFunction(2.0, 3.0), 1.0 / 12.0, 1e-9);

        // Модифицированные функции Бесселя Iₙ / Kₙ.
        CHECK_NEAR(besselI0(0.0), 1.0, 1e-12);
        CHECK_NEAR(besselI0(1.0), 1.2660658778, 1e-7);
        CHECK_NEAR(besselI1(1.0), 0.5651591039, 1e-7);
        CHECK_NEAR(besselI(2, 1.0), 0.1357476697, 1e-7);
        CHECK_NEAR(besselI(3, 2.0), 0.2127399591, 1e-7);
        CHECK_NEAR(besselI0(2.5), 3.2898423764, 1e-5);

        CHECK_NEAR(besselK0(1.0), 0.42111641, 1e-5);
        CHECK_NEAR(besselK1(1.0), 0.60199920, 1e-4);
        CHECK_NEAR(besselK(2, 1.0), 1.6251148, 1e-4);
        CHECK_NEAR(besselK(3, 1.0), 7.1024583, 1e-3);
        CHECK_NEAR(besselK0(0.5), 0.9245707, 1e-4);
        CHECK_NEAR(besselK0(2.0), 0.11389387, 1e-3);
        CHECK_NEAR(besselK1(2.0), 0.13986588, 1e-3);
        CHECK_NEAR(besselK(5, 2.0), 9.4335181, 1e-2);
    }

    // ── Iterative ───────────────────────────────────────────────
    {
        MatrixN A{{2, 1}, {1, 3}};
        VectorN b{5, 10};
        auto x = conjugateGradient(A, b);
        CHECK_NEAR(x[0], 1.0, 1e-8);
        CHECK_NEAR(x[1], 3.0, 1e-8);
        auto gs = gaussSeidel(A, b);
        CHECK_NEAR(gs.value()[0], 1.0, 1e-7);
        CHECK_NEAR(gs.value()[1], 3.0, 1e-7);
        auto jc = jacobiSolve(A, b);
        CHECK_NEAR(jc.value()[0], 1.0, 1e-6);
        CHECK_NEAR(jc.value()[1], 3.0, 1e-6);
    }

    // ── PDE ────────────────────────────────────────────────────
    {
        const Scalar kPi = std::acos(Scalar(-1));
        const int nx = 51, nt = 60;
        auto heat = solveHeat1D(Scalar(0.1), Scalar(1), Scalar(0.1), nx, nt,
            [kPi](Scalar x) { return std::sin(kPi * x); });
        const Scalar heatExact = std::sin(kPi * heat.x[nx / 2]) *
            std::exp(-Scalar(0.1) * kPi * kPi * Scalar(0.1));
        CHECK_NEAR(heat.snapshots.back()[nx / 2], heatExact, 2e-2);

        auto wave = solveWave1D(Scalar(1), Scalar(1), Scalar(0.25), nx, 50,
            [kPi](Scalar x) { return std::sin(kPi * x); });
        const Scalar waveExact = std::sin(kPi * wave.x[nx / 2]) * std::cos(kPi * Scalar(0.25));
        CHECK_NEAR(wave.snapshots.back()[nx / 2], waveExact, 2e-2);

        // 2D Пуассон −Δu = 2π²·sin(πx)sin(πy), точное решение u = sin(πx)sin(πy).
        const int n2 = 41;
        auto poisson = solvePoisson2D(
            n2,
            [kPi](Scalar x, Scalar y) { return Scalar(2) * kPi * kPi * std::sin(kPi * x) * std::sin(kPi * y); },
            [](Scalar, Scalar) { return Scalar(0); },
            Scalar(1.9), Scalar(1e-8), 100000);
        Scalar maxerr = Scalar(0);
        for (int i = 0; i < n2; ++i)
            for (int j = 0; j < n2; ++j)
            {
                const Scalar x = static_cast<Scalar>(i) / (n2 - 1);
                const Scalar y = static_cast<Scalar>(j) / (n2 - 1);
                maxerr = std::max(maxerr, std::abs(poisson.u[i][j] - std::sin(kPi * x) * std::sin(kPi * y)));
            }
        CHECK_NEAR(maxerr, 0.0, 1e-2);
    }

    // ── Sparse + Krylov ─────────────────────────────────────────
    {
        MatrixN D{{4, 0, 1}, {0, 3, 0}, {1, 0, 2}};
        auto S = toSparse(D);
        VectorN xv{1, 2, 3};
        auto ys = spmv(S, xv);
        auto yd = matVec(D, xv);
        CHECK_NEAR(ys[0], yd[0], 1e-12);
        CHECK_NEAR(ys[1], yd[1], 1e-12);
        CHECK_NEAR(ys[2], yd[2], 1e-12);

        MatrixN A{{3, 2}, {1, 4}};
        VectorN b{5, 6}; // решение (0.8, 1.3)
        auto Sa = toSparse(A);
        auto mv = [&](const VectorN& v) { return spmv(Sa, v); };
        auto xg = gmres(mv, b, 30, 500, 1e-10);
        CHECK_NEAR(xg[0], 0.8, 1e-7);
        CHECK_NEAR(xg[1], 1.3, 1e-7);
        auto xb = bicgstab(mv, b, 500, 1e-10);
        CHECK_NEAR(xb[0], 0.8, 1e-7);
        CHECK_NEAR(xb[1], 1.3, 1e-7);

        // Большая ленточная СЛАУ: сходимость Krylov по невязке.
        const int n = 60;
        MatrixN T(n, VectorN(n, Scalar(0)));
        for (int i = 0; i < n; ++i)
        {
            T[i][i] = Scalar(4);
            if (i > 0)
                T[i][i - 1] = Scalar(-1);
            if (i < n - 1)
                T[i][i + 1] = Scalar(-1);
        }
        VectorN tb(n, Scalar(1));
        auto St = toSparse(T);
        auto mvT = [&](const VectorN& v) { return spmv(St, v); };
        auto xgT = gmres(mvT, tb, 30, 2000, 1e-12);
        auto rg = spmv(St, xgT);
        Scalar resG = Scalar(0);
        for (int i = 0; i < n; ++i)
            resG += (rg[i] - tb[i]) * (rg[i] - tb[i]);
        CHECK_NEAR(std::sqrt(resG), 0.0, 1e-8);
    }

    // ── Splines ──────────────────────────────────────────────────
    {
        std::vector<Scalar> xs, ys;
        const int N = 9;
        for (int i = 0; i < N; ++i)
        {
            const Scalar t = std::acos(Scalar(-1)) * i / (N - 1);
            xs.push_back(t);
            ys.push_back(std::sin(t));
        }
        auto sp = buildCubicSpline(xs, ys);
        assert(sp.has_value());
        for (std::size_t i = 0; i < xs.size(); ++i)
            CHECK_NEAR(evalSpline(sp.value(), xs[i]), ys[i], 1e-12);
        Scalar maxerr = Scalar(0);
        for (int i = 0; i < 100; ++i)
        {
            const Scalar t = std::acos(Scalar(-1)) * i / 100.0;
            maxerr = std::max(maxerr, std::abs(evalSpline(sp.value(), t) - std::sin(t)));
        }
        CHECK_NEAR(maxerr, 0.0, 1e-3);
        CHECK_NEAR(evalSplineDerivative(sp.value(), 0.0), 1.0, 1e-3);
    }

    std::printf("MIR4D_MathNumeric: все проверки пройдены\n");
    return 0;
}
