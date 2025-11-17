#include "matrix.hpp"
#include "algorithms.hpp"
#include "cli_menu.hpp"
#include "csv_io.hpp"
#include "timer.hpp"
#include "config.hpp"
#include "verification.hpp"
#include <iostream>
#include <iomanip>
#include <mpi.h>
#include <cstring>

// Cross-platform stdin detection
#ifdef _WIN32
    #include <io.h>
    #define isatty _isatty
    #define STDIN_FILENO 0
#else
    #include <unistd.h>
#endif

using namespace matmul;

// Algorithm dispatcher implementation
Matrix matmul::multiply(const Matrix& A, const Matrix& B, const Config& config) {
    switch (config.algorithm) {
        case Algorithm::NAIVE:
            switch (config.mode) {
                case ExecutionMode::SEQUENTIAL:
                    return naive::sequential(A, B, config.optimization);
                case ExecutionMode::OPENMP:
                    return naive::openmp(A, B, config.optimization, config.num_threads);
                case ExecutionMode::MPI:
                    return naive::mpi(A, B, config.optimization);
                case ExecutionMode::HYBRID:
                    return naive::hybrid(A, B, config.optimization, config.num_threads);
            }
            break;

        case Algorithm::STRASSEN:
            switch (config.mode) {
                case ExecutionMode::SEQUENTIAL:
                    return strassen::sequential(A, B, config.optimization);
                case ExecutionMode::OPENMP:
                    return strassen::openmp(A, B, config.optimization, config.num_threads);
                case ExecutionMode::MPI:
                    return strassen::mpi(A, B, config.optimization);
                case ExecutionMode::HYBRID:
                    return strassen::hybrid(A, B, config.optimization, config.num_threads);
            }
            break;

        case Algorithm::OPENBLAS:
            return openblas::multiply(A, B);
    }

    throw std::runtime_error("Invalid algorithm/mode combination");
}

// Parse command-line arguments into Config
// Returns true if arguments were parsed successfully, false if help was shown or error occurred
bool parse_arguments(int argc, char** argv, Config& config) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // Help
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return false;
        }
        // Algorithm
        else if (arg == "-a" || arg == "--algorithm") {
            if (i + 1 < argc) {
                config.algorithm = parse_algorithm(argv[++i]);
            } else {
                throw std::runtime_error("--algorithm requires an argument");
            }
        }
        // Execution mode
        else if (arg == "-m" || arg == "--mode") {
            if (i + 1 < argc) {
                config.mode = parse_execution_mode(argv[++i]);
            } else {
                throw std::runtime_error("--mode requires an argument");
            }
        }
        // Matrix size
        else if (arg == "-s" || arg == "--size") {
            if (i + 1 < argc) {
                config.matrix_size = std::atoi(argv[++i]);
                if (config.matrix_size <= 0) {
                    throw std::runtime_error("Matrix size must be positive");
                }
            } else {
                throw std::runtime_error("--size requires an argument");
            }
        }
        // Number of threads
        else if (arg == "-t" || arg == "--threads") {
            if (i + 1 < argc) {
                config.num_threads = std::atoi(argv[++i]);
                if (config.num_threads <= 0) {
                    throw std::runtime_error("Number of threads must be positive");
                }
            } else {
                throw std::runtime_error("--threads requires an argument");
            }
        }
        // Enable optimization
        else if (arg == "-o" || arg == "--optimize") {
            config.optimization.cache_friendly = true;
            config.optimization.use_blocking = true;
        }
        // Block size
        else if (arg == "-b" || arg == "--block-size") {
            if (i + 1 < argc) {
                config.optimization.block_size = std::atoi(argv[++i]);
                if (config.optimization.block_size <= 0) {
                    throw std::runtime_error("Block size must be positive");
                }
                config.optimization.cache_friendly = true;
                config.optimization.use_blocking = true;
            } else {
                throw std::runtime_error("--block-size requires an argument");
            }
        }
        // Input file
        else if (arg == "-i" || arg == "--input") {
            if (i + 1 < argc) {
                config.input_file = argv[++i];
                config.output_file = CsvIO::generate_output_filename(config.input_file);
            } else {
                throw std::runtime_error("--input requires an argument");
            }
        }
        // Validation
        else if (arg == "--validate") {
            config.validate_against_openblas = true;
        }
        // Verification mode
        else if (arg == "--verify") {
            config.verification_mode = true;
            // Default: verify all algorithms
            config.verify_algorithms = {Algorithm::NAIVE, Algorithm::STRASSEN, Algorithm::OPENBLAS};
        }
        // Unknown argument
        else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    // Set defaults for OpenMP/Hybrid modes if not specified
    if ((config.mode == ExecutionMode::OPENMP || config.mode == ExecutionMode::HYBRID) &&
        config.num_threads == 1) {
        config.num_threads = 4;  // Default to 4 threads
    }

    return true;
}

