#ifndef RLRPG_RENDER_DATA_HPP
#define RLRPG_RENDER_DATA_HPP

#include<termlib/terminal_text_style.hpp>

#include<tl/optional.hpp>

struct SymbolRenderData {
    TextStyle style;
    char symbol;

    SymbolRenderData(char symbol, TextStyle style = {})
        : symbol(symbol)
        , style(style) {}
};

struct CellRenderData {
    tl::optional<SymbolRenderData> level;
    tl::optional<SymbolRenderData> item;
    tl::optional<SymbolRenderData> unit;

    tl::optional<SymbolRenderData> get() const {
        return unit.disjunction(item).disjunction(level);
    }

    CellRenderData forCache() const {
        CellRenderData cached;
        cached.item = item;
        if (level.has_value() and level->symbol != '.') {
            cached.level = level;
        }
        return cached;
    }
};

#endif // RLRPG_RENDER_DATA_HPP

