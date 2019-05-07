#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#include<string_view>
#include<memory>
#include"vec2.hpp"
#include"abstract_terminal_window.hpp"
#include"default_window_provider.hpp"

class TerminalRenderer {
public:
    TerminalRenderer()
        : win(DefaultWindowProvider::getWindow())
        , currStyle() {}

    explicit TerminalRenderer(AbstractTerminalWindow & window)
        : win(window)
        , currStyle() {}

    AbstractTerminalWindow & getTerminalWindow() const {
        return win;
    }

    TerminalRenderer & setCursorPosition(Vec2i position) {
        win.setCursorPosition(position);
        return *this;
    }

    TerminalRenderer & moveCursor(Vec2i offset) {
        win.moveCursor(offset);
        return *this;
    }

    Vec2i getCursorPosition() const {
        return win.getCursorPosition();
    }

    TerminalRenderer & put(std::string_view str) {
        win.put(str);
        return *this;
    }

    TerminalRenderer & put(std::string_view ch, TextStyle style, bool replaceStyle = false) {
        auto oldStyle = currStyle;
        if (replaceStyle)
            setTextStyle(style);
        else
            addTextStyle(style);
        win.put(ch);
        setTextStyle(oldStyle);
        return *this;
    }

    TerminalRenderer & put(char ch) {
        win.put(ch);
        return *this;
    }

    TerminalRenderer & put(char ch, TextStyle style, bool replaceStyle = false) {
        auto oldStyle = currStyle;
        if (replaceStyle)
            setTextStyle(style);
        else
            addTextStyle(style);
        win.put(ch);
        setTextStyle(oldStyle);
        return *this;
    }

    TerminalRenderer & display() {
        win.display();
        return *this;
    }

    TerminalRenderer & setTextStyle(TextStyle style) {
        currStyle = style;
        win.setTextStyle(currStyle);
        return *this;
    }

    TerminalRenderer & addTextStyle(TextStyle style) {
        currStyle += style;
        win.setTextStyle(currStyle);
        return *this;
    }

    TerminalRenderer & clear(Color bgColor = Color::Black) {
        win.clear(bgColor);
        return *this;
    }

private:
    AbstractTerminalWindow & win;

    TextStyle currStyle;
};

#endif // RENDER_SYSTEM_HPP

