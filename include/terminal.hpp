#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <string>

namespace terminal {
    // Key codes for special keys
    enum class KeyCode {
        UNKNOWN,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        ENTER,
        ESCAPE,
        SPACE,
        BACKSPACE,
        TAB,
        CHAR  // Regular character
    };

    struct KeyPress {
        KeyCode code;
        char character;  // Only valid when code == KeyCode::CHAR
    };

    // Terminal state management
    void enable_raw_mode();
    void disable_raw_mode();
    bool is_raw_mode_enabled();

    // Keyboard input
    KeyPress read_key();
    bool kbhit();  // Check if key is available without blocking

    // Terminal information
    int get_width();
    int get_height();
    bool supports_ansi();

    // Cursor control
    void hide_cursor();
    void show_cursor();
    void move_cursor_up(int n = 1);
    void move_cursor_down(int n = 1);
    void move_to_column(int col);

    // Line control
    void clear_line();
    void clear_lines(int n);  // Clear n lines upward

    // RAII wrapper for raw mode
    class RawMode {
    public:
        RawMode();
        ~RawMode();
        RawMode(const RawMode&) = delete;
        RawMode& operator=(const RawMode&) = delete;
    };
}

#endif // TERMINAL_HPP
