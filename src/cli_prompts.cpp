#include "cli_prompts.hpp"
#include "terminal.hpp"
#include "ansi_codes.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>

namespace prompts {
    namespace {
        // Helper to render options list
        void render_options(
            const std::vector<std::string>& options,
            int selected_index,
            const std::string& selected_prefix,
            const std::string& unselected_prefix,
            const std::vector<bool>& checked = {}
        ) {
            for (size_t i = 0; i < options.size(); ++i) {
                const bool is_selected = (static_cast<int>(i) == selected_index);
                const std::string prefix = is_selected ? selected_prefix : unselected_prefix;

                std::cout << (is_selected ? ansi::CYAN : "")
                          << (is_selected ? ansi::BOLD : "");

                // Show checkbox if in multi-select mode
                if (!checked.empty()) {
                    std::cout << prefix;
                    std::cout << (checked[i] ? "[✓] " : "[ ] ");
                } else {
                    std::cout << prefix;
                }

                std::cout << options[i]
                          << (is_selected ? ansi::RESET : "")
                          << "\n";
            }
        }

        // Helper to clear rendered options
        void clear_rendered_options(int num_options, int extra_lines = 0) {
            terminal::clear_lines(num_options + extra_lines);
        }
    }

    // Template implementation for select_option
    template<typename T>
    int select_option(
        const std::string& title,
        const std::vector<T>& options,
        const std::function<std::string(const T&)>& display_fn,
        int initial_selection,
        const PromptConfig& config
    ) {
        if (options.empty()) {
            return -1;
        }

        // Convert to strings
        std::vector<std::string> option_strings;
        option_strings.reserve(options.size());
        for (const auto& opt : options) {
            option_strings.push_back(display_fn(opt));
        }

        // Use the string version
        return select_option(title, option_strings, initial_selection, config);
    }

    // String version implementation
    int select_option(
        const std::string& title,
        const std::vector<std::string>& options,
        int initial_selection,
        const PromptConfig& config
    ) {
        if (options.empty()) {
            return -1;
        }

        int selected = std::clamp(initial_selection, 0, static_cast<int>(options.size()) - 1);

        // Display title
        std::cout << "\n" << ansi::BOLD << ansi::CYAN << title << ansi::RESET << "\n";

        terminal::RawMode raw;
        terminal::hide_cursor();

        bool first_render = true;
        bool done = false;

        while (!done) {
            // Clear previous render (except on first iteration)
            if (!first_render) {
                int lines_to_clear = options.size();
                if (config.show_help) lines_to_clear += 2;  // +1 for empty line, +1 for help text
                clear_rendered_options(lines_to_clear, 0);
            }
            first_render = false;

            // Render options
            render_options(options, selected, config.prefix, config.unselected_prefix);

            // Show help text
            if (config.show_help) {
                std::cout << ansi::DIM << "\nUse ↑/↓ arrows to navigate, Enter to select, ESC to cancel" << ansi::RESET;
            }
            std::cout << std::flush;

            // Read key
            auto key = terminal::read_key();

            switch (key.code) {
                case terminal::KeyCode::UP:
                    selected = (selected - 1 + options.size()) % options.size();
                    break;
                case terminal::KeyCode::DOWN:
                    selected = (selected + 1) % options.size();
                    break;
                case terminal::KeyCode::ENTER:
                    done = true;
                    break;
                case terminal::KeyCode::ESCAPE:
                    terminal::show_cursor();
                    return -1;  // Cancelled
                default:
                    break;
            }
        }

        // Clear the menu before showing final result
        int lines_to_clear = options.size();
        if (config.show_help) lines_to_clear += 2;  // +1 for empty line, +1 for help text
        clear_rendered_options(lines_to_clear, 0);

        terminal::show_cursor();

        if (config.clear_on_select) {
            // Clear title area (current line + title line + empty line before title)
            terminal::clear_line();  // Clear current position
            terminal::move_cursor_up(1);  // Move to title line
            terminal::clear_line();  // Clear title
            terminal::move_cursor_up(1);  // Move to empty line before title
            terminal::clear_line();  // Clear empty line
        } else {
            // Show final selection
            std::cout << ansi::GREEN << config.prefix << options[selected] << ansi::RESET << "\n";
        }

        return selected;
    }

