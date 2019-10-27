#include<units/enemy.hpp>

#include<item.hpp>
#include<direction.hpp>
#include<units/hero.hpp>
#include<game.hpp>

#include<effolkronium/random.hpp>

#include<fmt/format.h>

#include<queue>

using Random = effolkronium::random_static;

std::unordered_map<std::string, Enemy> enemyTypes;

Enemy::Enemy(std::string_view id) {
    this->id = id;
    const auto & foodTypes = g_game.getFoodTypes();
    const auto & weaponTypes = g_game.getWeaponTypes();
    const auto & ammoTypes = g_game.getAmmoTypes();

    if (id == "barbarian") {
        health = 7;
        maxHealth = 7;
        inventory.add(foodTypes.at("egg")->clone());
        inventory.add(weaponTypes.at("copper_shortsword")->clone());
        weapon = dynamic_cast<Weapon *>(&inventory['b']);
        vision = 16;
        xpCost = 3;
    } else if (id == "zombie") {
        health = 10;
        maxHealth = 10;
        inventory.add(weaponTypes.at("stick")->clone());
        weapon = dynamic_cast<Weapon *>(&inventory['a']);
        vision = 10;
        xpCost = 2;
    } else if (id == "guardian") {
        health = 5;
        maxHealth = 5;
        inventory.add(weaponTypes.at("pistol")->clone());
        inventory.add(ammoTypes.at("steel_bullets")->clone());
        weapon = dynamic_cast<Weapon *>(&inventory['a']);
        ammo = dynamic_cast<Ammo *>(&inventory['b']);
        ammo->count = Random::get(4, 30);
        vision = 16;
        xpCost = 5;
    } else {
        throw std::logic_error(fmt::format("Trying to create an enemy with unknown id '{}'", id));
    }
}

Enemy::Enemy(const Enemy & other)
    : Unit(other)
    , target(other.target)
    , lastTurnMoved(other.lastTurnMoved)
    , xpCost(other.xpCost) {
    if (other.ammo == nullptr) {
        ammo = nullptr;
    } else {
        ammo = dynamic_cast<Ammo *>(&inventory[other.ammo->inventorySymbol]);
    }
}

Enemy & Enemy::operator =(const Enemy & other) {
    if (this == &other) {
        return *this;
    }
    Unit::operator =(other);
    target = other.target;
    lastTurnMoved = other.lastTurnMoved;
    xpCost = other.xpCost;
    if (other.ammo == nullptr) {
        ammo = nullptr;
    } else {
        ammo = dynamic_cast<Ammo *>(&inventory[other.ammo->inventorySymbol]);
    }
    return *this;
}

void Enemy::dropInventory() {
    ammo = nullptr;
    Unit::dropInventory();
}

void Enemy::shoot() {
    if (weapon == nullptr or ammo == nullptr)
        return;

    auto dir = directionFrom(g_game.getHero().pos - pos).value();
    Vec2i offset = toVec2i(dir);
    char sym = toChar(dir);
    for (int i = 1; i < weapon->range + ammo->range; i++) {
        Coord2i cell = pos + offset * i;

        if (g_game.level()[cell] == 2)
            break;

        const auto & unitsMap = g_game.getUnitsMap();
        if (unitsMap[cell] and unitsMap[cell]->getType() == Unit::Type::Hero) {
            g_game.getHero().dealDamage(ammo->damage + weapon->damageBonus);
            break;
        }
        g_game.getRenderer()
            .setCursorPosition(cell)
            .put(sym)
            .display();
        sleep(DELAY / 3);
    }

    ammo->count--;
    if (ammo->count <= 0) {
        inventory.remove(ammo->inventorySymbol);
        ammo = nullptr;
    }
}

