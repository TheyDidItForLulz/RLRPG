#include<enemy.hpp>
#include<globals.hpp>
#include<direction.hpp>
#include<hero.hpp>

#include<queue>

Enemy enemyTypes[Enemy::TYPES_COUNT];

Enemy::Enemy(int eType) {
    switch (eType) {
        case 0: {
            health = 7;
            maxHealth = 7;
            inventory.add(std::make_unique<Food>(foodTypes[0]));
            inventory.add(std::make_unique<Weapon>(weaponTypes[0]));
            weapon = dynamic_cast<Weapon *>(&inventory['b']);
            symbol = 201;
            vision = 16;
            xpCost = 3;
            break;
        }
        case 1: {
            health = 10;
            maxHealth = 10;
            inventory.add(std::make_unique<Weapon>(weaponTypes[3]));
            weapon = dynamic_cast<Weapon *>(&inventory['a']);
            symbol = 202;
            vision = 10;
            xpCost = 2;
            break;
        }
        case 2: {
            health = 5;
            maxHealth = 5;
            inventory.add(std::make_unique<Weapon>(weaponTypes[5]));
            inventory.add(std::make_unique<Ammo>(ammoTypes[0]));
            weapon = dynamic_cast<Weapon *>(&inventory['a']);
            ammo = dynamic_cast<Ammo *>(&inventory['b']);
            ammo->count = std::rand() % 30 + 4;
            symbol = 203;
            vision = 16;
            xpCost = 5;
            break;
        }
    }
}

Enemy::Enemy(const Enemy & other)
    : Unit(other) {
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

    auto dir = directionFrom(g_hero->pos - pos).value();
    Vec2i offset = toVec2i(dir);
    char sym = toChar(dir);
    for (int i = 1; i < weapon->range + ammo->range; i++) {
        Coord2i cell = pos + offset * i;

        if (level[cell] == 2)
            break;

        if (unitMap[cell] and unitMap[cell]->getType() == UnitHero) {
            g_hero->dealDamage(ammo->damage + weapon->damageBonus);
            break;
        }
        termRend
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
    if (g_mode == 2) {
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
            if (unitMap.isIndex(tv)
                    and (not unitMap[tv] or unitMap[tv]->getType() == UnitHero)
                    and level[tv] != 2 and used[tv] == 0) {
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
    if (level[cell] == 2)
        throw std::logic_error("Trying to move an enemy into a wall");

    if (not unitMap[cell]) {
        setTo(cell);
        return;
    }

    if (unitMap[cell]->getType() == UnitEnemy or weapon == nullptr)
        return;

    if (g_hero->armor == nullptr or g_hero->armor->mdf != 2) {
        g_hero->dealDamage(weapon->damage);
    } else {
        dealDamage(weapon->damage);
    }

    if (health <= 0) {
        unitMap[pos].reset();
        return;
    }
}

void Enemy::updatePosition() {
    lastTurnMoved = g_turns;

    if (not g_hero->isInvisible() and canSee(g_hero->pos)) {
        bool onDiagLine = std::abs(g_hero->pos.y - pos.y) == std::abs(g_hero->pos.x - pos.x);
        bool canShootHero = (pos.y == g_hero->pos.y or pos.x == g_hero->pos.x or onDiagLine)
                and weapon and weapon->isRanged and ammo
                and weapon->range + ammo->range >= std::abs(g_hero->pos.y - pos.y) + std::abs(g_hero->pos.x - pos.x);
        if (canShootHero) {
            shoot();
        } else {
            target = g_hero->pos;

            if (auto next = searchForShortestPath(g_hero->pos)) {
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

    unitMap.forEach([&] (Coord2i cell, const Unit::Ptr & unit) {
        if (cell != pos and level[cell] != 2 and not unit and canSee(cell)) {
            visibleCells.push_back(cell);
        }
    });

    int attempts = 15;
    for (int i = 0; i < attempts; ++i) {
        target = visibleCells[std::rand() % visibleCells.size()];

        if (auto next = searchForShortestPath(*target)) {
            moveTo(*next);
            return;
        }
    }
}