    // Multi-selection implementation
    std::vector<int> multi_select(
        const std::string& title,
        const std::vector<std::string>& options,
        const std::vector<int>& initial_selection,
        const PromptConfig& config
    ) {
        if (options.empty()) {
            return {};
        }

        int selected = 0;
        std::vector<bool> checked(options.size(), false);

        // Set initial selections
        for (int idx : initial_selection) {
            if (idx >= 0 && idx < static_cast<int>(options.size())) {
                checked[idx] = true;
            }
        }

        // Display title
        std::cout << "\n" << ansi::BOLD << ansi::CYAN << title << ansi::RESET << "\n";

        terminal::RawMode raw;
        terminal::hide_cursor();

        bool first_render = true;
        bool done = false;

        while (!done) {
            // Clear previous render (except on first iteration)
            if (!first_render) {
                clear_rendered_options(options.size() + 2, 0);  // +1 for empty line, +1 for help text
            }
            first_render = false;

            // Render options with checkboxes
            render_options(options, selected, config.prefix, config.unselected_prefix, checked);

            // Show help text
            std::cout << ansi::DIM << "\nUse ↑/↓ to navigate, SPACE to toggle, Enter to confirm, ESC to cancel" << ansi::RESET;
            std::cout << std::flush;

            // Read key
            auto key = terminal::read_key();

            switch (key.code) {
                case terminal::KeyCode::UP:
                    selected = (selected - 1 + options.size()) % options.size();
                    break;
                case terminal::KeyCode::DOWN:
                    selected = (selected + 1) % options.size();
                    break;
                case terminal::KeyCode::SPACE:
                    checked[selected] = !checked[selected];
                    break;
                case terminal::KeyCode::ENTER:
                    done = true;
                    break;
                case terminal::KeyCode::ESCAPE:
                    terminal::show_cursor();
                    return {};  // Cancelled - return empty
                default:
                    break;
            }
        }

        // Clear the menu before showing final result
        clear_rendered_options(options.size() + 2, 0);  // +1 for empty line, +1 for help text

        terminal::show_cursor();

        // Collect selected indices
        std::vector<int> result;
        for (size_t i = 0; i < checked.size(); ++i) {
            if (checked[i]) {
                result.push_back(static_cast<int>(i));
            }
        }

        if (config.clear_on_select) {
            // Clear title area (current line + title line + empty line before title)
            terminal::clear_line();  // Clear current position
            terminal::move_cursor_up(1);  // Move to title line
            terminal::clear_line();  // Clear title
            terminal::move_cursor_up(1);  // Move to empty line before title
            terminal::clear_line();  // Clear empty line
        } else {
            // Show selected items
            std::cout << ansi::GREEN << "Selected: ";
            for (size_t i = 0; i < result.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << options[result[i]];
            }
            std::cout << ansi::RESET << "\n";
        }

        return result;
    }

    // Text input implementation
    std::string text_input(
        const std::string& prompt,
        const std::string& default_value,
        const std::function<bool(const std::string&)>& validator
    ) {
        std::string input;
        bool valid = false;

        while (!valid) {
            std::cout << ansi::CYAN << prompt;
            if (!default_value.empty()) {
                std::cout << " [" << default_value << "]";
            }
            std::cout << ": " << ansi::RESET;

            std::getline(std::cin, input);

            // Use default if empty
            if (input.empty() && !default_value.empty()) {
                input = default_value;
            }

            // Validate if validator provided
            if (validator) {
                if (validator(input)) {
                    valid = true;
                } else {
                    display_error("Invalid input. Please try again.");
                }
            } else {
                valid = true;
            }
        }

        return input;
    }

    // Number input implementation
    int number_input(
        const std::string& prompt,
        int default_value,
        std::optional<int> min_value,
        std::optional<int> max_value
    ) {
        auto validator = [&](const std::string& input) -> bool {
            try {
                int value = std::stoi(input);
                if (min_value && value < *min_value) {
                    std::cout << ansi::RED << "Value must be at least " << *min_value << ansi::RESET << "\n";
                    return false;
                }
                if (max_value && value > *max_value) {
                    std::cout << ansi::RED << "Value must be at most " << *max_value << ansi::RESET << "\n";
                    return false;
                }
                return true;
            } catch (...) {
                std::cout << ansi::RED << "Please enter a valid number" << ansi::RESET << "\n";
                return false;
            }
        };

        std::string input = text_input(prompt, std::to_string(default_value), validator);
        return std::stoi(input);
    }

    // Confirmation implementation
    bool confirm(
        const std::string& prompt,
        bool default_value
    ) {
        const std::string default_text = default_value ? "Y/n" : "y/N";
        std::cout << ansi::CYAN << prompt << " [" << default_text << "]: " << ansi::RESET;

        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) {
            return default_value;
        }

        char first = std::tolower(input[0]);
        return first == 'y';
    }

    // Display functions
    void display_header(const std::string& title) {
        std::cout << "\n" << ansi::BOLD << ansi::BRIGHT_CYAN
                  << "=== " << title << " ==="
                  << ansi::RESET << "\n\n";
    }

    void display_info(const std::string& message) {
        std::cout << ansi::CYAN << "ℹ " << message << ansi::RESET << "\n";
    }

    void display_success(const std::string& message) {
        std::cout << ansi::GREEN << "✓ " << message << ansi::RESET << "\n";
    }

    void display_error(const std::string& message) {
        std::cout << ansi::RED << "✗ " << message << ansi::RESET << "\n";
    }

    void display_warning(const std::string& message) {
        std::cout << ansi::YELLOW << "⚠ " << message << ansi::RESET << "\n";
    }

    // Explicit template instantiations for common types
    template int select_option<std::string>(
        const std::string&,
        const std::vector<std::string>&,
        const std::function<std::string(const std::string&)>&,
        int,
        const PromptConfig&
    );
}
