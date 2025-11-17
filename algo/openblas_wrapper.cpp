#include "algorithms.hpp"
#include <stdexcept>

// BLAS/CBLAS interface
extern "C" {
    // CBLAS dgemm function for matrix multiplication
    // C = alpha * A * B + beta * C
    void cblas_dgemm(
        const int Order,        // Row-major (CblasRowMajor=101) or Column-major
        const int TransA,       // Transpose A? (CblasNoTrans=111, CblasTrans=112)
        const int TransB,       // Transpose B?
        const int M,            // Rows of A and C
        const int N,            // Columns of B and C
        const int K,            // Columns of A, Rows of B
        const double alpha,     // Scalar alpha
        const double *A,        // Matrix A
        const int lda,          // Leading dimension of A
        const double *B,        // Matrix B
        const int ldb,          // Leading dimension of B
        const double beta,      // Scalar beta
        double *C,              // Matrix C (output)
        const int ldc           // Leading dimension of C
    );
}

// CBLAS constants
const int CblasRowMajor = 101;
const int CblasNoTrans = 111;

namespace matmul {
namespace openblas {

Matrix multiply(const Matrix& A, const Matrix& B) {
    if (A.cols() != B.rows()) {
        throw std::runtime_error("Matrix dimensions incompatible for multiplication");
    }

    int m = A.rows();
    int n = B.cols();
    int k = A.cols();

    Matrix C(m, n);
    C.zero();

    // Call CBLAS dgemm: C = 1.0 * A * B + 0.0 * C
    cblas_dgemm(
        CblasRowMajor,      // Row-major order
        CblasNoTrans,       // Don't transpose A
        CblasNoTrans,       // Don't transpose B
        m,                  // Rows of A and C
        n,                  // Columns of B and C
        k,                  // Columns of A, Rows of B
        1.0,                // alpha = 1.0
        A.data(),           // Matrix A
        k,                  // Leading dimension of A
        B.data(),           // Matrix B
        n,                  // Leading dimension of B
        0.0,                // beta = 0.0
        C.data(),           // Matrix C
        n                   // Leading dimension of C
    );

    return C;
}

} // namespace openblas
} // namespace matmul
