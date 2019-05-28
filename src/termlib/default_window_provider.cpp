#include<termlib/ncurses_terminal_window.hpp>
#include<termlib/default_window_provider.hpp>

AbstractTerminalWindow & DefaultWindowProvider::getWindow() {
    static NcursesTerminalWindow window;
    return window;
}

