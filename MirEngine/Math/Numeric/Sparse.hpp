
#pragma once

#include "LinearSystem.hpp"

#include <algorithm>
#include <vector>

namespace mir::math
{

struct Triplet
{
    std::size_t row;
    std::size_t col;
    Scalar value;
};

struct SparseMatrixCSR
{
    std::size_t rows = 0;
    std::size_t cols = 0;
    std::vector<std::size_t> rowPtr;
    std::vector<std::size_t> colIdx;
    std::vector<Scalar> values;
};

[[nodiscard]] inline SparseMatrixCSR buildSparse(
    std::size_t rows,
    std::size_t cols,
    std::vector<Triplet> triplets)
{
    std::sort(triplets.begin(), triplets.end(), [](const Triplet& a, const Triplet& b) {
        if (a.row != b.row)
            return a.row < b.row;
        return a.col < b.col;
    });

    std::vector<Triplet> merged;
    merged.reserve(triplets.size());
    for (const auto& t : triplets)
    {
        if (!merged.empty() && merged.back().row == t.row && merged.back().col == t.col)
            merged.back().value += t.value;
        else
            merged.push_back(t);
    }

    SparseMatrixCSR M;
    M.rows = rows;
    M.cols = cols;
    M.rowPtr.assign(rows + 1, 0);
    for (const auto& t : merged)
        ++M.rowPtr[t.row + 1];
    for (std::size_t i = 0; i < rows; ++i)
        M.rowPtr[i + 1] += M.rowPtr[i];

    M.colIdx.resize(merged.size());
    M.values.resize(merged.size());
    std::vector<std::size_t> cursor(rows, 0);
    for (const auto& t : merged)
    {
        const std::size_t pos = M.rowPtr[t.row] + cursor[t.row]++;
        M.colIdx[pos] = t.col;
        M.values[pos] = t.value;
    }
    return M;
}

[[nodiscard]] inline SparseMatrixCSR toSparse(const MatrixN& A)
{
    std::vector<Triplet> t;
    for (std::size_t i = 0; i < A.size(); ++i)
        for (std::size_t j = 0; j < A[i].size(); ++j)
            if (A[i][j] != Scalar(0))
                t.push_back({i, j, A[i][j]});
    return buildSparse(A.size(), A.empty() ? 0 : A.front().size(), std::move(t));
}

[[nodiscard]] inline VectorN spmv(const SparseMatrixCSR& M, const VectorN& x)
{
    VectorN y(M.rows, Scalar(0));
    for (std::size_t i = 0; i < M.rows; ++i)
    {
        Scalar s = Scalar(0);
        for (std::size_t k = M.rowPtr[i]; k < M.rowPtr[i + 1]; ++k)
            s += M.values[k] * x[M.colIdx[k]];
        y[i] = s;
    }
    return y;
}

}
