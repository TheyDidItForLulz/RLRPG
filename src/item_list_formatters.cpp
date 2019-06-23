//
// Created by yuri12358 on 5/30/19.
//

#include<item_list_formatters.hpp>
#include <item_list_formatters.hpp>


using namespace formatters;

std::string FromInventory::operator()(int, const Item & item) const {
    std::string id = format("{} -", item.inventorySymbol);
    std::string name = format(" {}", item.getName());

    std::string count;
    if (item.count > 1)
        count = format(" {}x", item.count);

    std::string modifier;
    if (item.showMdf)
        modifier = format(" {{{}}}", item.getMdf());

    std::string equipped;
    if (&item == weapon)
        equipped = " (being wielded)";
    else if (&item == armor)
        equipped = " (being worn)";

    return format("{}{}{}{}{}", id, count, name, modifier, equipped);
}

std::string FromList::operator()(int i, const Item & item) const {
    char charID = char(i < 26 ? 'a' + i : 'A' + (i - 26));

    std::string id = format("{} -", charID);
    std::string name = format(" {}", item.getName());

    std::string count;
    if (item.count > 1)
        count = format(" {}x", item.count);

    std::string modifier;
    if (item.showMdf)
        modifier = format(" {{{}}}", item.getMdf());

    return format("{}{}{}{}", id, name, count, modifier);
}

std::string SelectMultipleFromInventory::operator()(int index, const Item & item) const {
    char selectedMark = selected.at(index) ? '+' : '-';
    std::string id = format("{} {}", item.inventorySymbol, selectedMark);
    std::string name = format(" {}", item.getName());

    std::string count;
    if (item.count > 1)
        count = format(" {}x", item.count);

    std::string modifier;
    if (item.showMdf)
        modifier = format(" {{{}}}", item.getMdf());

    std::string equipped;
    if (&item == weapon)
        equipped = " (being wielded)";
    else if (&item == armor)
        equipped = " (being worn)";

    return format("{}{}{}{}{}", id, count, name, modifier, equipped);
}

std::string SelectMultipleFromList::operator()(int i, const Item & item) const {
    char charID = char(i < 26 ? 'a' + i : 'A' + (i - 26));
    char selectedMark = selected.at(i) ? '+' : '-';

    std::string id = format("{} {}", charID, selectedMark);
    std::string name = format(" {}", item.getName());

    std::string count;
    if (item.count > 1)
        count = format(" {}x", item.count);

    std::string modifier;
    if (item.showMdf)
        modifier = format(" {{{}}}", item.getMdf());

    return format("{}{}{}{}", id, name, count, modifier);
}
