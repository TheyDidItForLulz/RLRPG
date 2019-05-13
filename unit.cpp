#include<ctime>
#include<thread>

#include<unit.hpp>
#include<utils.hpp>
#include<globals.hpp>

#include<queue>

int g_vision = 16;
int g_maxBurden = 25;                                

Unit::Ptr unitMap[FIELD_ROWS][FIELD_COLS];

Unit::Unit(const Unit & other)
    : health(other.health)
    , maxHealth(other.maxHealth)
    , pos(other.pos)
    , symbol(other.symbol)
    , vision(other.vision)
    , weapon(), armor() {
}

Unit & Unit::operator =(const Unit & other) {
    if (this == &other) {
        return *this;
    }
    health = other.health;
    maxHealth = other.maxHealth;
    pos = other.pos;
    symbol = other.symbol;
    vision = other.vision;
    weapon = nullptr;
    armor = nullptr;
    return *this;
}

std::string Unit::getName() {
    switch (symbol) {
        case 200:
            return "Hero";
        case 201:
            return "Barbarian";
        case 202:
            return "Zombie";
    }
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
        int x = from.x + i;
        int y = from.y + i * k;
        if (steep)
            std::swap(x, y);
        if (map[y][x] == 2)
            return false;
    }
    return true;
}

void Unit::heal(int hp) {
    health = std::min(health + hp, maxHealth);
}

bool Unit::canSee(Coord cell) const {
    double offset = 1. / VISION_PRECISION;
    Vec2d celld{ cell };
    return linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ offset, offset })
        or linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ offset, 1 - offset })
        or linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ 1 - offset, offset })
        or linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ 1 - offset, 1 - offset });
}

void Unit::setTo(Coord cell) {
    if (map[cell.y][cell.x] == 2 or unitMap[cell.y][cell.x] or pos == cell)
        return;

    unitMap[cell.y][cell.x] = std::move(unitMap[pos.y][pos.x]);
    pos = cell;
}

void Unit::dealDamage(int damage) {
    int defence = 0;
    if (armor != nullptr)
        defence = armor->defence;
    health -= damage * (100 - defence) / 100.f;
}

