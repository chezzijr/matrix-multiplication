#ifndef ANSI_CODES_HPP
#define ANSI_CODES_HPP

namespace ansi {
    // Text formatting
    constexpr auto RESET = "\033[0m";
    constexpr auto BOLD = "\033[1m";
    constexpr auto DIM = "\033[2m";
    constexpr auto ITALIC = "\033[3m";
    constexpr auto UNDERLINE = "\033[4m";
    constexpr auto BLINK = "\033[5m";
    constexpr auto REVERSE = "\033[7m";
    constexpr auto HIDDEN = "\033[8m";

    // Foreground colors
    constexpr auto BLACK = "\033[30m";
    constexpr auto RED = "\033[31m";
    constexpr auto GREEN = "\033[32m";
    constexpr auto YELLOW = "\033[33m";
    constexpr auto BLUE = "\033[34m";
    constexpr auto MAGENTA = "\033[35m";
    constexpr auto CYAN = "\033[36m";
    constexpr auto WHITE = "\033[37m";
    constexpr auto GRAY = "\033[90m";

    // Bright foreground colors
    constexpr auto BRIGHT_RED = "\033[91m";
    constexpr auto BRIGHT_GREEN = "\033[92m";
    constexpr auto BRIGHT_YELLOW = "\033[93m";
    constexpr auto BRIGHT_BLUE = "\033[94m";
    constexpr auto BRIGHT_MAGENTA = "\033[95m";
    constexpr auto BRIGHT_CYAN = "\033[96m";
    constexpr auto BRIGHT_WHITE = "\033[97m";

    // Background colors
    constexpr auto BG_BLACK = "\033[40m";
    constexpr auto BG_RED = "\033[41m";
    constexpr auto BG_GREEN = "\033[42m";
    constexpr auto BG_YELLOW = "\033[43m";
    constexpr auto BG_BLUE = "\033[44m";
    constexpr auto BG_MAGENTA = "\033[45m";
    constexpr auto BG_CYAN = "\033[46m";
    constexpr auto BG_WHITE = "\033[47m";

    // Cursor control
    constexpr auto CURSOR_UP = "\033[A";
    constexpr auto CURSOR_DOWN = "\033[B";
    constexpr auto CURSOR_FORWARD = "\033[C";
    constexpr auto CURSOR_BACK = "\033[D";
    constexpr auto CURSOR_SAVE = "\033[s";
    constexpr auto CURSOR_RESTORE = "\033[u";
    constexpr auto CURSOR_HIDE = "\033[?25l";
    constexpr auto CURSOR_SHOW = "\033[?25h";

    // Screen control
    constexpr auto CLEAR_LINE = "\033[2K";
    constexpr auto CLEAR_LINE_TO_END = "\033[K";
    constexpr auto CLEAR_LINE_TO_START = "\033[1K";
    constexpr auto CLEAR_SCREEN = "\033[2J";
    constexpr auto CLEAR_SCREEN_TO_END = "\033[J";
    constexpr auto CLEAR_SCREEN_TO_START = "\033[1J";

    // Helper functions for dynamic cursor movement
    inline std::string cursor_up(int n = 1) {
        return "\033[" + std::to_string(n) + "A";
    }

    inline std::string cursor_down(int n = 1) {
        return "\033[" + std::to_string(n) + "B";
    }

    inline std::string cursor_forward(int n = 1) {
        return "\033[" + std::to_string(n) + "C";
    }

    inline std::string cursor_back(int n = 1) {
        return "\033[" + std::to_string(n) + "D";
    }

    inline std::string move_to(int row, int col) {
        return "\033[" + std::to_string(row) + ";" + std::to_string(col) + "H";
    }

    inline std::string move_to_column(int col) {
        return "\033[" + std::to_string(col) + "G";
    }
}

#endif // ANSI_CODES_HPP
