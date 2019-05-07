#ifndef TERMINAL_COLOR_HPP
#define TERMINAL_COLOR_HPP

enum class Color {
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};

struct TerminalColor {
    Color fg, bg;

    TerminalColor(Color foreground = Color::White, Color background = Color::Black)
        : fg(foreground)
        , bg(background) {}
};

#endif // TERMINAL_COLOR_HPP

