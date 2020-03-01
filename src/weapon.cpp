#include<items/weapon.hpp>

#include<items/ammo.hpp>

#include<cassert>
#include<memory>

Weapon::Cartridge::Cartridge(int capacity): capacity(capacity) {
    assert(capacity >= 0);
}

Weapon::Cartridge::Cartridge(Cartridge const & other): capacity(other.capacity) {
    for (auto const & bullet : other) {
        loaded.push_back(std::make_unique<Ammo>(*bullet));
    }
}

Weapon::Cartridge & Weapon::Cartridge::operator =(Cartridge const & other) {
    capacity = other.capacity;
    loaded.clear();
    for (auto const & bullet : other) {
        loaded.push_back(std::make_unique<Ammo>(*bullet));
    }
    return *this;
}

Ptr<Ammo> Weapon::Cartridge::load(Ptr<Ammo> bullet) {
    if (loaded.size() == capacity) {
        return bullet;
    }
    loaded.push_back(std::move(bullet));
    return nullptr;
}

Ptr<Ammo> Weapon::Cartridge::unloadOne() {
    if (loaded.empty()) {
        return nullptr;
    }
    auto bullet = std::move(loaded.back());
    loaded.pop_back();
    return bullet;
}

Ammo & Weapon::Cartridge::next() {
    assert(not isEmpty());
    return *loaded.back();
}

Ammo const & Weapon::Cartridge::next() const {
    assert(not isEmpty());
    return *loaded.back();
}

Ammo const * Weapon::Cartridge::operator [](int ind) const {
    assert(ind >= 0 and ind < capacity);
    if (ind < loaded.size()) {
        return loaded[ind].get();
    }
    return nullptr;
}

auto Weapon::Cartridge::begin() const -> decltype(loaded.begin()) {
    return loaded.begin();
}

auto Weapon::Cartridge::end() const -> decltype(loaded.end()) {
    return loaded.end();
}

int Weapon::Cartridge::getCapacity() const {
    return capacity;
}

int Weapon::Cartridge::getCurrSize() const {
    return (int) loaded.size();
}

bool Weapon::Cartridge::isEmpty() const {
    return loaded.empty();
}

bool Weapon::Cartridge::isFull() const {
    return loaded.size() == capacity;
}

