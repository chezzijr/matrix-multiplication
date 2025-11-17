#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>

namespace matmul {

// Algorithm types
enum class Algorithm {
    NAIVE,
    STRASSEN,
    OPENBLAS
};

// Execution mode
enum class ExecutionMode {
    SEQUENTIAL,
    OPENMP,
    MPI,
    HYBRID  // MPI + OpenMP
};

// Optimization options
struct OptimizationOptions {
    bool cache_friendly = false;
    bool use_blocking = false;
    int block_size = 64;
};

// Configuration for matrix multiplication
struct Config {
    Algorithm algorithm = Algorithm::NAIVE;
    ExecutionMode mode = ExecutionMode::SEQUENTIAL;
    OptimizationOptions optimization;

    // Parallelization parameters
    int num_threads = 1;      // For OpenMP
    int num_processes = 1;    // For MPI (informational, actual count from mpirun)

    // Matrix parameters
    int matrix_size = 100;
    std::string input_file = "";   // Empty = random initialization
    std::string output_file = "";  // Derived from input_file if provided

    // Results
    double execution_time = 0.0;

    // Verification options
    bool verification_mode = false;                    // Run multiple algorithms and compare
    std::vector<Algorithm> verify_algorithms;          // Algorithms to verify (for verification mode)
    std::vector<ExecutionMode> verify_modes;           // Modes to test (for verification mode)
    bool validate_against_openblas = false;            // Single-run validation against OpenBLAS
    double abs_tolerance = 1e-8;                       // Absolute error tolerance
    double rel_tolerance = 1e-5;                       // Relative error tolerance

    // Validation results (populated after validation)
    bool validation_performed = false;
    bool validation_passed = false;
};

// Helper functions to convert enums to strings
inline std::string algorithm_to_string(Algorithm algo) {
    switch (algo) {
        case Algorithm::NAIVE: return "Naive";
        case Algorithm::STRASSEN: return "Strassen";
        case Algorithm::OPENBLAS: return "OpenBLAS";
        default: return "Unknown";
    }
}

inline std::string mode_to_string(ExecutionMode mode) {
    switch (mode) {
        case ExecutionMode::SEQUENTIAL: return "Sequential";
        case ExecutionMode::OPENMP: return "OpenMP";
        case ExecutionMode::MPI: return "MPI";
        case ExecutionMode::HYBRID: return "Hybrid (MPI+OpenMP)";
        default: return "Unknown";
    }
}

} // namespace matmul

#endif // CONFIG_HPP
