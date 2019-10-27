#include<units/unit.hpp>

#include<item.hpp>
#include<utils.hpp>
#include<array2d.hpp>
#include<game.hpp>

#include<thread>
#include<queue>
#include<cassert>
#include<iterator>

static const int VISION_PRECISION = 256;

Unit::Unit(const Unit & other)
    : health(other.health)
    , maxHealth(other.maxHealth)
    , pos(other.pos)
    , id(other.id)
    , vision(other.vision)
    , inventory(other.inventory)
    , weapon(), armor() {
    if (other.weapon != nullptr) {
        weapon = dynamic_cast<Weapon *>(&inventory[other.weapon->inventorySymbol]);
    }
    if (other.armor != nullptr) {
        armor = dynamic_cast<Armor *>(&inventory[other.armor->inventorySymbol]);
    }
}

Unit & Unit::operator =(const Unit & other) {
    if (this == &other) {
        return *this;
    }
    health = other.health;
    maxHealth = other.maxHealth;
    pos = other.pos;
    id = other.id;
    vision = other.vision;
    inventory = other.inventory;
    if (other.weapon == nullptr) {
        weapon = nullptr;
    } else {
        weapon = dynamic_cast<Weapon *>(&inventory[other.weapon->inventorySymbol]);
    }
    if (other.armor == nullptr) {
        armor = nullptr;
    } else {
        armor = dynamic_cast<Armor *>(&inventory[other.armor->inventorySymbol]);
    }
    return *this;
}

std::string Unit::getName() {
    return name;
}

bool Unit::linearVisibilityCheck(Vec2d from, Vec2d to) const {
    Vec2d d = to - from;
    bool steep = std::abs(d.x) < std::abs(d.y);
    if (steep) {
        std::swap(d.x, d.y);
        std::swap(from.x, from.y);
    }
    double k = d.y / d.x;
    int s = sgn(d.x);
    for (int i = 0; i * s < d.x * s; i += s) {
        Vec2i c = from + Vec2d{ double(i), i * k };
        if (steep)
            std::swap(c.x, c.y);
        if (g_game.level()[c] == 2)
            return false;
    }
    return true;
}

void Unit::heal(int hp) {
    health = std::min(health + hp, maxHealth);
}

bool Unit::canSee(Coord2i cell) const {
    double offset = 1.0 / VISION_PRECISION;
    Vec2d celld{ cell };
    return distSquared(pos, cell) < sqr(vision)
        and (linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ offset, offset })
        or linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ offset, 1 - offset })
        or linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ 1 - offset, offset })
        or linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ 1 - offset, 1 - offset }));
}

void Unit::setTo(Coord2i cell) {
    auto & unitsMap = g_game.getUnitsMap();
    if (g_game.level()[cell] == 2 or unitsMap[cell] or pos == cell)
        return;

    unitsMap[cell] = std::move(unitsMap[pos]);
    pos = cell;
}

void Unit::dealDamage(int damage) {
    int defence = 0;
    if (armor != nullptr)
        defence = armor->defence;
    health -= damage * (100 - defence) / 100.f;
}

void Unit::dropInventory() {
    weapon = nullptr;
    takeArmorOff();
    while (not inventory.isEmpty()) {
        auto id = inventory.begin()->second->inventorySymbol;
        g_game.drop(inventory.remove(id), pos);
    }
    assert(inventory.isEmpty());
}

void Unit::takeArmorOff() {
    armor = nullptr;
}

void Unit::unequipWeapon() {
    weapon = nullptr;
}
