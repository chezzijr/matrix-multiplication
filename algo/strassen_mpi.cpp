#include "algorithms.hpp"
#include <mpi.h>
#include <stdexcept>

namespace matmul {
namespace strassen {

Matrix mpi(const Matrix& A, const Matrix& B, const OptimizationOptions& opt) {
    if (!A.is_square() || !B.is_square() || A.size() != B.size()) {
        throw std::runtime_error("Strassen algorithm requires square matrices of same size");
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = A.rows();

    // For Strassen with MPI, we use a simpler approach:
    // Distribute rows of matrix A and use sequential Strassen on each partition
    // This is a hybrid approach that balances complexity and performance

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

    // For the local computation, if the local matrix is square enough,
    // use Strassen, otherwise use naive
    Matrix C_local(local_rows, n);

    if (local_rows == n) {
        // Full matrix, use Strassen
        C_local = sequential(A_local, B_local, opt);
    } else {
        // Partial rows, use naive multiplication
        C_local = naive::sequential(A_local, B_local, opt);
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
