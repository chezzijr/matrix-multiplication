#ifndef CLI_PROMPTS_HPP
#define CLI_PROMPTS_HPP

#include <string>
#include <vector>
#include <optional>
#include <functional>

namespace prompts {
    // Configuration options for prompts
    struct PromptConfig {
        bool show_help = true;           // Show help text (e.g., "Use arrow keys...")
        bool clear_on_select = true;     // Clear the prompt after selection
        std::string prefix = "> ";       // Prefix for selected item
        std::string unselected_prefix = "  ";  // Prefix for unselected items
    };

    // Single selection from a list using arrow keys
    // Returns the index of the selected item
    template<typename T>
    int select_option(
        const std::string& title,
        const std::vector<T>& options,
        const std::function<std::string(const T&)>& display_fn,
        int initial_selection = 0,
        const PromptConfig& config = PromptConfig{}
    );

    // Convenience overload for string vectors
    int select_option(
        const std::string& title,
        const std::vector<std::string>& options,
        int initial_selection = 0,
        const PromptConfig& config = PromptConfig{}
    );

    // Multi-selection from a list (checkbox style)
    // Returns indices of selected items
    std::vector<int> multi_select(
        const std::string& title,
        const std::vector<std::string>& options,
        const std::vector<int>& initial_selection = {},
        const PromptConfig& config = PromptConfig{}
    );

    // Text input with optional validation
    std::string text_input(
        const std::string& prompt,
        const std::string& default_value = "",
        const std::function<bool(const std::string&)>& validator = nullptr
    );

    // Integer input with optional range validation
    int number_input(
        const std::string& prompt,
        int default_value = 0,
        std::optional<int> min_value = std::nullopt,
        std::optional<int> max_value = std::nullopt
    );

    // Yes/No confirmation
    bool confirm(
        const std::string& prompt,
        bool default_value = true
    );

    // Display a header/title
    void display_header(const std::string& title);

    // Display an informational message
    void display_info(const std::string& message);

    // Display a success message
    void display_success(const std::string& message);

    // Display an error message
    void display_error(const std::string& message);

    // Display a warning message
    void display_warning(const std::string& message);
}

#endif // CLI_PROMPTS_HPP
