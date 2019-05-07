#ifndef NCURSES_COLORS_HPP
#define NCURSES_COLORS_HPP

#include"terminal_color.hpp"
#include<ncurses.h>

struct NcursesColorPair {
    int fg, bg;

    NcursesColorPair(TerminalColor termColor)
        : fg(colorToNcurses(termColor.fg))
        , bg(colorToNcurses(termColor.bg)) {}

    NcursesColorPair(int fg, int bg)
        : fg(fg)
        , bg(bg) {}

    int getID() const {
        return fg + bg * 8;
    }

    void init() const {
        init_pair(getID(), fg, bg);
    }

    static int colorToNcurses(Color color) {
        switch (color) {
            case Color::Black:  return COLOR_BLACK;
            case Color::Red:    return COLOR_RED;
            case Color::Green:  return COLOR_GREEN;
            case Color::Blue:   return COLOR_BLUE;
            case Color::Yellow: return COLOR_YELLOW;
            case Color::Magenta:return COLOR_MAGENTA;
            case Color::Cyan:   return COLOR_CYAN;
            case Color::White:  return COLOR_WHITE;
        }
        return 0;
    }
};

#endif // NCURSES_COLORS_HPP

