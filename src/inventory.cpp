#include<inventory.hpp>
#include<stdexcept>
#include<algorithm>

Inventory::Inventory(const Inventory & other) {
    for (const auto & item : other.items) {
        items.emplace(item.first, item.second->clone());
    }
}

Inventory & Inventory::operator=(const Inventory & other) {
    items.clear();
    for (const auto & item : other.items) {
        items.emplace(item.first, item.second->clone());
    }
    return *this;
}

AddStatus Inventory::add(Item::Ptr item) {
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
    return AddStatus::FullInvError{ std::move(item) };
}

Item::Ptr Inventory::remove(char id) {
    auto iter = items.find(id);
    if (iter == items.end()) {
        throw std::logic_error("Trying to remove an item that doesn't exist");
    }
    auto item = std::move(iter->second);
    items.erase(iter);
    item->inventorySymbol = 0;
    return item;
}

int Inventory::size() const {
    return items.size();
}

bool Inventory::isFull() const {
    return size() == 52;
}

bool Inventory::isEmpty() const {
    return items.empty();
}

bool Inventory::hasID(char id) const {
    return items.count(id) == 1;
}

Item & Inventory::operator [](char id) {
    auto iter = find(id);
    if (iter == end())
        throw std::logic_error("Trying to get an item that doesn't exist");
    //auto & t = *iter->second;
    return *iter->second;
}

const Item & Inventory::operator [](char id) const {
    auto iter = find(id);
    if (iter == cend())
        throw std::logic_error("Trying to get an item that doesn't exist");
    return *iter->second;
}

InventoryIterator Inventory::find(char id) {
    return InventoryIterator(items.find(id));
}

ConstInventoryIterator Inventory::find(char id) const {
    return ConstInventoryIterator(items.find(id));
}

InventoryIterator Inventory::begin() {
    return InventoryIterator(items.begin());
}

ConstInventoryIterator Inventory::begin() const {
    return ConstInventoryIterator(items.begin());
}

ConstInventoryIterator Inventory::cbegin() const {
    return ConstInventoryIterator(items.begin());
}

InventoryIterator Inventory::end() {
    return InventoryIterator(items.end());
}

ConstInventoryIterator Inventory::end() const {
    return ConstInventoryIterator(items.end());
}

ConstInventoryIterator Inventory::cend() const {
    return ConstInventoryIterator(items.end());
}

InventoryIterator Inventory::erase(ConstInventoryIterator iter) {
    return InventoryIterator(items.erase(iter.iter));
}
