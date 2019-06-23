//
// Created by yuri12358 on 5/30/19.
//

#ifndef RLRPG_ITEM_LIST_FORMATTERS_HPP
#define RLRPG_ITEM_LIST_FORMATTERS_HPP

#include<item.hpp>

#include<fmt/core.h>

using fmt::format;

namespace formatters {
    struct FromInventory {
        Weapon * weapon;
        Armor * armor;

        FromInventory(Weapon * weapon, Armor * armor)
            : weapon (weapon), armor(armor) {}

        std::string operator()(int index, const Item & item) const;
    };

    struct SelectMultipleFromInventory {
        const std::vector<bool> & selected;
        Weapon * weapon;
        Armor * armor;

        SelectMultipleFromInventory(const std::vector<bool> & selected, Weapon * weapon, Armor * armor)
        : weapon (weapon), armor(armor), selected(selected) {}

        std::string operator()(int index, const Item & item) const;
    };

    struct FromList {
        std::string operator()(int index, const Item & item) const;
    };

    struct SelectMultipleFromList {
        const std::vector<bool> & selected;

        explicit SelectMultipleFromList(const std::vector<bool> & selected): selected(selected) {}

        std::string operator()(int index, const Item & item) const;
    };
}

#endif //RLRPG_ITEM_LIST_FORMATTERS_HPP
