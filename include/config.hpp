#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

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

// Helper functions to parse strings to enums
inline Algorithm parse_algorithm(const std::string& str) {
    std::string lower = str;
    for (char& c : lower) c = std::tolower(c);

    if (lower == "naive") return Algorithm::NAIVE;
    if (lower == "strassen") return Algorithm::STRASSEN;
    if (lower == "openblas" || lower == "blas") return Algorithm::OPENBLAS;

    throw std::runtime_error("Unknown algorithm: " + str);
}

inline ExecutionMode parse_execution_mode(const std::string& str) {
    std::string lower = str;
    for (char& c : lower) c = std::tolower(c);

    if (lower == "seq" || lower == "sequential") return ExecutionMode::SEQUENTIAL;
    if (lower == "omp" || lower == "openmp") return ExecutionMode::OPENMP;
    if (lower == "mpi") return ExecutionMode::MPI;
    if (lower == "hybrid") return ExecutionMode::HYBRID;

    throw std::runtime_error("Unknown execution mode: " + str);
}

// Print usage/help information
inline void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n\n";
    std::cout << "Matrix Multiplication with Multiple Algorithms and Parallelization Modes\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  -a, --algorithm <type>     Algorithm: naive, strassen, openblas (default: naive)\n";
    std::cout << "  -m, --mode <type>          Execution mode: seq, omp, mpi, hybrid (default: seq)\n";
    std::cout << "  -s, --size <N>             Matrix size NxN (default: 100)\n";
    std::cout << "  -t, --threads <N>          Number of OpenMP threads (default: 4)\n";
    std::cout << "  -o, --optimize             Enable cache-friendly blocking\n";
    std::cout << "  -b, --block-size <N>       Block size for optimization (default: 64)\n";
    std::cout << "  -i, --input <file>         Input CSV file (default: random matrices)\n";
    std::cout << "  --validate                 Validate result against OpenBLAS\n";
    std::cout << "  --verify                   Verification mode (compare multiple algorithms)\n";
    std::cout << "  -h, --help                 Show this help message\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  # Interactive mode (if no arguments)\n";
    std::cout << "  " << program_name << "\n\n";
    std::cout << "  # Run naive sequential 1000x1000\n";
    std::cout << "  " << program_name << " --algorithm naive --mode seq --size 1000\n\n";
    std::cout << "  # Run Strassen with MPI (4 processes)\n";
    std::cout << "  mpirun -np 4 " << program_name << " -a strassen -m mpi -s 2000 --optimize\n\n";
    std::cout << "  # Run with OpenMP and validate\n";
    std::cout << "  " << program_name << " -a naive -m omp -t 8 -s 1000 --validate\n\n";
    std::cout << "NOTE: When using MPI (mpirun), you must provide command-line arguments.\n";
    std::cout << "      Interactive mode is not available with mpirun.\n";
}

} // namespace matmul

#endif // CONFIG_HPP
