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
            inventory[0] = std::make_unique<Food>(foodTypes[0]);
            inventory[1] = std::make_unique<Weapon>(weaponTypes[0]);
            weapon = dynamic_cast<Weapon *>(inventory[1].get());
            symbol = 201;
            vision = 16;
            xpCost = 3;
            break;
        }
        case 1: {
            health = 10;
            maxHealth = 10;
            inventory[0] = std::make_unique<Weapon>(weaponTypes[3]);
            weapon = dynamic_cast<Weapon *>(inventory[0].get());
            symbol = 202;
            vision = 10;
            xpCost = 2;
            break;
        }
        case 2: {
            health = 5;
            maxHealth = 5;
            inventory[0] = std::make_unique<Weapon>(weaponTypes[5]);
            inventory[1] = std::make_unique<Ammo>(ammoTypes[0]);
            weapon = dynamic_cast<Weapon *>(inventory[0].get());
            ammo = dynamic_cast<Ammo *>(inventory[1].get());
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
    for (int i = 0; i < MAX_INV_SIZE; ++i) {
        if (not other.inventory[i])
            continue;
        inventory[i] = other.inventory[i]->clone();
        if (other.inventory[i].get() == other.weapon)
            weapon = dynamic_cast<Weapon *>(inventory[i].get());
        if (other.inventory[i].get() == other.armor)
            armor = dynamic_cast<Armor *>(inventory[i].get());
        if (other.inventory[i].get() == other.ammo)
            ammo = dynamic_cast<Ammo *>(inventory[i].get());
    }
}

Enemy & Enemy::operator =(const Enemy & other) {
    if (this == &other) {
        return *this;
    }
    Unit::operator =(other);
    weapon = nullptr;
    armor = nullptr;
    ammo = nullptr;
    for (int i = 0; i < MAX_INV_SIZE; ++i) {
        inventory[i].reset();
        if (not other.inventory[i])
            continue;
        inventory[i] = other.inventory[i]->clone();
        if (other.inventory[i].get() == other.weapon)
            weapon = dynamic_cast<Weapon *>(inventory[i].get());
        if (other.inventory[i].get() == other.armor)
            armor = dynamic_cast<Armor *>(inventory[i].get());
        if (other.inventory[i].get() == other.ammo)
            ammo = dynamic_cast<Ammo *>(inventory[i].get());
    }
    return *this;
}

void Enemy::dropInventory() {
    weapon = nullptr;
    armor = nullptr;
    ammo = nullptr;
    for (int i = 0; i < MAX_INV_SIZE; i++) {
        drop(std::move(inventory[i]), pos);
    }
}

void Enemy::shoot() {
    if (weapon == nullptr or ammo == nullptr)
        return;

    auto dir = directionFrom(g_hero->pos - pos).value();
    Vec2i offset = toVec2i(dir);
    char sym = toChar(dir);
    for (int i = 1; i < weapon->range + ammo->range; i++) {
        Coord cell = pos + offset * i;

        if (map[cell.y][cell.x] == 2)
            break;

        if (unitMap[cell.y][cell.x] and unitMap[cell.y][cell.x]->getType() == UnitHero) {
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
        for (int i = 0; i < MAX_INV_SIZE; ++i) {
            if (inventory[i].get() == ammo) {
                inventory[i].reset();
                ammo = nullptr;
            }
        }
    }
}

std::optional<Coord> Enemy::searchForShortestPath(Coord to) const {
    if (to == pos)
        return std::nullopt;

    int maxDepth = 2 + std::abs(to.x - pos.x) + std::abs(to.y - pos.y);

    std::queue<Coord> q;
    q.push(pos);

    int used[FIELD_ROWS][FIELD_COLS] = {};
    used[pos.y][pos.x] = 1;

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
        Coord v = q.front();
        if (v == to)
            break;

        if (used[v.y][v.x] > maxDepth)
            return std::nullopt;

        q.pop();

        for (auto dir : dirs) {
            auto tv = v + dir;
            if (tv.x >= 0 and tv.y >= 0 and tv.x < FIELD_COLS - 1 and tv.y < FIELD_ROWS - 1
                    and (not unitMap[tv.y][tv.x] or unitMap[tv.y][tv.x]->getType() == UnitHero)
                    and map[tv.y][tv.x] != 2 and used[tv.y][tv.x] == 0) {
                q.push(tv);
                used[tv.y][tv.x] = 1 + used[v.y][v.x];
            }
        }
    }

    if (not used[to.y][to.x])
        return std::nullopt;

    Coord v = to;
    while (used[v.y][v.x] > 2) {
        for (auto dir : dirs) {
            auto tv = v - dir;
            if (tv.x >= 0 and tv.y >= 0 and tv.x < FIELD_COLS - 1 and tv.y < FIELD_ROWS - 1
                    and used[tv.y][tv.x] + 1 == used[v.y][v.x]) {
                v = tv;
                break;
            }
        }
    }

    return v;
}

