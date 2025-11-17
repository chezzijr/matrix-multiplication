#include "algorithms.hpp"
#include <mpi.h>
#include <omp.h>
#include <stdexcept>

namespace matmul {
namespace strassen {

Matrix hybrid(const Matrix& A, const Matrix& B, const OptimizationOptions& opt, int num_threads) {
    if (!A.is_square() || !B.is_square() || A.size() != B.size()) {
        throw std::runtime_error("Strassen algorithm requires square matrices of same size");
    }

    omp_set_num_threads(num_threads);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = A.rows();

    // Distribute rows of matrix A and use OpenMP Strassen on each partition
    int rows_per_proc = n / size;
    int remainder = n % size;
    int local_rows = rows_per_proc + (rank < remainder ? 1 : 0);
    int row_offset = rank * rows_per_proc + std::min(rank, remainder);

    // Broadcast matrix B to all processes
    Matrix B_local = B;

    // Create local portion of A
    Matrix A_local(local_rows, n);
    for (int i = 0; i < local_rows; ++i) {
        for (int j = 0; j < n; ++j) {
            A_local(i, j) = A(row_offset + i, j);
        }
    }

    // For the local computation, use OpenMP-enabled algorithm
    Matrix C_local(local_rows, n);

    if (local_rows == n) {
        // Full matrix, use Strassen with OpenMP
        C_local = openmp(A_local, B_local, opt, num_threads);
    } else {
        // Partial rows, use naive multiplication with OpenMP
        C_local = naive::openmp(A_local, B_local, opt, num_threads);
    }

    // Gather results
    Matrix C(n, n);
    C.zero();

    std::vector<int> sendcounts(size);
    std::vector<int> displs(size);

    for (int i = 0; i < size; ++i) {
        int proc_rows = rows_per_proc + (i < remainder ? 1 : 0);
        sendcounts[i] = proc_rows * n;
        displs[i] = (i * rows_per_proc + std::min(i, remainder)) * n;
    }

    MPI_Allgatherv(C_local.data(), local_rows * n, MPI_DOUBLE,
                   C.data(), sendcounts.data(), displs.data(),
                   MPI_DOUBLE, MPI_COMM_WORLD);

    return C;
}

} // namespace strassen
} // namespace matmul
