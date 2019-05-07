#ifndef TERMINAL_TEXT_STYLE_HPP
#define TERMINAL_TEXT_STYLE_HPP

#include"terminal_color.hpp"
#include<optional>

struct TextStyle {
    enum TextAttribute {
        None = 0,
        Bold = 1 << 0,
        Underlined = 1 << 1
    };

    int attributes;

    TextStyle(TerminalColor color)
        : optcolor(color)
        , attributes() {}

    TextStyle(int attrMask = 0, std::optional<TerminalColor> color = {})
        : attributes(attrMask)
        , optcolor(color) {}

    TextStyle & operator +=(TextStyle other) {
        if (other.optcolor)
            *this += other.optcolor.value();
        *this += other.attributes;
        return *this;
    }

    TextStyle & operator +=(int attrs) {
        attributes |= attrs;
        return *this;
    }

    TextStyle & operator +=(TerminalColor color) {
        optcolor = color;
        return *this;
    }

    TerminalColor getColor() const {
        return optcolor.value_or(TerminalColor{});
    }

private:
    std::optional<TerminalColor> optcolor;
};

#endif // TERMINAL_TEXT_STYLE_HPP