void print_results(const Config& config, int rank) {
    if (rank != 0) return;  // Only rank 0 prints

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "         Matrix Multiplication          \n";
    std::cout << "========================================\n";
    std::cout << "Algorithm:       " << algorithm_to_string(config.algorithm) << "\n";
    std::cout << "Execution Mode:  " << mode_to_string(config.mode) << "\n";

    if (config.mode == ExecutionMode::OPENMP || config.mode == ExecutionMode::HYBRID) {
        std::cout << "Threads:         " << config.num_threads << "\n";
    }

    std::cout << "Matrix Size:     " << config.matrix_size << "x" << config.matrix_size << "\n";

    if (config.optimization.cache_friendly) {
        std::cout << "Optimization:    Cache-friendly (block size: "
                  << config.optimization.block_size << ")\n";
    } else {
        std::cout << "Optimization:    None\n";
    }

    if (!config.input_file.empty()) {
        std::cout << "Input File:      " << config.input_file << "\n";
        std::cout << "Output File:     " << config.output_file << "\n";
    } else {
        std::cout << "Input:           Random matrices\n";
    }

    std::cout << "========================================\n";
    std::cout << "Execution Time:  " << std::fixed << std::setprecision(6)
              << config.execution_time << " seconds\n";
    std::cout << "========================================\n";
    std::cout << "\n";
}

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Config config;

    try {
        bool use_interactive = false;

        // Determine if we should use interactive mode or argument parsing
        if (argc > 1) {
            // Command-line arguments provided - parse them
            if (rank == 0) {
                if (!parse_arguments(argc, argv, config)) {
                    // Help was shown or parsing failed
                    MPI_Abort(MPI_COMM_WORLD, 0);
                    return 0;
                }
            }
        } else {
            // No arguments - try interactive mode
            if (rank == 0) {
                // Check if stdin is available (terminal)
                if (!isatty(STDIN_FILENO)) {
                    std::cerr << "Error: No command-line arguments provided and stdin is not a terminal.\n";
                    std::cerr << "       Interactive mode requires a terminal.\n\n";
                    std::cerr << "Usage: " << argv[0] << " [OPTIONS]\n";
                    std::cerr << "Run '" << argv[0] << " --help' for more information.\n\n";
                    std::cerr << "NOTE: When using MPI (mpirun), you must provide command-line arguments.\n";
                    MPI_Abort(MPI_COMM_WORLD, 1);
                    return 1;
                }

                // Run interactive CLI menu
                CliMenu menu;
                if (!menu.run(config)) {
                    std::cout << "Operation cancelled.\n";
                    MPI_Abort(MPI_COMM_WORLD, 0);
                    return 0;
                }
            }
            use_interactive = true;
        }

        // Broadcast configuration to all processes
        MPI_Bcast(&config.algorithm, sizeof(Algorithm), MPI_BYTE, 0, MPI_COMM_WORLD);
        MPI_Bcast(&config.mode, sizeof(ExecutionMode), MPI_BYTE, 0, MPI_COMM_WORLD);
        MPI_Bcast(&config.optimization, sizeof(OptimizationOptions), MPI_BYTE, 0, MPI_COMM_WORLD);
        MPI_Bcast(&config.num_threads, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&config.matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Broadcast input_file length and content
        int filename_len = config.input_file.length();
        MPI_Bcast(&filename_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (filename_len > 0) {
            if (rank != 0) {
                config.input_file.resize(filename_len);
            }
            MPI_Bcast(&config.input_file[0], filename_len, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        // Broadcast output_file length and content
        int output_filename_len = config.output_file.length();
        MPI_Bcast(&output_filename_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (output_filename_len > 0) {
            if (rank != 0) {
                config.output_file.resize(output_filename_len);
            }
            MPI_Bcast(&config.output_file[0], output_filename_len, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        // Load or generate matrices (rank 0 does this, then broadcasts)
        Matrix A(config.matrix_size);
        Matrix B(config.matrix_size);

        if (rank == 0) {
            if (!config.input_file.empty()) {
                std::cout << "Loading matrices from " << config.input_file << "...\n";
                // For simplicity, assume input file contains both matrices
                // In practice, you'd need two files or a specific format
                if (!CsvIO::read_matrix(config.input_file, A)) {
                    std::cerr << "Error: Failed to load matrix A\n";
                    MPI_Abort(MPI_COMM_WORLD, 1);
                    return 1;
                }
                // For now, use the same matrix as B or generate random B
                B = A;
            } else {
                std::cout << "Generating random matrices...\n";
                A.randomize(0.0, 10.0);
                B.randomize(0.0, 10.0);
            }
        }

        // Broadcast matrices to all processes
        MPI_Bcast(A.data(), config.matrix_size * config.matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(B.data(), config.matrix_size * config.matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if (config.verification_mode) {
            // Verification mode - only rank 0 runs this
            if (rank == 0) {
                verification::run_verification_suite(A, B, config, rank);
            }
        } else {
            // Normal execution mode
            if (rank == 0) {
                std::cout << "Computing matrix multiplication...\n";
            }

            Timer timer;
            timer.start();

            Matrix C = multiply(A, B, config);

            timer.stop();
            config.execution_time = timer.elapsed_seconds();

            // Only rank 0 handles validation and output
            if (rank == 0) {
                // Optional validation against OpenBLAS
                if (config.validate_against_openblas) {
                    std::cout << "\n";
                    bool valid = verification::validate_against_reference(
                        C, A, B, config.algorithm, config);
                    config.validation_performed = true;
                    config.validation_passed = valid;

                    if (!valid) {
                        std::cerr << "\nWARNING: Validation failed! Results differ from OpenBLAS reference.\n";
                    }
                }

                // Save result if output file specified
                if (!config.output_file.empty()) {
                    std::cout << "Saving result to " << config.output_file << "...\n";
                    if (!CsvIO::write_matrix(config.output_file, C)) {
                        std::cerr << "Warning: Failed to save output matrix\n";
                    }
                }

                // Print results
                print_results(config, rank);

                // Print validation status if performed
                if (config.validation_performed) {
                    std::cout << "Validation Status: ";
                    if (config.validation_passed) {
                        std::cout << "\033[32mPASSED\033[0m ✓\n";
                    } else {
                        std::cout << "\033[31mFAILED\033[0m ✗\n";
                    }
                    std::cout << "\n";
                }
            }
        }

    } catch (const std::exception& e) {
        if (rank == 0) {
            std::cerr << "Error: " << e.what() << "\n";
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    MPI_Finalize();
    return 0;
}