void Enemy::moveTo(Coord cell) {
    if (cell.x < 0 or cell.y < 0 or cell.y >= FIELD_ROWS or cell.x >= FIELD_COLS)
        throw std::logic_error("Trying to move an enemy away from the map");

    if (map[cell.y][cell.x] == 2)
        throw std::logic_error("Trying to move an enemy into a wall");

    if (not unitMap[cell.y][cell.x]) {
        setTo(cell);
        return;
    }

    if (unitMap[cell.y][cell.x]->getType() == UnitEnemy or weapon == nullptr)
        return;

    if (g_hero->armor == nullptr or g_hero->armor->mdf != 2) {
        g_hero->dealDamage(weapon->damage);
    } else {
        dealDamage(weapon->damage);
    }

    if (health <= 0) {
        unitMap[pos.y][pos.x].reset();
        return;
    }
}

void Enemy::updatePosition() {
    lastTurnMoved = g_turns;

    bool canSeeHero =
        not g_hero->isInvisible()
        and distSquared(pos, g_hero->pos) < sqr(vision)
        and canSee(g_hero->pos);

    if (canSeeHero) {
        bool onDiagLine = std::abs(g_hero->pos.y - pos.y) == std::abs(g_hero->pos.x - pos.x);
        if ((pos.y == g_hero->pos.y or pos.x == g_hero->pos.x or onDiagLine)
                and weapon and weapon->isRanged and ammo
                and weapon->range + ammo->range >= std::abs(g_hero->pos.y - pos.y) + std::abs(g_hero->pos.x - pos.x)) {
            shoot();
        } else {
            target = g_hero->pos;

            if (auto next = searchForShortestPath(g_hero->pos)) {
                moveTo(*next);
                return;
            } else {
                throw std::logic_error("Can see hero, but can't see him (?)");
                canSeeHero = false;
            }
        }
    }
    if (not canSeeHero) {
        if (auto next = searchForShortestPath(*target)) {
            moveTo(*next);
            return;
        } else {
            std::vector<Coord> visibleCells;

            for (int i = std::max(pos.y - vision, 0); i < std::min(FIELD_ROWS, pos.y + vision); i++) {
                for (int j = std::max(pos.x - vision, 0); j < std::min(pos.x + vision, FIELD_COLS); j++) {
                    Vec2i cell{ j, i };
                    if (cell != pos and map[i][j] != 2
                            and distSquared(pos, cell) < sqr(vision)
                            and not unitMap[i][j] and canSee(cell)) {
                        visibleCells.push_back(cell);
                    }
                }    
            }

            int attempts = 15;
            for (int i = 0; i < attempts; ++i) {
                target = visibleCells[std::rand() % visibleCells.size()];

                if (auto next = searchForShortestPath(*target)) {
                    moveTo(*next);
                    return;
                }
            }
        }
    }
}

