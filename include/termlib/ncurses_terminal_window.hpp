#ifndef NCURSES_TERMINAL_WINDOW_HPP
#define NCURSES_TERMINAL_WINDOW_HPP

#include"abstract_terminal_window.hpp"
#include"ncurses_color_pair.hpp"
#include<ncurses.h>

class NcursesTerminalWindow : public AbstractTerminalWindow {
public:
    NcursesTerminalWindow() {
        pWin = initscr();

        setEchoing(false);

        initColors();
    }

    ~NcursesTerminalWindow() {
        endwin();
    }

    void setCursorPosition(Vec2i position) override {
        wmove(pWin, position.y, position.x);
    }

    Vec2i getCursorPosition() const override {
        Vec2i pos;
        getyx(pWin, pos.y, pos.x);
        return pos;
    }

    void put(char ch) override {
        waddch(pWin, ch);
    }

    void display() override {
        wrefresh(pWin);
    }

    std::optional<char> getChar(int timeoutMillis = -1) override {
        timeout(timeoutMillis);

        auto got = wgetch(pWin);
        if (got == ERR) {
            return {};
        } else {
            return char(got);
        }
    }

    void setTextStyle(TextStyle style) override {
        int pairID = NcursesColorPair{ style.getColor() }.getID();
        int attrMask = getNcursesAttrMask(style.attributes);
        attrset(COLOR_PAIR(pairID) | attrMask);
    }

    void setEchoing(bool e) override {
        echoing = e;
        if (echoing) {
            echo();
        } else {
            noecho();
        }
    }

    Vec2i getSize() const override {
        Vec2i size;
        getmaxyx(pWin, size.y, size.x);
        return size;
    }

    void clear(Color background = Color::Black) override {
        wclear(pWin);

        NcursesColorPair pair{ TerminalColor{ Color::Black, background } };
        wbkgd(pWin, COLOR_PAIR(pair.getID()));
    }

private:
    WINDOW * pWin;

    void initColors() {
        if (has_colors()) {
            start_color();

            initColorPairs();
        }
    }

    void initColorPairs() {
        int colors = 8;
        
        for (int fg = 0; fg < colors; ++fg) {
            for (int bg = 0; bg < colors; ++bg) {
                NcursesColorPair pair{ fg, bg };
                pair.init();
            }
        }
    }

    int getNcursesAttrMask(int mask) {
        int ncursMask = 0;

        if (mask & TextStyle::Bold)
            ncursMask |= A_BOLD;
        
        if (mask & TextStyle::Underlined)
            ncursMask |= A_UNDERLINE;

        return ncursMask;
    }
};

#endif // NCURSES_TERMINAL_WINDOW_HPP

