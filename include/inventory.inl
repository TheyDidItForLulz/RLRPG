#include<items/item.hpp>

#include<stdexcept>

template<class ItemType>
AddStatus Inventory::add(Ptr<ItemType> && item) {
    if (not item)
        throw std::logic_error("Trying to add empty item to the inventory");

    if (item->isStackable) {
        for (auto & pair : items) {
            if (pair.second->id == item->id) {
                pair.second->count += item->count;
                return AddStatus::Stacked{ pair.first, item->count };
            }
        }
    }

    for (char i = 'a'; i <= 'z'; ++i) {
        if (items.count(i) == 0) {
            item->inventorySymbol = i;
            items[i] = std::move(item);
            return AddStatus::New{ i };
        }
    }
    for (char i = 'A'; i <= 'Z'; ++i) {
        if (items.count(i) == 0) {
            item->inventorySymbol = i;
            items[i] = std::move(item);
            return AddStatus::New{ i };
        }
    }
    return AddStatus::AddError{};
}

template<class ItemType>
AddStatus Inventory::add(Ptr<ItemType> && item, char at) {
    if (not item)
        throw std::logic_error("Trying to add empty item to the inventory");

    auto iter = items.find(at);
    if (iter == items.end()) {
        item->inventorySymbol = at;
        items[at] = std::move(item);
        return AddStatus::New{ at };
    }

    if (item->id != iter->second->id or not item->isStackable) {
        return AddStatus::AddError{};
    }

    iter->second->count += item->count;
    return AddStatus::Stacked{ at, item->count };
}

