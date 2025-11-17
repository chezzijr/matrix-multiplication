#ifndef VERIFICATION_HPP
#define VERIFICATION_HPP

#include "matrix.hpp"
#include "config.hpp"
#include <string>
#include <vector>

namespace matmul {
namespace verification {

// Print a detailed comparison report with formatted table
void print_comparison_report(const ComparisonResult& result,
                             const std::string& label1,
                             const std::string& label2);

// Validate a result against OpenBLAS reference implementation
// Returns true if validation passes
bool validate_against_reference(const Matrix& result,
                                const Matrix& A,
                                const Matrix& B,
                                Algorithm algo,
                                const Config& config);

// Run full verification suite comparing multiple algorithms
// Only works in Sequential/OpenMP modes (not MPI)
void run_verification_suite(const Matrix& A,
                           const Matrix& B,
                           const Config& config,
                           int rank);

// Compare two matrices and print results
// Returns true if they match within tolerance
bool compare_and_report(const Matrix& result1,
                        const Matrix& result2,
                        const std::string& label1,
                        const std::string& label2,
                        double abs_tol = 1e-8,
                        double rel_tol = 1e-5);

} // namespace verification
} // namespace matmul

#endif // VERIFICATION_HPP
