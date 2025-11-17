#include "cli_menu.hpp"
#include "csv_io.hpp"
#include "cli_prompts.hpp"
#include "ansi_codes.hpp"
#include <iostream>
#include <sstream>

namespace matmul {

CliMenu::CliMenu() {}

CliMenu::~CliMenu() {}

bool CliMenu::run(Config& config) {
    // Step 1: Select normal vs verification mode
    config.verification_mode = select_verification_mode();

    if (config.verification_mode) {
        // Verification mode workflow
        // Step 2: Select algorithms to verify
        config.verify_algorithms = select_algorithms_to_verify();

        // Step 3: Select execution mode (restrict to Sequential/OpenMP)
        std::vector<std::string> options = {
            "Sequential",
            "OpenMP (Shared Memory)"
        };
        std::vector<ExecutionMode> values = {
            ExecutionMode::SEQUENTIAL,
            ExecutionMode::OPENMP
        };
        config.mode = select_from_menu("Select Execution Mode", options, values);

        // Step 4: Select number of threads (if using OpenMP)
        if (config.mode == ExecutionMode::OPENMP) {
            config.num_threads = select_num_threads();
        }

        // Step 5: Select optimizations
        config.optimization = select_optimizations();

        // Step 6: Select matrix size
        config.matrix_size = select_matrix_size();

        // Step 7: Show summary and confirm
        prompts::display_header("Verification Mode Configuration");
        std::cout << "  Mode: Verification\n";
        std::cout << "  Algorithms to verify:\n";
        for (const auto& algo : config.verify_algorithms) {
            std::cout << "    - " << algorithm_to_string(algo) << "\n";
        }
        std::cout << "  Execution Mode: " << mode_to_string(config.mode) << "\n";
        if (config.mode == ExecutionMode::OPENMP) {
            std::cout << "  Threads: " << config.num_threads << "\n";
        }
        std::cout << "  Matrix Size: " << config.matrix_size << "x" << config.matrix_size << "\n";
        if (config.optimization.cache_friendly) {
            std::cout << "  Optimization: Cache-friendly (block size: " << config.optimization.block_size << ")\n";
        } else {
            std::cout << "  Optimization: None\n";
        }

    } else {
        // Normal execution workflow
        // Step 2: Select algorithm
        config.algorithm = select_algorithm();

        // OpenBLAS is library-provided and doesn't need execution mode or optimization config
        if (config.algorithm == Algorithm::OPENBLAS) {
            // Set defaults for OpenBLAS
            config.mode = ExecutionMode::SEQUENTIAL;
            config.optimization = OptimizationOptions{};  // No custom optimizations

            // Step 3: Select matrix size
            config.matrix_size = select_matrix_size();

            // Step 4: Select input file (optional)
            config.input_file = select_input_file();
            if (!config.input_file.empty()) {
                config.output_file = CsvIO::generate_output_filename(config.input_file);
            }

            // Step 5: Show summary and confirm
            display_config_summary(config);
        } else {
            // Naive/Strassen workflow with full configuration
            // Step 3: Select execution mode
            config.mode = select_execution_mode();

            // Step 4: Select number of threads (if using OpenMP or Hybrid)
            if (config.mode == ExecutionMode::OPENMP || config.mode == ExecutionMode::HYBRID) {
                config.num_threads = select_num_threads();
            }

            // Step 5: Select optimizations
            config.optimization = select_optimizations();

            // Step 6: Select matrix size
            config.matrix_size = select_matrix_size();

            // Step 7: Select input file (optional)
            config.input_file = select_input_file();
            if (!config.input_file.empty()) {
                config.output_file = CsvIO::generate_output_filename(config.input_file);
            }

            // Step 8: Select validation option
            config.validate_against_openblas = select_validation_option();

            // Step 9: Show summary and confirm
            display_config_summary(config);
            if (config.validate_against_openblas) {
                prompts::display_info("Validation: Will validate against OpenBLAS");
            }
        }
    }

    return prompts::confirm("\nProceed with this configuration?", true);
}

Algorithm CliMenu::select_algorithm() {
    std::vector<std::string> options = {
        "Naive Matrix Multiplication",
        "Strassen Algorithm",
        "OpenBLAS (Reference)"
    };
    std::vector<Algorithm> values = {
        Algorithm::NAIVE,
        Algorithm::STRASSEN,
        Algorithm::OPENBLAS
    };

    return select_from_menu("Select Algorithm", options, values);
}

ExecutionMode CliMenu::select_execution_mode() {
    std::vector<std::string> options = {
        "Sequential",
        "OpenMP (Shared Memory)",
        "MPI (Distributed Memory)",
        "Hybrid (MPI + OpenMP)"
    };
    std::vector<ExecutionMode> values = {
        ExecutionMode::SEQUENTIAL,
        ExecutionMode::OPENMP,
        ExecutionMode::MPI,
        ExecutionMode::HYBRID
    };

    return select_from_menu("Select Execution Mode", options, values);
}

int CliMenu::select_num_threads() {
    return prompts::number_input("Enter number of threads", 4, 1, 256);
}

OptimizationOptions CliMenu::select_optimizations() {
    OptimizationOptions opt;

    std::vector<std::string> options = {
        "No optimizations",
        "Cache-friendly (blocking)",
        "Custom block size"
    };
    std::vector<int> values = {0, 1, 2};

    int choice = select_from_menu("Select Optimization", options, values);

    if (choice == 1) {
        opt.cache_friendly = true;
        opt.use_blocking = true;
        opt.block_size = 64;  // Default block size
    } else if (choice == 2) {
        opt.cache_friendly = true;
        opt.use_blocking = true;
        opt.block_size = prompts::number_input("Enter block size", 64, 8, 512);
    }

    return opt;
}

int CliMenu::select_matrix_size() {
    std::vector<std::string> options = {
        "100x100 (Small)",
        "1000x1000 (Medium)",
        "2000x2000 (Large)",
        "5000x5000 (Very Large)",
        "10000x10000 (Huge)",
        "Custom size"
    };
    std::vector<int> values = {100, 1000, 2000, 5000, 10000, -1};

    int size = select_from_menu("Select Matrix Size", options, values);

    if (size == -1) {
        size = prompts::number_input("Enter matrix size (NxN)", 100, 10, 20000);
    }

    return size;
}

std::string CliMenu::select_input_file() {
    std::vector<std::string> options = {
        "Generate random matrices",
        "Load from CSV file"
    };
    std::vector<int> values = {1, 2};

    int choice = select_from_menu("Input File Selection", options, values);

    std::string filename;
    if (choice == 2) {
        filename = prompts::text_input("Enter CSV filename", "");
    }

    return filename;
}

template<typename T>
T CliMenu::select_from_menu(const std::string& title,
                            const std::vector<std::string>& options,
                            const std::vector<T>& values) {
    int selected = prompts::select_option(title, options);

    if (selected == -1) {
        // Cancelled or error - return first option as default
        return values[0];
    }

    return values[selected];
}

void CliMenu::display_config_summary(const Config& config) {
    prompts::display_header("Configuration Summary");

    std::cout << "  Algorithm: " << algorithm_to_string(config.algorithm) << "\n";
    std::cout << "  Execution Mode: " << mode_to_string(config.mode) << "\n";

    if (config.mode == ExecutionMode::OPENMP || config.mode == ExecutionMode::HYBRID) {
        std::cout << "  Number of Threads: " << config.num_threads << "\n";
    }

    std::cout << "  Matrix Size: " << config.matrix_size << "x" << config.matrix_size << "\n";

    if (config.optimization.cache_friendly) {
        std::cout << "  Optimization: Cache-friendly (block size: " << config.optimization.block_size << ")\n";
    } else {
        std::cout << "  Optimization: None\n";
    }

    if (config.input_file.empty()) {
        std::cout << "  Input: Random matrices\n";
    } else {
        std::cout << "  Input File: " << config.input_file << "\n";
        std::cout << "  Output File: " << config.output_file << "\n";
    }
}

bool CliMenu::select_verification_mode() {
    std::vector<std::string> options = {
        "Normal Execution",
        "Verification Mode"
    };
    std::vector<bool> values = {false, true};

    return select_from_menu("Execution Mode", options, values);
}

std::vector<Algorithm> CliMenu::select_algorithms_to_verify() {
    std::vector<Algorithm> selected;

    std::vector<std::string> options = {
        "Naive",
        "Strassen",
        "OpenBLAS"
    };
    std::vector<Algorithm> values = {
        Algorithm::NAIVE,
        Algorithm::STRASSEN,
        Algorithm::OPENBLAS
    };

    prompts::display_info("Select at least 2 algorithms to compare");
    std::vector<int> selected_indices = prompts::multi_select("Select Algorithms to Verify", options);

    // Build result vector
    for (int idx : selected_indices) {
        selected.push_back(values[idx]);
    }

    // Ensure at least 2 selected
    if (selected.size() < 2) {
        prompts::display_error("Error: Select at least 2 algorithms!");
        return select_algorithms_to_verify();  // Retry
    }

    return selected;
}

bool CliMenu::select_validation_option() {
    std::vector<std::string> options = {
        "No validation",
        "Validate against OpenBLAS"
    };
    std::vector<bool> values = {false, true};

    return select_from_menu("Validation Option", options, values);
}

} // namespace matmul
