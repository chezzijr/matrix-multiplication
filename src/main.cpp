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
        // Only rank 0 runs the CLI menu
        if (rank == 0) {
            CliMenu menu;
            if (!menu.run(config)) {
                std::cout << "Operation cancelled.\n";
                MPI_Abort(MPI_COMM_WORLD, 0);
                return 0;
            }
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
