#include "terminal.hpp"
#include "ansi_codes.hpp"
#include <iostream>
#include <cstdio>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <sys/select.h>
#endif

namespace terminal {
    namespace {
        // Static state
        bool raw_mode_enabled = false;

        #ifndef _WIN32
        struct termios original_termios;
        #endif
    }

    // Terminal state management
    void enable_raw_mode() {
        if (raw_mode_enabled) return;

        #ifdef _WIN32
            // Windows implementation
            HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
            DWORD mode;
            GetConsoleMode(hStdin, &mode);
            SetConsoleMode(hStdin, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
        #else
            // POSIX implementation
            tcgetattr(STDIN_FILENO, &original_termios);

            struct termios raw = original_termios;
            raw.c_lflag &= ~(ECHO | ICANON);  // Disable echo and canonical mode
            raw.c_cc[VMIN] = 0;   // Non-blocking read
            raw.c_cc[VTIME] = 1;  // 100ms timeout

            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        #endif

        raw_mode_enabled = true;
    }

    void disable_raw_mode() {
        if (!raw_mode_enabled) return;

        #ifdef _WIN32
            HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
            DWORD mode;
            GetConsoleMode(hStdin, &mode);
            SetConsoleMode(hStdin, mode | ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
        #else
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
        #endif

        raw_mode_enabled = false;
    }

    bool is_raw_mode_enabled() {
        return raw_mode_enabled;
    }

    // Keyboard input
    KeyPress read_key() {
        if (!raw_mode_enabled) {
            enable_raw_mode();
        }

        #ifdef _WIN32
            if (_kbhit()) {
                int ch = _getch();

                if (ch == 0 || ch == 224) {  // Special key prefix
                    ch = _getch();
                    switch (ch) {
                        case 72: return {KeyCode::UP, '\0'};
                        case 80: return {KeyCode::DOWN, '\0'};
                        case 75: return {KeyCode::LEFT, '\0'};
                        case 77: return {KeyCode::RIGHT, '\0'};
                        default: return {KeyCode::UNKNOWN, '\0'};
                    }
                }

                switch (ch) {
                    case 13: return {KeyCode::ENTER, '\0'};
                    case 27: return {KeyCode::ESCAPE, '\0'};
                    case 32: return {KeyCode::SPACE, '\0'};
                    case 8: return {KeyCode::BACKSPACE, '\0'};
                    case 9: return {KeyCode::TAB, '\0'};
                    default: return {KeyCode::CHAR, static_cast<char>(ch)};
                }
            }
            return {KeyCode::UNKNOWN, '\0'};
        #else
            char c;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                if (c == '\033') {  // Escape sequence
                    char seq[3];

                    // Try to read next two characters
                    if (read(STDIN_FILENO, &seq[0], 1) != 1) return {KeyCode::ESCAPE, '\0'};
                    if (read(STDIN_FILENO, &seq[1], 1) != 1) return {KeyCode::ESCAPE, '\0'};

                    if (seq[0] == '[') {
                        switch (seq[1]) {
                            case 'A': return {KeyCode::UP, '\0'};
                            case 'B': return {KeyCode::DOWN, '\0'};
                            case 'C': return {KeyCode::RIGHT, '\0'};
                            case 'D': return {KeyCode::LEFT, '\0'};
                            default: return {KeyCode::UNKNOWN, '\0'};
                        }
                    }
                    return {KeyCode::ESCAPE, '\0'};
                }

                switch (c) {
                    case '\n':
                    case '\r': return {KeyCode::ENTER, '\0'};
                    case ' ': return {KeyCode::SPACE, '\0'};
                    case 127:  // DEL key (backspace on some systems)
                    case '\b': return {KeyCode::BACKSPACE, '\0'};
                    case '\t': return {KeyCode::TAB, '\0'};
                    default: return {KeyCode::CHAR, c};
                }
            }
            return {KeyCode::UNKNOWN, '\0'};
        #endif
    }

    bool kbhit() {
        #ifdef _WIN32
            return _kbhit() != 0;
        #else
            struct timeval tv = {0, 0};
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
        #endif
    }

    // Terminal information
    int get_width() {
        #ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
            return csbi.srWindow.Right - csbi.srWindow.Left + 1;
        #else
            struct winsize w;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
                return 80;  // Default fallback
            }
            return w.ws_col;
        #endif
    }

    int get_height() {
        #ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
            return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        #else
            struct winsize w;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
                return 24;  // Default fallback
            }
            return w.ws_row;
        #endif
    }

    bool supports_ansi() {
        #ifdef _WIN32
            // Windows 10+ supports ANSI in modern terminals
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode;
            if (GetConsoleMode(hOut, &mode)) {
                mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                return SetConsoleMode(hOut, mode);
            }
            return false;
        #else
            // Most POSIX systems support ANSI
            const char* term = getenv("TERM");
            return term != nullptr && std::string(term) != "dumb";
        #endif
    }

    // Cursor control
    void hide_cursor() {
        std::cout << ansi::CURSOR_HIDE << std::flush;
    }

    void show_cursor() {
        std::cout << ansi::CURSOR_SHOW << std::flush;
    }

    void move_cursor_up(int n) {
        std::cout << ansi::cursor_up(n) << std::flush;
    }

    void move_cursor_down(int n) {
        std::cout << ansi::cursor_down(n) << std::flush;
    }

    void move_to_column(int col) {
        std::cout << ansi::move_to_column(col) << std::flush;
    }

    // Line control
    void clear_line() {
        std::cout << '\r' << ansi::CLEAR_LINE << std::flush;
    }

    void clear_lines(int n) {
        for (int i = 0; i < n; ++i) {
            if (i > 0) move_cursor_up(1);
            clear_line();
        }
    }

    // RAII wrapper
    RawMode::RawMode() {
        enable_raw_mode();
    }

    RawMode::~RawMode() {
        disable_raw_mode();
    }
}
