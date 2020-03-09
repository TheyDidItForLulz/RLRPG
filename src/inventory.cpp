#include<inventory.hpp>

#include<items/item.hpp>

#include<stdexcept>
#include<algorithm>

Inventory::Inventory(Inventory const & other) {
    for (auto const & item : other.items) {
        items.emplace(item.first, item.second->cloneItem());
    }
}

Inventory & Inventory::operator=(Inventory const & other) {
    items.clear();
    for (auto const & item : other.items) {
        items.emplace(item.first, item.second->cloneItem());
    }
    return *this;
}

Ptr<Item> Inventory::remove(char id) {
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

Item const & Inventory::operator [](char id) const {
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
