#ifndef CLI_MENU_HPP
#define CLI_MENU_HPP

#include "config.hpp"
#include <string>
#include <vector>

namespace matmul {

class CliMenu {
public:
    CliMenu();
    ~CliMenu();

    // Run the interactive menu and fill the config
    // Returns true if user confirmed, false if cancelled
    bool run(Config& config);

private:
    // Menu screens
    Algorithm select_algorithm();
    ExecutionMode select_execution_mode();
    int select_num_threads();
    OptimizationOptions select_optimizations();
    int select_matrix_size();
    std::string select_input_file();

    // Verification mode screens
    bool select_verification_mode();
    std::vector<Algorithm> select_algorithms_to_verify();
    bool select_validation_option();

    // Generic menu helper
    template<typename T>
    T select_from_menu(const std::string& title,
                       const std::vector<std::string>& options,
                       const std::vector<T>& values);

    // Display helper
    void display_config_summary(const Config& config);
};

} // namespace matmul

#endif // CLI_MENU_HPP
