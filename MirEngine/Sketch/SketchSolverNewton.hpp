#pragma once

#include "SketchConstraintEvaluator.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace mir
{

struct SketchNewtonOptions
{
    std::size_t maxIterations{64};
    double tolerance{1.0e-8};
    double finiteDifferenceStep{1.0e-6};
    double maxStep{1.0e6};
};

struct SketchNewtonResult
{
    bool converged{false};
    std::size_t iterations{0};
    double residual{std::numeric_limits<double>::infinity()};
};

class SketchSolverNewton
{
public:
    [[nodiscard]] SketchNewtonResult solve(
        const SketchEquationSystem& system,
        std::vector<double>& variables,
        SketchNewtonOptions options = {}) const
    {
        SketchNewtonResult result{};
        if (system.all().empty())
        {
            result.converged = true;
            result.residual = 0.0;
            return result;
        }

        if (variables.empty() || options.maxIterations == 0 ||
            !std::isfinite(options.tolerance) || options.tolerance < 0.0 ||
            !std::isfinite(options.finiteDifferenceStep) ||
            options.finiteDifferenceStep <= 0.0 ||
            !std::isfinite(options.maxStep) || options.maxStep <= 0.0)
        {
            return result;
        }

        const double h = std::max(1.0e-8, options.finiteDifferenceStep);
        SketchConstraintEvaluator evaluator{};
        const std::size_t equationCount = system.all().size();
        const std::size_t variableCount = variables.size();

        for (std::size_t iteration = 0; iteration < options.maxIterations; ++iteration)
        {
            const auto current = evaluator.evaluate(system, variables);
            result.iterations = iteration;
            result.residual = std::sqrt(std::max(0.0, current.squaredError));

            if (current.invalidEquationCount != 0 || !std::isfinite(result.residual))
                return result;

            if (result.residual <= options.tolerance)
            {
                result.converged = true;
                return result;
            }

            // Build a finite-difference Jacobian and solve the damped normal
            // equations (J^T J + lambda I) * delta = -J^T r. This updates all
            // sketch variables together and is substantially more stable than
            // coordinate-wise Newton steps for coupled constraints such as a
            // rectangle, where changing one coordinate immediately affects
            // several equations.
            std::vector<double> residuals(equationCount, 0.0);
            for (std::size_t row = 0; row < equationCount; ++row)
                residuals[row] = system.all()[row].residual(variables);

            std::vector<double> jacobian(equationCount * variableCount, 0.0);
            for (std::size_t column = 0; column < variableCount; ++column)
            {
                const double original = variables[column];

                variables[column] = original + h;
                std::vector<double> plus(equationCount, 0.0);
                for (std::size_t row = 0; row < equationCount; ++row)
                    plus[row] = system.all()[row].residual(variables);

                variables[column] = original - h;
                std::vector<double> minus(equationCount, 0.0);
                for (std::size_t row = 0; row < equationCount; ++row)
                    minus[row] = system.all()[row].residual(variables);

                variables[column] = original;

                for (std::size_t row = 0; row < equationCount; ++row)
                {
                    const double derivative = (plus[row] - minus[row]) / (2.0 * h);
                    if (!std::isfinite(derivative))
                        return result;
                    jacobian[row * variableCount + column] = derivative;
                }
            }

            std::vector<double> normal(variableCount * variableCount, 0.0);
            std::vector<double> rhs(variableCount, 0.0);

            for (std::size_t i = 0; i < variableCount; ++i)
            {
                for (std::size_t j = 0; j < variableCount; ++j)
                {
                    double value = 0.0;
                    for (std::size_t row = 0; row < equationCount; ++row)
                        value += jacobian[row * variableCount + i] *
                                 jacobian[row * variableCount + j];
                    normal[i * variableCount + j] = value;
                }

                double value = 0.0;
                for (std::size_t row = 0; row < equationCount; ++row)
                    value += jacobian[row * variableCount + i] * residuals[row];
                rhs[i] = -value;
            }

            // Small Tikhonov damping keeps singular or nearly singular sketch
            // systems solvable without changing the exact solution of a well
            // conditioned system.
            double damping = 1.0e-10;
            std::vector<double> delta;
            bool solved = false;
            for (int attempt = 0; attempt < 8 && !solved; ++attempt)
            {
                auto systemMatrix = normal;
                for (std::size_t i = 0; i < variableCount; ++i)
                    systemMatrix[i * variableCount + i] += damping;

                delta = solveLinearSystem(systemMatrix, rhs, variableCount);
                if (!delta.empty())
                    solved = true;
                else
                    damping *= 100.0;
            }

            if (!solved)
                return result;

            for (double& step : delta)
                step = std::clamp(step, -options.maxStep, options.maxStep);

            bool improved = false;
            double stepScale = 1.0;
            for (int attempt = 0; attempt < 16; ++attempt)
            {
                for (std::size_t i = 0; i < variableCount; ++i)
                    variables[i] += delta[i] * stepScale;

                const auto candidate = evaluator.evaluate(system, variables);
                if (candidate.invalidEquationCount == 0 &&
                    std::isfinite(candidate.squaredError) &&
                    candidate.squaredError < current.squaredError)
                {
                    improved = true;
                    break;
                }

                for (std::size_t i = 0; i < variableCount; ++i)
                    variables[i] -= delta[i] * stepScale;
                stepScale *= 0.5;
            }

            if (!improved)
                break;
        }

        const auto finalReport = evaluator.evaluate(system, variables);
        result.residual = std::sqrt(std::max(0.0, finalReport.squaredError));
        result.converged = finalReport.invalidEquationCount == 0 &&
                           std::isfinite(result.residual) &&
                           result.residual <= options.tolerance;
        return result;
    }

private:
    [[nodiscard]] static std::vector<double> solveLinearSystem(
        std::vector<double> matrix,
        std::vector<double> rhs,
        std::size_t n)
    {
        if (matrix.size() != n * n || rhs.size() != n || n == 0)
            return {};

        for (std::size_t pivot = 0; pivot < n; ++pivot)
        {
            std::size_t best = pivot;
            double bestMagnitude = std::abs(matrix[pivot * n + pivot]);
            for (std::size_t row = pivot + 1; row < n; ++row)
            {
                const double magnitude = std::abs(matrix[row * n + pivot]);
                if (magnitude > bestMagnitude)
                {
                    best = row;
                    bestMagnitude = magnitude;
                }
            }

            if (!std::isfinite(bestMagnitude) || bestMagnitude <= 1.0e-14)
                return {};

            if (best != pivot)
            {
                for (std::size_t column = pivot; column < n; ++column)
                    std::swap(matrix[pivot * n + column], matrix[best * n + column]);
                std::swap(rhs[pivot], rhs[best]);
            }

            const double diagonal = matrix[pivot * n + pivot];
            for (std::size_t row = pivot + 1; row < n; ++row)
            {
                const double factor = matrix[row * n + pivot] / diagonal;
                if (!std::isfinite(factor))
                    return {};

                matrix[row * n + pivot] = 0.0;
                for (std::size_t column = pivot + 1; column < n; ++column)
                    matrix[row * n + column] -= factor * matrix[pivot * n + column];
                rhs[row] -= factor * rhs[pivot];
            }
        }

        std::vector<double> solution(n, 0.0);
        for (std::size_t row = n; row-- > 0;)
        {
            double value = rhs[row];
            for (std::size_t column = row + 1; column < n; ++column)
                value -= matrix[row * n + column] * solution[column];

            const double diagonal = matrix[row * n + row];
            if (!std::isfinite(diagonal) || std::abs(diagonal) <= 1.0e-14)
                return {};

            solution[row] = value / diagonal;
            if (!std::isfinite(solution[row]))
                return {};
        }

        return solution;
    }
};

} // namespace mir
