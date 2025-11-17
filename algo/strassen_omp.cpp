#include "algorithms.hpp"
#include <omp.h>
#include <stdexcept>

namespace matmul {
namespace strassen {

// Threshold for switching to naive multiplication
const int STRASSEN_OMP_THRESHOLD = 64;

// Helper function for OpenMP Strassen recursion
static Matrix strassen_recursive_omp(const Matrix& A, const Matrix& B,
                                      const OptimizationOptions& opt, int num_threads) {
    int n = A.rows();

    // Base case: use naive OpenMP multiplication for small matrices
    if (n <= STRASSEN_OMP_THRESHOLD) {
        return naive::openmp(A, B, opt, num_threads);
    }

    // Ensure matrix size is even
    if (n % 2 != 0) {
        int padded_n = n + 1;
        Matrix A_padded(padded_n, padded_n);
        Matrix B_padded(padded_n, padded_n);
        A_padded.zero();
        B_padded.zero();

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                A_padded(i, j) = A(i, j);
                B_padded(i, j) = B(i, j);
            }
        }

        Matrix C_padded = strassen_recursive_omp(A_padded, B_padded, opt, num_threads);

        Matrix C(n, n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                C(i, j) = C_padded(i, j);
            }
        }
        return C;
    }

    int half = n / 2;

    // Divide matrices into quadrants
    Matrix A11 = A.submatrix(0, 0, half, half);
    Matrix A12 = A.submatrix(0, half, half, n);
    Matrix A21 = A.submatrix(half, 0, n, half);
    Matrix A22 = A.submatrix(half, half, n, n);

    Matrix B11 = B.submatrix(0, 0, half, half);
    Matrix B12 = B.submatrix(0, half, half, n);
    Matrix B21 = B.submatrix(half, 0, n, half);
    Matrix B22 = B.submatrix(half, half, n, n);

    // Compute the 7 Strassen products in parallel
    Matrix M1, M2, M3, M4, M5, M6, M7;

    #pragma omp parallel sections num_threads(num_threads)
    {
        #pragma omp section
        M1 = strassen_recursive_omp(A11 + A22, B11 + B22, opt, num_threads / 7 + 1);

        #pragma omp section
        M2 = strassen_recursive_omp(A21 + A22, B11, opt, num_threads / 7 + 1);

        #pragma omp section
        M3 = strassen_recursive_omp(A11, B12 - B22, opt, num_threads / 7 + 1);

        #pragma omp section
        M4 = strassen_recursive_omp(A22, B21 - B11, opt, num_threads / 7 + 1);

        #pragma omp section
        M5 = strassen_recursive_omp(A11 + A12, B22, opt, num_threads / 7 + 1);

        #pragma omp section
        M6 = strassen_recursive_omp(A21 - A11, B11 + B12, opt, num_threads / 7 + 1);

        #pragma omp section
        M7 = strassen_recursive_omp(A12 - A22, B21 + B22, opt, num_threads / 7 + 1);
    }

    // Compute result quadrants
    Matrix C11 = M1 + M4 - M5 + M7;
    Matrix C12 = M3 + M5;
    Matrix C21 = M2 + M4;
    Matrix C22 = M1 - M2 + M3 + M6;

    // Combine quadrants into result matrix
    Matrix C(n, n);
    C.set_submatrix(0, 0, C11);
    C.set_submatrix(0, half, C12);
    C.set_submatrix(half, 0, C21);
    C.set_submatrix(half, half, C22);

    return C;
}

Matrix openmp(const Matrix& A, const Matrix& B, const OptimizationOptions& opt, int num_threads) {
    if (!A.is_square() || !B.is_square() || A.size() != B.size()) {
        throw std::runtime_error("Strassen algorithm requires square matrices of same size");
    }

    omp_set_num_threads(num_threads);
    return strassen_recursive_omp(A, B, opt, num_threads);
}

} // namespace strassen
} // namespace matmul
