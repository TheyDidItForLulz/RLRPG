//
// Created by yuri12358 on 5/30/19.
//

#ifndef RLRPG_ITEM_LIST_FORMATTERS_HPP
#define RLRPG_ITEM_LIST_FORMATTERS_HPP

#include<item.hpp>

#include<cassert>
#include<fmt/core.h>

namespace formatters {
    struct LetterNumberingByIndex {
        char operator()(int itemIndex, Item const &) {
            assert(itemIndex >= 0);
            if (itemIndex < 26)
                return static_cast<char>('a' + itemIndex);
            itemIndex -= 26;
            if (itemIndex < 26)
                return static_cast<char>('A' + itemIndex);
            return '-';
        }
    };

    struct LetterNumberingByInventoryID {
        char operator()(int, Item const & item) {
            return item.inventorySymbol;
        }
    };

    struct DontMark {
        char operator()(int, Item const &) {
            return '-';
        }
    };

    struct MarkSelected {
        std::vector<bool> & selected;

        char operator()(int i, Item const &) {
            return selected.at(i) ? '+' : '-';
        }
    };

    struct WithoutEquippedStatus {
        std::string operator()(int, Item const &) {
            return "";
        }
    };

    struct WithEquippedStatus {
        Weapon * weapon = nullptr;
        Armor * armor = nullptr;

        std::string operator()(int, Item const & item) {
            if (&item == weapon)
                return " (being wielded)";
            else if (&item == armor)
                return " (being worn)";
            return "";
        }
    };

}

template<class NS, class MS, class ES>
std::string formatItem(int i, Item const & item,
                       NS numberingStrategy, MS markStrategy, ES equippedStrategy) {
    using fmt::format;
    std::string name = format(" {}", item.getName());

    std::string count;
    if (item.count > 1)
        count = format(" {}x", item.count);

    std::string modifier;
    if (item.showMdf)
        modifier = format(" {{{}}}", item.getMdf());

    return format("{} {}{}{}{}{}",
                  numberingStrategy(i, item),
                  markStrategy(i, item),
                  count,
                  name,
                  modifier,
                  equippedStrategy(i, item));
}

#endif //RLRPG_ITEM_LIST_FORMATTERS_HPP
