#ifndef TERMINAL_READER_HPP
#define TERMINAL_READER_HPP

#include"abstract_terminal_window.hpp"
#include"default_window_provider.hpp"
#include<string>

class TerminalReader {
public:
    TerminalReader()
        : win(DefaultWindowProvider::getWindow()) {}

    explicit TerminalReader(AbstractTerminalWindow & window): win(window) {}

    AbstractTerminalWindow & getTerminalWindow() const {
        return win;
    }

    char readChar() {
        return win.getChar().value();
    }

    std::optional<char> waitCharFor(int millis) {
        return win.getChar(millis);
    }

    std::string getLine() {
        std::string buffer;

        char got = readChar();
        while (got != '\n') {
            buffer += got;
            got = readChar();
        }

        return buffer;
    }

    void setEchoing(bool echo) {
        win.setEchoing(echo);
    }

    bool getEchoing() const {
        return win.getEchoing();
    }

private:
    AbstractTerminalWindow & win;
};

#endif // TERMINAL_READER_HPP