std::optional<Coord2i> Enemy::searchForShortestPath(Coord2i to) const {
    if (to == pos)
        return std::nullopt;

    int maxDepth = 2 + std::abs(to.x - pos.x) + std::abs(to.y - pos.y);

    std::queue<Coord2i> q;
    q.push(pos);

    Array2D<int, LEVEL_ROWS, LEVEL_COLS> used;
    used[pos] = 1;

    std::vector<Vec2i> dirs = {
        toVec2i(Direction::Up),
        toVec2i(Direction::Down),
        toVec2i(Direction::Right),
        toVec2i(Direction::Left)
    };
    if (g_game.getMode() == 2) {
        dirs.push_back(toVec2i(Direction::UpRight));
        dirs.push_back(toVec2i(Direction::UpLeft));
        dirs.push_back(toVec2i(Direction::DownRight));
        dirs.push_back(toVec2i(Direction::DownLeft));
    }

    while (not q.empty()) {
        Coord2i v = q.front();
        if (v == to)
            break;

        if (used[v] > maxDepth)
            return std::nullopt;

        q.pop();

        for (auto dir : dirs) {
            auto tv = v + dir;
            const auto & unitsMap = g_game.getUnitsMap();
            if (unitsMap.isIndex(tv)
                    and (not unitsMap[tv] or unitsMap[tv]->getType() == Unit::Type::Hero)
                    and g_game.level()[tv] != 2 and used[tv] == 0) {
                q.push(tv);
                used[tv] = 1 + used[v];
            }
        }
    }

    if (not used[to])
        return std::nullopt;

    Coord2i v = to;
    while (used[v] > 2) {
        for (auto dir : dirs) {
            auto tv = v - dir;
            if (used.isIndex(tv) and used[tv] + 1 == used[v]) {
                v = tv;
                break;
            }
        }
    }

    return v;
}

void Enemy::moveTo(Coord2i cell) {
    if (g_game.level()[cell] == 2)
        throw std::logic_error("Trying to move an enemy into a wall");

    const auto & unitsMap = g_game.getUnitsMap();
    if (not unitsMap[cell]) {
        setTo(cell);
        return;
    }

    if (unitsMap[cell]->getType() == Unit::Type::Enemy or weapon == nullptr)
        return;

    auto & hero = g_game.getHero();
    if (hero.armor == nullptr or hero.armor->mdf != 2) {
        hero.dealDamage(weapon->damage);
    } else {
        dealDamage(weapon->damage);
    }

    if (health <= 0) {
        g_game.getUnitsMap()[pos].reset();
        return;
    }
}

void Enemy::updatePosition() {
    lastTurnMoved = g_game.getTurnNumber();
    const auto & hero = g_game.getHero();

    if (not hero.isInvisible() and canSee(hero.pos)) {
        bool onDiagLine = std::abs(hero.pos.y - pos.y) == std::abs(hero.pos.x - pos.x);
        bool canShootHero = (pos.y == hero.pos.y or pos.x == hero.pos.x or onDiagLine)
                and weapon and weapon->isRanged and ammo
                and weapon->range + ammo->range >= std::abs(hero.pos.y - pos.y) + std::abs(hero.pos.x - pos.x);
        if (canShootHero) {
            shoot();
        } else {
            target = hero.pos;

            if (auto next = searchForShortestPath(hero.pos)) {
                moveTo(*next);
                return;
            }
        }
    }
    std::optional<Coord2i> next;
    if (target.has_value() and (next = searchForShortestPath(*target))) {
        moveTo(*next);
        return;
    }

    std::vector<Coord2i> visibleCells;

    g_game.getUnitsMap().forEach([&] (Coord2i cell, const Ptr<Unit> & unit) {
        if (cell != pos and g_game.level()[cell] != 2 and not unit and canSee(cell)) {
            visibleCells.push_back(cell);
        }
    });

    int attempts = 15;
    for (int i = 0; i < attempts; ++i) {
        target = *Random::get(visibleCells);

        if (auto next = searchForShortestPath(*target)) {
            moveTo(*next);
            return;
        }
    }
}

