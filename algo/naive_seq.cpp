#include "algorithms.hpp"
#include <stdexcept>

namespace matmul {
namespace naive {

Matrix sequential(const Matrix& A, const Matrix& B, const OptimizationOptions& opt) {
    if (A.cols() != B.rows()) {
        throw std::runtime_error("Matrix dimensions incompatible for multiplication");
    }

    int m = A.rows();
    int n = B.cols();
    int k = A.cols();

    Matrix C(m, n);
    C.zero();

    if (opt.cache_friendly && opt.use_blocking) {
        // Cache-friendly blocked implementation
        int block_size = opt.block_size;

        for (int ii = 0; ii < m; ii += block_size) {
            for (int jj = 0; jj < n; jj += block_size) {
                for (int kk = 0; kk < k; kk += block_size) {
                    // Compute block
                    int i_max = std::min(ii + block_size, m);
                    int j_max = std::min(jj + block_size, n);
                    int k_max = std::min(kk + block_size, k);

                    for (int i = ii; i < i_max; ++i) {
                        for (int j = jj; j < j_max; ++j) {
                            double sum = C(i, j);
                            for (int kk_inner = kk; kk_inner < k_max; ++kk_inner) {
                                sum += A(i, kk_inner) * B(kk_inner, j);
                            }
                            C(i, j) = sum;
                        }
                    }
                }
            }
        }
    } else {
        // Standard ijk order
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                double sum = 0.0;
                for (int ki = 0; ki < k; ++ki) {
                    sum += A(i, ki) * B(ki, j);
                }
                C(i, j) = sum;
            }
        }
    }

    return C;
}

} // namespace naive
} // namespace matmul
