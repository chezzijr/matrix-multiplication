#include "verification.hpp"
#include "algorithms.hpp"
#include "timer.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace matmul {
namespace verification {

void print_comparison_report(const ComparisonResult& result,
                             const std::string& label1,
                             const std::string& label2) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "      Comparison Report\n";
    std::cout << "========================================\n";
    std::cout << "Algorithm 1:     " << label1 << "\n";
    std::cout << "Algorithm 2:     " << label2 << "\n";
    std::cout << "----------------------------------------\n";

    // Overall result
    if (result.all_close) {
        std::cout << "Status:          \033[32mPASSED\033[0m ✓\n";  // Green
    } else {
        std::cout << "Status:          \033[31mFAILED\033[0m ✗\n";  // Red
        std::cout << "Failures:        " << result.num_failures << " / "
                  << result.num_elements << " ("
                  << std::fixed << std::setprecision(2) << result.failure_rate << "%)\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "Error Statistics:\n";
    std::cout << "  Max Absolute:  " << std::scientific << std::setprecision(6)
              << result.max_abs_error << "\n";
    std::cout << "  Mean Absolute: " << std::scientific << std::setprecision(6)
              << result.mean_abs_error << "\n";
    std::cout << "  RMS Error:     " << std::scientific << std::setprecision(6)
              << result.rms_error << "\n";
    std::cout << "  Max Relative:  " << std::fixed << std::setprecision(6)
              << (result.max_rel_error * 100.0) << "%\n";
    std::cout << "  Mean Relative: " << std::fixed << std::setprecision(6)
              << (result.mean_rel_error * 100.0) << "%\n";

    std::cout << "----------------------------------------\n";
    std::cout << "Tolerances:\n";
    std::cout << "  Absolute:      " << std::scientific << std::setprecision(3)
              << result.abs_tolerance << "\n";
    std::cout << "  Relative:      " << std::scientific << std::setprecision(3)
              << result.rel_tolerance << "\n";

    if (result.worst_row >= 0) {
        std::cout << "----------------------------------------\n";
        std::cout << "Worst Error Location:\n";
        std::cout << "  Position:      [" << result.worst_row << ", " << result.worst_col << "]\n";
        std::cout << "  " << label1 << ":  " << std::scientific << std::setprecision(10)
                  << result.worst_value_this << "\n";
        std::cout << "  " << label2 << ":  " << std::scientific << std::setprecision(10)
                  << result.worst_value_other << "\n";
        std::cout << "  Difference:    " << std::scientific << std::setprecision(10)
                  << (result.worst_value_this - result.worst_value_other) << "\n";
    }

    std::cout << "========================================\n";
    std::cout << "\n";
}

bool validate_against_reference(const Matrix& result,
                                const Matrix& A,
                                const Matrix& B,
                                Algorithm algo,
                                const Config& config) {
    std::cout << "Computing OpenBLAS reference result...\n";

    Matrix reference = openblas::multiply(A, B);

    std::cout << "Comparing " << algorithm_to_string(algo) << " vs OpenBLAS...\n";

    ComparisonResult cmp = result.compare(reference, config.abs_tolerance, config.rel_tolerance);

    print_comparison_report(cmp, algorithm_to_string(algo), "OpenBLAS");

    return cmp.all_close;
}

void run_verification_suite(const Matrix& A,
                           const Matrix& B,
                           const Config& config,
                           int rank) {
    if (rank != 0) {
        // Only rank 0 runs verification
        return;
    }

    std::cout << "\n";
    std::cout << "================================================\n";
    std::cout << "       VERIFICATION SUITE\n";
    std::cout << "================================================\n";
    std::cout << "Matrix Size:     " << A.rows() << "x" << A.cols() << "\n";
    std::cout << "Algorithms:      ";

    for (size_t i = 0; i < config.verify_algorithms.size(); ++i) {
        std::cout << algorithm_to_string(config.verify_algorithms[i]);
        if (i < config.verify_algorithms.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "\n";
    std::cout << "Execution Mode:  " << mode_to_string(config.mode) << "\n";
    std::cout << "================================================\n\n";

    // Store results from each algorithm
    std::vector<Matrix> results;
    std::vector<std::string> labels;
    std::vector<double> times;

    // Run each algorithm
    for (Algorithm algo : config.verify_algorithms) {
        Config algo_config = config;
        algo_config.algorithm = algo;

        std::string label = algorithm_to_string(algo);
        std::cout << "Running " << label << "...\n";

        Timer timer;
        timer.start();

        Matrix C = multiply(A, B, algo_config);

        timer.stop();
        double elapsed = timer.elapsed_seconds();

        results.push_back(C);
        labels.push_back(label);
        times.push_back(elapsed);

        std::cout << "  Time: " << std::fixed << std::setprecision(6) << elapsed << " seconds\n\n";
    }

    // Compare all pairs
    std::cout << "================================================\n";
    std::cout << "       PAIRWISE COMPARISONS\n";
    std::cout << "================================================\n\n";

    bool all_passed = true;

    for (size_t i = 0; i < results.size(); ++i) {
        for (size_t j = i + 1; j < results.size(); ++j) {
            ComparisonResult cmp = results[i].compare(results[j],
                                                      config.abs_tolerance,
                                                      config.rel_tolerance);

            print_comparison_report(cmp, labels[i], labels[j]);

            if (!cmp.all_close) {
                all_passed = false;
            }
        }
    }

    // Summary
    std::cout << "================================================\n";
    std::cout << "       VERIFICATION SUMMARY\n";
    std::cout << "================================================\n";

    std::cout << "\nExecution Times:\n";
    for (size_t i = 0; i < labels.size(); ++i) {
        std::cout << "  " << std::setw(12) << labels[i] << ": "
                  << std::fixed << std::setprecision(6) << times[i] << " s\n";
    }

    std::cout << "\nOverall Status:  ";
    if (all_passed) {
        std::cout << "\033[32mALL TESTS PASSED\033[0m ✓\n";
    } else {
        std::cout << "\033[31mSOME TESTS FAILED\033[0m ✗\n";
    }

    std::cout << "================================================\n\n";
}

bool compare_and_report(const Matrix& result1,
                        const Matrix& result2,
                        const std::string& label1,
                        const std::string& label2,
                        double abs_tol,
                        double rel_tol) {
    ComparisonResult cmp = result1.compare(result2, abs_tol, rel_tol);
    print_comparison_report(cmp, label1, label2);
    return cmp.all_close;
}

} // namespace verification
} // namespace matmul
