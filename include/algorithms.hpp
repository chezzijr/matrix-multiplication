#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include "matrix.hpp"
#include "config.hpp"

namespace matmul {

// Algorithm dispatcher - calls the appropriate implementation based on config
Matrix multiply(const Matrix& A, const Matrix& B, const Config& config);

// Naive algorithm implementations
namespace naive {
    Matrix sequential(const Matrix& A, const Matrix& B, const OptimizationOptions& opt);
    Matrix openmp(const Matrix& A, const Matrix& B, const OptimizationOptions& opt, int num_threads);
    Matrix mpi(const Matrix& A, const Matrix& B, const OptimizationOptions& opt);
    Matrix hybrid(const Matrix& A, const Matrix& B, const OptimizationOptions& opt, int num_threads);
}

// Strassen algorithm implementations
namespace strassen {
    Matrix sequential(const Matrix& A, const Matrix& B, const OptimizationOptions& opt);
    Matrix openmp(const Matrix& A, const Matrix& B, const OptimizationOptions& opt, int num_threads);
    Matrix mpi(const Matrix& A, const Matrix& B, const OptimizationOptions& opt);
    Matrix hybrid(const Matrix& A, const Matrix& B, const OptimizationOptions& opt, int num_threads);
}

// OpenBLAS implementation
namespace openblas {
    Matrix multiply(const Matrix& A, const Matrix& B);
}

} // namespace matmul

#endif // ALGORITHMS_HPP
