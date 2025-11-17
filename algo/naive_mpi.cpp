#include "algorithms.hpp"
#include <mpi.h>
#include <stdexcept>

namespace matmul {
namespace naive {

Matrix mpi(const Matrix& A, const Matrix& B, const OptimizationOptions& opt) {
    if (A.cols() != B.rows()) {
        throw std::runtime_error("Matrix dimensions incompatible for multiplication");
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int m = A.rows();
    int n = B.cols();
    int k = A.cols();

    // Calculate rows per process
    int rows_per_proc = m / size;
    int remainder = m % size;
    int local_rows = rows_per_proc + (rank < remainder ? 1 : 0);
    int row_offset = rank * rows_per_proc + std::min(rank, remainder);

    // Broadcast matrix B to all processes
    Matrix B_local = B;

    // Create local portion of A
    Matrix A_local(local_rows, k);
    for (int i = 0; i < local_rows; ++i) {
        for (int j = 0; j < k; ++j) {
            A_local(i, j) = A(row_offset + i, j);
        }
    }

    // Compute local result
    Matrix C_local(local_rows, n);
    C_local.zero();

    if (opt.cache_friendly && opt.use_blocking) {
        int block_size = opt.block_size;

        for (int ii = 0; ii < local_rows; ii += block_size) {
            for (int jj = 0; jj < n; jj += block_size) {
                for (int kk = 0; kk < k; kk += block_size) {
                    int i_max = std::min(ii + block_size, local_rows);
                    int j_max = std::min(jj + block_size, n);
                    int k_max = std::min(kk + block_size, k);

                    for (int i = ii; i < i_max; ++i) {
                        for (int j = jj; j < j_max; ++j) {
                            double sum = C_local(i, j);
                            for (int kk_inner = kk; kk_inner < k_max; ++kk_inner) {
                                sum += A_local(i, kk_inner) * B_local(kk_inner, j);
                            }
                            C_local(i, j) = sum;
                        }
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < local_rows; ++i) {
            for (int j = 0; j < n; ++j) {
                double sum = 0.0;
                for (int ki = 0; ki < k; ++ki) {
                    sum += A_local(i, ki) * B_local(ki, j);
                }
                C_local(i, j) = sum;
            }
        }
    }

    // Gather results
    Matrix C(m, n);
    C.zero();

    // Prepare send counts and displacements
    std::vector<int> sendcounts(size);
    std::vector<int> displs(size);

    for (int i = 0; i < size; ++i) {
        int proc_rows = rows_per_proc + (i < remainder ? 1 : 0);
        sendcounts[i] = proc_rows * n;
        displs[i] = (i * rows_per_proc + std::min(i, remainder)) * n;
    }

    // Gather all local results
    MPI_Allgatherv(C_local.data(), local_rows * n, MPI_DOUBLE,
                   C.data(), sendcounts.data(), displs.data(),
                   MPI_DOUBLE, MPI_COMM_WORLD);

    return C;
}

} // namespace naive
} // namespace matmul
