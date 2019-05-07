#ifndef DEFAULT_WINDOW_PROVIDER_HPP
#define DEFAULT_WINDOW_PROVIDER_HPP

#include"abstract_terminal_window.hpp"

class DefaultWindowProvider {
public:
    static AbstractTerminalWindow & getWindow();

private:
    DefaultWindowProvider();
};

#endif // DEFAULT_WINDOW_PROVIDER_HPP

