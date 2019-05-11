#include<ctime>
#include<thread>

#include"include/unit.hpp"
#include"include/utils.hpp"
#include"include/level.hpp"
#include"include/colors.hpp"
#include"include/controls.hpp"
#include"include/globals.hpp"
#include"include/log.hpp"
#include<termlib/termlib.hpp>

#include<fmt/core.h>
#include<fmt/printf.h>

#include<queue>
#include<algorithm>

using namespace fmt::literals;

int g_vision = 16;
int g_maxBurden = 25;                                

extern TerminalRenderer termRend;
extern TerminalReader termRead;

Unit::Unit(const Unit & other)
    : health(other.health)
    , maxHealth(other.maxHealth)
    , inventoryVol(other.inventoryVol)
    , posH(other.posH)
    , posL(other.posL)
    , symbol(other.symbol)
    , vision(other.vision)
    , unitWeapon(), unitArmor(), unitAmmo() {
    for (int i = 0; i < UNITINVENTORY; ++i) {
        if (not other.unitInventory[i])
            continue;
        unitInventory[i] = other.unitInventory[i]->clone();
        if (other.unitInventory[i].get() == other.unitWeapon)
            unitWeapon = dynamic_cast<Weapon *>(unitInventory[i].get());
        if (other.unitInventory[i].get() == other.unitArmor)
            unitArmor = dynamic_cast<Armor *>(unitInventory[i].get());
        if (other.unitInventory[i].get() == other.unitAmmo)
            unitAmmo = dynamic_cast<Ammo *>(unitInventory[i].get());
    }
}

Unit & Unit::operator =(const Unit & other) {
    if (this == &other) {
        return *this;
    }
    health = other.health;
    maxHealth = other.maxHealth;
    inventoryVol = other.inventoryVol;
    posH = other.posH;
    posL = other.posL;
    symbol = other.symbol;
    vision = other.vision;
    unitWeapon = nullptr;
    unitArmor = nullptr;
    unitAmmo = nullptr;
    for (int i = 0; i < UNITINVENTORY; ++i) {
        unitInventory[i].reset();
        if (not other.unitInventory[i])
            continue;
        unitInventory[i] = other.unitInventory[i]->clone();
        if (other.unitInventory[i].get() == other.unitWeapon)
            unitWeapon = dynamic_cast<Weapon *>(unitInventory[i].get());
        if (other.unitInventory[i].get() == other.unitArmor)
            unitArmor = dynamic_cast<Armor *>(unitInventory[i].get());
        if (other.unitInventory[i].get() == other.unitAmmo)
            unitAmmo = dynamic_cast<Ammo *>(unitInventory[i].get());
    }
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

bool Unit::linearVisibilityCheck(double fromX, double fromY, double toX, double toY) const {
    double dx = toX - fromX;
    double dy = toY - fromY;
    bool steep = std::abs(dx) < std::abs(dy);
    if (steep) {
        std::swap(dx, dy);
        std::swap(fromX, fromY);
    }
    double k = dy / dx;
    int s = sgn(dx);
    for (int i = 0; i * s < dx * s; i += s) {
        int x = fromX + i;
        int y = fromY + i * k;
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

bool Unit::canSeeCell(int h, int l) const {
    double offset = 1. / VISION_PRECISION;
    return
        linearVisibilityCheck(posL + .5, posH + .5, l + offset, h + offset) ||
        linearVisibilityCheck(posL + .5, posH + .5, l + offset, h + 1 - offset) ||
        linearVisibilityCheck(posL + .5, posH + .5, l + 1 - offset, h + offset) ||
        linearVisibilityCheck(posL + .5, posH + .5, l + 1 - offset, h + 1 - offset);
}

void Unit::dropInventory() {
    for (int i = 0; i < UNITINVENTORY; i++) {
        if (not unitInventory[i])
            continue;
        if (unitInventory[i]->isStackable) {
            auto it = findItemAtCell(posH, posL, unitInventory[i]->symbol);
            if (it != end(itemsMap[posH][posL])) {
                (*it)->count += unitInventory[i]->count;
                unitInventory[i].reset();
                continue;
            }
        }
        itemsMap[posH][posL].push_back(std::move(unitInventory[i]));
    }
}

void Unit::move(int row, int col) {
    if (map[row][col] == 2 or unitMap[row][col].type != UnitEmpty or posH == row and posL == col)
        return;

    unitMap[row][col] = unitMap[posH][posL];
    unitMap[row][col].getUnit().posH = row;
    unitMap[row][col].getUnit().posL = col;
    unitMap[posH][posL].type = UnitEmpty;
}

void Unit::dealDamage(int damage) {
    int defence = 0;
    if (unitArmor != nullptr)
        defence = unitArmor->defence;
    health -= damage * (100 - defence) / 100.f;
}

Enemy differentEnemies[Enemy::TYPES_COUNT];

Enemy::Enemy(int eType) {
    switch (eType) {
        case 0: {
            health = 7;
            maxHealth = 7;
            unitInventory[0] = std::make_unique<Food>(foodTypes[0]);
            unitInventory[1] = std::make_unique<Weapon>(weaponTypes[0]);
            unitWeapon = dynamic_cast<Weapon *>(unitInventory[1].get());
            inventoryVol = 2;
            symbol = 201;
            vision = 16;
            xpIncreasing = 3;
            break;
        }
        case 1: {
            health = 10;
            maxHealth = 10;
            unitInventory[0] = std::make_unique<Weapon>(weaponTypes[3]);
            unitWeapon = dynamic_cast<Weapon *>(unitInventory[0].get());
            inventoryVol = 1;
            symbol = 202;
            vision = 10;
            xpIncreasing = 2;
            break;
        }
        case 2: {
            health = 5;
            maxHealth = 5;
            unitInventory[0] = std::make_unique<Weapon>(weaponTypes[5]);
            unitInventory[1] = std::make_unique<Ammo>(ammoTypes[0]);
            unitWeapon = dynamic_cast<Weapon *>(unitInventory[0].get());
            unitAmmo = dynamic_cast<Ammo *>(unitInventory[1].get());
            unitAmmo->count = std::rand() % 30 + 4;
            inventoryVol = 2;
            symbol = 203;
            vision = 16;
            xpIncreasing = 5;
            break;
        }
    }
    dist = 0;
    targetH = -1;
    targetL = -1;
}

char getProjectileSymbol(Direction direction) {
    switch (direction) {
        case Direction::Up:
        case Direction::Down: return '|';
        case Direction::Left:
        case Direction::Right: return '-';
        case Direction::UpRight:
        case Direction::DownLeft: return '/';
        case Direction::UpLeft:
        case Direction::DownRight: return '\\';
        default: throw std::logic_error("Unknown direction");
    }
}

void Enemy::shoot() {
    if (unitWeapon == nullptr or unitAmmo == nullptr)
        return;

    auto dir = directionFrom(Vec2i{ g_hero.posL - posL, g_hero.posH - posH }).value();
    Vec2i offset = toVec2i(dir);
    char sym = getProjectileSymbol(dir);
    for (int i = 1; i < unitWeapon->range + unitAmmo->range; i++) {
        int row = posH + offset.y * i;
        int col = posL + offset.x * i;

        if (map[row][col] == 2)
            break;

        if (unitMap[row][col].type == UnitHero) {
            g_hero.dealDamage(unitAmmo->damage + unitWeapon->damageBonus);
            break;
        }
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
        sleep(DELAY / 3);
    }

    unitAmmo->count--;
    if (unitAmmo->count <= 0) {
        for (int i = 0; i < UNITINVENTORY; ++i) {
            if (unitInventory[i].get() == unitAmmo) {
                unitInventory[i].reset();
                unitAmmo = nullptr;
            }
        }
    }
}

int bfs(int targetH, int targetL, int h, int l, int &posH, int &posL) {
    if (targetH == h and targetL == l) {
        return -1;
    }
    int depth = 2 + std::abs(targetH - h) + std::abs(targetL - l);                        // <- smth a little bit strange
    std::queue<Vec2i> q;
    q.push(Vec2i{ l, h });
    int used[FIELD_ROWS][FIELD_COLS] = {};
    used[h][l] = 1;
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
        auto v = q.front();
        if (v.y == targetH && v.x == targetL)
            break;
        if (used[v.y][v.x] > depth) {
            return -1;
        }
        q.pop();

        for (auto dir : dirs) {
            auto tv = v + dir;
            if (tv.x >= 0 and tv.y >= 0 and tv.x < FIELD_COLS - 1 and tv.y < FIELD_ROWS - 1
                    and unitMap[tv.y][tv.x].type != UnitEnemy and map[tv.y][tv.x] != 2 and used[tv.y][tv.x] == 0) {
                q.push(tv);
                used[tv.y][tv.x] = 1 + used[v.y][v.x];
            }
        }
    }

    if (!used[targetH][targetL]) {
        return -1;
    }
    Vec2i v{ targetL, targetH };
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

    posH = v.y;
    posL = v.x;
    return depth - 2;
}

void Enemy::updatePosition() {
    movedOnTurn = g_turns;

    bool canSeeHero =
        not g_hero.isInvisible()
        and distSquared(Vec2i{ posL, posH }, Vec2i{ g_hero.posL, g_hero.posH }) < sqr(vision)
        and canSeeCell(g_hero.posH, g_hero.posL);
    
    int pH = 1, pL = 1;

    if (canSeeHero) {
        if ((posH == g_hero.posH or posL == g_hero.posL or std::abs(g_hero.posH - posH) == std::abs(g_hero.posL - posL))
                and unitWeapon and unitWeapon->Ranged and unitAmmo
                and unitWeapon->range + unitAmmo->range >= std::abs(g_hero.posH - posH) + std::abs(g_hero.posL - posL)) {
            shoot();
        } else {
            targetH = g_hero.posH;
            targetL = g_hero.posL;

            if (bfs(g_hero.posH, g_hero.posL, posH, posL, pH, pL) == -1) {
                canSeeHero = false;
            } else {
                if (unitMap[pH][pL].type == UnitEnemy) {
                    return;
                } else if (unitWeapon and unitMap[pH][pL].type == UnitHero) {
                    //if (unitWeapon->getType() == ItemWeapon or unitunitWeapon->type) {
                    if (g_hero.heroArmor == nullptr or g_hero.heroArmor->mdf != 2) {
                        g_hero.dealDamage(unitWeapon->damage);
                    } else {
                        dealDamage(unitWeapon->damage);
                    }
                        /*
                    } else if (unitWeapon->type == ItemTools) {
                        if (g_hero.heroArmor == nullptr or g_hero.heroArmor->mdf != 2) {
                            g_hero.dealDamage(unitWeapon->damage);
                        } else {
                            health -= unitWeapon->item.invTools.damage;
                        }
                    }*/
                    if (health <= 0) {
                        unitMap[posH][posL].type = UnitEmpty;
                        return;
                    }
                } else {
                    move(pH, pL);
                    return;
                }
            }
        }
    }
    if (not canSeeHero) {
        bool needRandDir = false;
        if (targetH != -1 and (targetH != posH or targetL != posL)) {
            if (bfs(targetH, targetL, posH, posL, pH, pL) == -1) {
                needRandDir = true;
            } else {
                if (pH < FIELD_ROWS && pH > 0 && pL < FIELD_COLS && pL > 0) {
                    if (unitWeapon and unitMap[pH][pL].type == UnitHero) {
                        //if (unitWeapon->type == ItemWeapon) {
                        if (g_hero.heroArmor == nullptr or g_hero.heroArmor->mdf != 2) {
                            g_hero.dealDamage(unitWeapon->damage);
                        } else {
                            dealDamage(unitWeapon->damage);
                        }
                        /*} else if (unitWeapon->type == ItemTools) {
                            if (g_hero.heroArmor->item.invArmor.mdf != 2) {
                                g_hero.health -= unitWeapon->item.invTools.damage * ((100 - g_hero.heroArmor->item.invArmor.defence) / 100.0);
                            } else {
                                health -= unitWeapon->item.invTools.damage;
                            }
                        }*/
                        if (health <= 0) {
                            unitMap[posH][posL].type = UnitEmpty;
                            return;
                        }
                    } else {
                        move(pH, pL);
                        return;
                    }
                }
            }
        } else {
            needRandDir = true;
        }
        if (needRandDir) {
            std::vector<int> visionArrayH;
            std::vector<int> visionArrayL;

            for (int i = std::max(posH - vision, 0); i < std::min(FIELD_ROWS, posH + vision); i++) {
                for (int j = std::max(posL - vision, 0); j < std::min(posL + vision, FIELD_COLS); j++) {
                    if ((i != posH or j != posL) and map[i][j] != 2
                            and distSquared(Vec2i{ posL, posH }, Vec2{ j, i }) < sqr(vision)
                            and unitMap[i][j].type == UnitEmpty and canSeeCell(i, j)) {
                        visionArrayH.push_back(i);
                        visionArrayL.push_back(j);
                    }
                }    
            }
            int attempts = 15;
            bool stepped = false;;
            for (int i = 0; i < attempts; ++i) {
                int r = std::rand() % visionArrayH.size(); 
                int rposH = visionArrayH[r];
                int rposL = visionArrayL[r];
                
                targetH = rposH;
                targetL = rposL;

                if (bfs(targetH, targetL, posH, posL, pH, pL) != -1) {
                    stepped = true;
                    break;
                }
            }
            if (stepped && pH < FIELD_ROWS && pH > 0 && pL < FIELD_COLS && pL > 0) {
                if (unitWeapon and unitMap[pH][pL].type == UnitHero) {
                    //if (unitWeapon->type == ItemWeapon) {
                    if (g_hero.heroArmor == nullptr or g_hero.heroArmor->mdf != 2) {
                        g_hero.dealDamage(unitWeapon->damage);
                    } else {
                        dealDamage(unitWeapon->damage);
                    }
                    /*} else if (unitWeapon->type == ItemTools) {
                        if (g_hero.heroArmor->item.invArmor.mdf != 2) {
                            g_hero.health -= unitWeapon->item.invTools.damage * ((100 - g_hero.heroArmor->item.invArmor.defence) / 100.0);
                        } else {
                            health -= unitWeapon->item.invTools.damage;
                        }
                    }*/
                    if (health <= 0) {
                        unitMap[posH][posL].type = UnitEmpty;
                        return;
                    }
                } else {
                    /*unitMap[pH][pL] = unitMap[posH][posL];
                    unitMap[pH][pL].getUnit().posH = pH;
                    unitMap[pH][pL].getUnit().posL = pL;
                    unitMap[posH][posL].type = UnitEmpty;*/
                    move(pH, pL);
                    return;
                }
            }
        }
    }
}

Hero::Hero() {
    maxHealth = 15;
    health = 15;
    symbol = 200;
}

void Hero::dealDamage(int damage) {
    int defence = 0;
    if (heroArmor != nullptr)
        defence = heroArmor->defence;
    health -= damage * (100 - defence) / 100.f;
}

bool Hero::isInvisible() const {
    return turnsInvisible > 0;
}

void Hero::checkVisibleCells() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            seenUpdated[i][j] = 0;
            if (sqr(posH - i) + sqr(posL - j) < sqr(g_vision)) {
                seenUpdated[i][j] = canSeeCell(i, j);
            }
        }
    }
}

bool Hero::isInventoryEmpty() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i])
            return false;
    }
    return true;
}

int Hero::findEmptyInventoryCell() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (not inventory[i])
            return i;
    }
    return 101010;                                            // Magic constant, means "Inventory is full".
}

int Hero::getInventoryItemsWeight() const {
    int totalWeight = 0;
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i]) {
            totalWeight += inventory[i]->weight;
        }
    }
    return totalWeight;
}

void Hero::printList(const std::vector<Item *> & items, std::string_view msg, int mode) const {
    int num = 0;

    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, num })
        .put(msg);

    num ++;
    switch (mode) {
        case 1: {
            for (int i = 0; i < items.size(); i++) {
                termRend.setCursorPosition(Vec2i{ FIELD_COLS + 10, num });
                if (items[i]->showMdf == true && items[i]->count == 1) {
                    if (items[i]->attribute == 100) {
                        termRend.put("[{}] {} {{{}}}. "_format(
                                items[i]->inventorySymbol,
                                items[i]->getName(),
                                items[i]->getMdf()));
                    } else {
                        termRend.put("[{}] {} ({}) {{{}}}. "_format(
                                items[i]->inventorySymbol,
                                items[i]->getName(),
                                items[i]->getAttribute(),
                                items[i]->getMdf()));
                    }
                } else if (items[i]->count > 1) {
                    if (items[i]->attribute == 100) {
                        termRend.put("[{}] {} {{{}}}. "_format(
                                items[i]->inventorySymbol,
                                items[i]->getName(),
                                items[i]->count));
                    } else {
                        termRend.put("[{}] {} ({}) {{{}}}. "_format(
                                items[i]->inventorySymbol,
                                items[i]->getName(),
                                items[i]->getAttribute(),
                                items[i]->count));
                    }
                } else if (items[i]->attribute == 100) {
                    termRend.put("[{}] {}. "_format(
                            items[i]->inventorySymbol,
                            items[i]->getName()));
                } else {
                    termRend.put("[{}] {} ({}). "_format(
                            items[i]->inventorySymbol,
                            items[i]->getName(),
                            items[i]->getAttribute()));
                }
                num ++;
            }
            break;
        }
        case 2: {
            for (int i = 0; i < items.size(); i++)
            {
                termRend.setCursorPosition(Vec2i{ FIELD_COLS + 10, num });
                if (items[i]->showMdf == true) {
                    termRend.put("[{}] {} ({}) {{{}}}. "_format(
                            char(i + 'a'),
                            items[i]->getName(),
                            items[i]->getAttribute(),
                            items[i]->getMdf()));
                } else {
                    termRend.put("[{}] {} ({}). "_format(
                            char(i + 'a'),
                            items[i]->getName(),
                            items[i]->getAttribute()));
                }
                num ++;
            }
            break;
        }
    }
}

bool Hero::isMapInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->symbol == 500)
            return true;
    }
    return false;
}

// 101010 something went wrong

int Hero::findAmmoInInventory() const {
    for (int i = 0; i < BANDOLIER; i++) {
        if (inventory[AMMO_SLOT + i]) {
            return i;
        }
    }
    return 101010;
}

int Hero::findScrollInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->getType() == ItemScroll) {
            return i;
        }
    }
    return 101010;
}

void Hero::pickUpAmmo(ItemPileIter ammoIter) {                                        // Picked ammo
    clearRightPane();
    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
        .put("In what slot do you want to pull your ammo?");
    int choice = 0;
    int num = 0;
    while (true) {
        num = 0;
        for (int i = 0; i < BANDOLIER; i++) {
            termRend.setCursorPosition(Vec2i{ FIELD_COLS + num + 12, 1 });
            num += 2;
            char symbol = '-';
            TextStyle style{ TerminalColor{} };
            if (inventory[AMMO_SLOT + i]) {
                switch (inventory[AMMO_SLOT + i]->symbol) {
                    case 450:
                        symbol = ',';
                        style = TextStyle{ TextStyle::Bold, Color::Black };
                        break;
                    case 451:
                        symbol = ',';
                        style = TextStyle{ TextStyle::Bold, Color::Red };
                        break;
                    default:
                        break;
                }
            }
            if (choice == i) {
                style += TextStyle::Underlined;
            }
            termRend.put(symbol, style);
        }
        char input = termRead.readChar();
        switch (input) {
            case CONTROL_LEFT:
                if (choice > 0)
                    choice--;
                break;
            case CONTROL_RIGHT:
                if (choice < BANDOLIER - 1)
                    choice++;
                break;
            case CONTROL_CONFIRM: {
                    Item::Ptr & ammo = *ammoIter;
                    if (inventory[AMMO_SLOT + choice] == nullptr) {
                        inventory[AMMO_SLOT + choice] = std::move(ammo);
                        itemsMap[posH][posL].erase(ammoIter);
                    } else if (inventory[AMMO_SLOT + choice]->symbol == ammo->symbol) {
                        inventory[AMMO_SLOT + choice]->count += ammo->count;
                        itemsMap[posH][posL].erase(ammoIter);
                    } else {
                        std::swap(ammo, inventory[AMMO_SLOT + choice]);
                    }
                    return;
                }
            case '\033':
                return;
                break;
        }
    }
}

void Hero::pickUp() {
    if (itemsMap[posH][posL].empty()) {
        message += "There is nothing here to pick up. ";
        g_stop = true;
        return;
    } else if (itemsMap[posH][posL].size() == 1) {
        auto it = itemsMap[posH][posL].begin();
        auto & itemToPick = *it;
        message += "You picked up {}. "_format(itemToPick->getName());

        if (itemToPick->getType() == ItemAmmo) {
            pickUpAmmo(it);
            return;
        }

        bool canStack = false;

        if (itemToPick->isStackable) {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i) {
                if (inventory[i] and inventory[i]->symbol == itemToPick->symbol) {
                    canStack = true;
                    inventory[i]->count += itemToPick->count;
                    itemsMap[posH][posL].pop_back();
                }
            }
        }

        if (not canStack) {
            int eic = findEmptyInventoryCell();
            if (eic != 101010) {
                inventory[eic] = std::move(itemToPick);
                inventory[eic]->inventorySymbol = eic + 'a';
                //log("Item: {} '{}'", inventory[eic].getItem().getName(), inventory[eic].getItem().inventorySymbol);
                itemsMap[posH][posL].pop_back();
                inventoryVol++;
            } else {
                message += "Your inventory is full, motherfuck'a! ";
            }
        }

        if (getInventoryItemsWeight() > g_maxBurden && !isBurdened) {
            message += "You're burdened. ";
            isBurdened = true;
        }

        return;
    }
    
    std::vector<Item *> list;
    for (const auto & item : itemsMap[posH][posL])
        list.push_back(item.get());

    printList(list, "What do you want to pick up? ", 2);

    int intch;
    while (true) {
        char choice = termRead.readChar();
        if (choice == '\033')
            return;

        intch = choice - 'a';
        if (intch >= 0 or intch < itemsMap[posH][posL].size())
            break;
    }

    auto itemIter = std::begin(itemsMap[posH][posL]);
    std::advance(itemIter, intch);
    auto & item = *itemIter;
    
    message += "You picked up {}. "_format(item->getName());
    
    if (item->getType() == ItemAmmo) {
        pickUpAmmo(itemIter);
        return;
    }

    bool canStack = false;

    if (item->isStackable) {
        for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i) {
            if (inventory[i] and inventory[i]->symbol == item->symbol) {
                canStack = true;
                inventory[i]->count += item->count;
                itemsMap[posH][posL].erase(itemIter);
            }
        }
    }

    if (not canStack) {
        int eic = findEmptyInventoryCell();
        if (eic != 101010) {
            inventory[eic] = std::move(item);
            inventory[eic]->inventorySymbol = eic + 'a';
            itemsMap[posH][posL].erase(itemIter);
            //log("picked up item ({}) ({})", eic + 'a', char(eic + 'a'));
            //log("Item(selected): {} '{}'", inventory[eic].getItem().getName(), inventory[eic].getItem().inventorySymbol);
            inventoryVol++;
        } else {
            message += "Your inventory is full, motherfuck'a! ";
        }
    }

    if (getInventoryItemsWeight() > g_maxBurden && !isBurdened) {
        message += "You're burdened. ";
        isBurdened = true;
    }
}

bool Hero::isFoodInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->getType() == ItemFood)
            return true;
    }
    return false;
}

bool Hero::isArmorInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->getType() == ItemArmor)
            return true;
    }
    return false;
}

bool Hero::isWeaponOrToolsInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and (inventory[i]->getType() == ItemWeapon or inventory[i]->getType() == ItemTools))
            return true;
    }
    return false;
}

bool Hero::isPotionInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->getType() == ItemPotion)
            return true;
    }
    return false;
}

void Hero::clearRightPane() const {
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            termRend
                .setCursorPosition(Vec2i{ FIELD_COLS + j + 10, i })
                .put(' ');
        }
    }
}

void Hero::eat() {
    if (isFoodInInventory()) {
        showInventory(CONTROL_EAT);
    } else {
        message += "You don't have anything to eat. ";
    }
}

void Hero::moveHero(char inp) {
    int a1 = 0, a2 = 0;
    
    switch (inp) {
        case CONTROL_UP:
        case CONTROL_DOWN:
        case CONTROL_LEFT:
        case CONTROL_RIGHT:
        case CONTROL_UPLEFT:
        case CONTROL_UPRIGHT:
        case CONTROL_DOWNLEFT:
        case CONTROL_DOWNRIGHT: {
            auto offset = toVec2i(*getDirectionByControl(inp));
            moveHeroImpl(posH + offset.y, posL + offset.x);
            break;
        }
        case CONTROL_PICKUP: {
            pickUp();
            break;
        }
        case CONTROL_EAT: {
            eat();
            break;
        }
        case CONTROL_SHOWINVENTORY: {
            if (not isInventoryEmpty()) {
                showInventory(CONTROL_SHOWINVENTORY);
            } else {
                message += "Your inventory is empty. ";
            }
            g_stop = true;
            break;                
        }
        case CONTROL_WEAR: {
            if (isArmorInInventory()) {
                showInventory(CONTROL_WEAR);
            } else {
                message += "You don't have anything to wear. ";
            }
            g_stop = true;
            break;

        }
        case CONTROL_WIELD: {
            if (isWeaponOrToolsInInventory()) {
                showInventory(CONTROL_WIELD);
            } else {
                message += "You don't have anything to wield. ";
            }
            g_stop = true;
            break;
            }
        case CONTROL_TAKEOFF: {
            showInventory(CONTROL_TAKEOFF);
            g_stop = true;
            break;
        }
        case CONTROL_UNEQUIP: {
            showInventory(CONTROL_UNEQUIP);
            g_stop = true;
            break;
        }
        case CONTROL_DROP: {
            if (not isInventoryEmpty()) {
                showInventory(CONTROL_DROP);
            }
            g_stop = true;
            break;        
        }
        case CONTROL_THROW: {
            if (not isInventoryEmpty()) {
                showInventory(CONTROL_THROW);
            }
            break;
        }
        case CONTROL_SHOOT: {
            shoot();
            break;
        }
        case CONTROL_DRINK: {
            if (isPotionInInventory()) {
                showInventory(CONTROL_DRINK);
            }
            g_stop = true;
            break;
        }
        case CONTROL_OPENBANDOLIER: {
            if (findAmmoInInventory() != 101010) {
                showInventory(CONTROL_OPENBANDOLIER);
            } else {
                message += "Your bandolier is empty. ";
            }
            g_stop = true;
            break;
        }
        case CONTROL_RELOAD: {
            if (heroWeapon == nullptr or not heroWeapon->Ranged) {
                message += "You have no ranged weapon in hands. ";
                g_stop = true;
            } else if (findAmmoInInventory() != 101010) {
                showInventory(CONTROL_RELOAD);
            } else {
                message += "You have no bullets to reload. ";
                g_stop = true;
            }
            break;
        }
        case CONTROL_READ: {
            if (findScrollInInventory() != 101010) {
                showInventory(CONTROL_READ);
            } else {
                message += "You don't have anything to read. ";
            }
            g_stop = true;
            break;
        }
        case '\\': {
            char hv = termRead.readChar();
            
            if (hv == 'h') {
                if (termRead.readChar() == 'e') {
                    if (termRead.readChar() == 'a') {
                        if (termRead.readChar() == 'l') {
                            hunger = 3000;
                            health = maxHealth * 100;
                        }
                    }
                }
            }
        
            if (hv == 'w') {
                if (termRead.readChar() == 'a') {
                    if (termRead.readChar() == 'l') {
                        if (termRead.readChar() == 'l') {
                            if (termRead.readChar() == 's') {
                                canMoveThroughWalls = true;
                            }
                        }
                    }
                }
            } else if (hv == 'd') {
                if (termRead.readChar() == 's') {
                    if (termRead.readChar() == 'c') {
                        canMoveThroughWalls = false;
                    }
                } else {
                    itemsMap[1][1].push_back(std::make_unique<Food>(foodTypes[0]));
                }
            } else if (hv == 'k') {
                if (termRead.readChar() == 'i') {
                    if (termRead.readChar() == 'l') {
                        if (termRead.readChar() == 'l') {
                            health -= (health * 2) / 3;
                            message += "Ouch! ";
                        }
                    }
                }
            }
            break;
        }
    }
}

void Hero::showInventory(char inp) {    
    std::vector<Item *> list;
    auto listInserter = std::back_inserter(list);
    switch (inp) {
        case CONTROL_SHOWINVENTORY: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i])
                    list.push_back(inventory[i].get());
            
            printList(list, "Here is your inventory.", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            break;
        }
        case CONTROL_EAT: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemFood)
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to eat?", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            auto & item = inventory[intch];
            if (item and item->getType() == ItemFood) {
                int prob = rand() % g_hero.luck;
                if (prob == 0) {
                    hunger += dynamic_cast<Food &>(*item).FoodHeal / 3;
                    health --;
                    message += "Fuck! This food was rotten! ";
                } else {
                    hunger += dynamic_cast<Food &>(*item).FoodHeal;
                }
                if (item->count == 1) {
                    item.reset();
                } else {
                    item->count--;
                }
            }
            break;
        }    
        case CONTROL_WEAR: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemArmor)
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to wear?", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            auto & item = inventory[intch];
            if (item and item->getType() == ItemArmor) {
                message += "Now you wearing {}. "_format(item->getName());

                if (heroArmor != nullptr) {
                    heroArmor->attribute = 100;
                }
                heroArmor = dynamic_cast<Armor *>(item.get());
                item->attribute = 201;
            }
            break;
        }
        case CONTROL_DROP: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i])
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to drop?", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            if (heroArmor != nullptr and choice == heroArmor->inventorySymbol)
                showInventory(CONTROL_TAKEOFF);
            if (heroWeapon != nullptr and choice == heroWeapon->inventorySymbol)
                showInventory(CONTROL_UNEQUIP);
            auto & item = inventory[intch];
            if (not item)
                break;
            if (not item->isStackable) {
                itemsMap[posH][posL].push_back(std::move(item));
            } else if (item->count > 1) {
                clearRightPane();
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                    .put("How much items do you want to drop? [1-9]");

                int dropCount = clamp(1, termRead.readChar() - '0', item->count);

                auto iter = findItemAtCell(posH, posL, item->symbol);
                if (iter != end(itemsMap[posH][posL])) {        
                    (*iter)->count += dropCount;
                } else {            
                    itemsMap[posH][posL].push_back(item->clone());
                    itemsMap[posH][posL].back()->count = dropCount;
                }
                item->count -= dropCount;
                if (item->count == 0) {
                    item.reset();
                }
            } else {
                auto iter = findItemAtCell(posH, posL, item->symbol);
                if (iter != end(itemsMap[posH][posL])) {
                    (*iter)->count++;
                    item.reset();
                } else {            
                    itemsMap[posH][posL].push_back(std::move(item));
                }
            }

            if (getInventoryItemsWeight() <= g_maxBurden && isBurdened) {
                message += "You are burdened no more. ";
                isBurdened = false;
            }

            break;
        }
        case CONTROL_TAKEOFF: {
            if (heroArmor) {
                heroArmor->attribute = 100;
                heroArmor = nullptr;
            }
            break;
        }
        case CONTROL_WIELD: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemWeapon)
                    list.push_back(inventory[i].get());

            /*for (auto & item : list) {
                log("Offering item '{}' '{}'", item.getItem().inventorySymbol, (int) item.getItem().inventorySymbol);
            }*/

            printList(list, "What do you want to wield?", 1);
            
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            auto & item = inventory[intch];
            //if (inventory[intch].type == ItemWeapon || inventory[intch].type == ItemTools) {
            if (item and item->getType() == ItemWeapon) {
                message += "You wield {}. "_format(item->getName());

                if (heroWeapon != nullptr) {
                    heroWeapon->attribute = 100;
                }
                heroWeapon = dynamic_cast<Weapon *>(item.get());
                item->attribute = 301;
            }
    
            break;
        
        }
        case CONTROL_UNEQUIP: {
            if (heroWeapon != nullptr) {
                heroWeapon->attribute = 100;
                heroWeapon = nullptr;
            }
            break;
        }
        case CONTROL_THROW: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i])
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to throw?", 1);

            char choice = termRead.readChar();
            if (choice == '\033') return;
            int intch = choice - 'a';

            auto & item = inventory[intch];
            if (item) {
                clearRightPane();
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
                    .put("In what direction?");
                char secondChoise = termRead.readChar();

                if (heroArmor != nullptr and item->inventorySymbol == heroArmor->inventorySymbol)
                    showInventory(CONTROL_TAKEOFF);
                else if (heroWeapon != nullptr and item->inventorySymbol == heroWeapon->inventorySymbol)
                    showInventory(CONTROL_UNEQUIP);

                throwAnimated(std::move(item), *getDirectionByControl(secondChoise));
            }
            break;
        }
        case CONTROL_DRINK: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemPotion)
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to drink?", 1);

            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';

            auto & item = inventory[intch];
            if (item and item->getType() == ItemPotion) {
                auto & potion = dynamic_cast<Potion &>(*item);
                switch (potion.effect) {
                    case 1: {
                        heal(3);
                        message += "Now you feeling better. ";
                        break;
                    }
                    case 2: {
                        g_hero.turnsInvisible = 150;
                        message += "Am I invisible? Oh, lol! ";
                        break;
                    }
                    case 3: {
                        for (int i = 0; i < 1; i++) {
                            int l = rand() % FIELD_COLS;
                            int h = rand() % FIELD_ROWS;
                            if (map[h][l] != 2 && unitMap[h][l].type == UnitEmpty) {
                                unitMap[h][l] = unitMap[posH][posL];
                                unitMap[posH][posL].type = UnitEmpty;
                                posH = h;
                                posL = l;
                                checkVisibleCells();
                            } else {
                                i--;
                            }
                        }
                        message += "Teleportation is so straaange thing! ";
                        break;
                    }
                    case 4: {
                        message += "Well.. You didn't die. Nice. ";
                        break;
                    }
                    case 5: {
                        g_vision = 1;
                        g_hero.turnsBlind = 50;
                        message += "My eyes!! ";
                        break;
                    }
                }
                potionTypeKnown[item->symbol - 600] = true;
                if (item->count == 1) {
                    item.reset();
                } else {
                    --item->count;
                }
            }
            break;
        }
        case CONTROL_READ: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemScroll)
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to read?", 1);

            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';

            auto & item = inventory[intch];
            if (item and item->getType() == ItemScroll) {
                switch (dynamic_cast<Scroll &>(*item).effect) {
                    case 1: {
                        message += "You wrote this map. Why you read it, I don't know. ";
                        break;
                    }
                    case 2: {
                        clearRightPane();
                        termRend
                            .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                            .put("What do you want to identify?");

                        char in = termRead.readChar();
                        int intin = in - 'a';
                        auto & item2 = inventory[intin];
                        if (item2) {
                            if (item2->getType() == ItemPotion) {
                                potionTypeKnown[item2->symbol - 600] = true;
                            } else {
                                item2->showMdf = true;
                            }    
                        
                            if (item->count == 1) {
                                item.reset();
                            } else {
                                --item->count;
                            }
                        }
                        break;
                    }
                }
            }
            break;
        }
        case CONTROL_OPENBANDOLIER: {
            clearRightPane();
            termRend
                .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                .put("Here is your ammo.");
            int choice = 0;
            int num = 0;
            std::optional<int> takenFrom;
            while (true) {
                num = 0;
                for (int i = 0; i < BANDOLIER; i++) {
                    num += 2;

                    TextStyle style = TextStyle{ TerminalColor{} };
                    char symbol = '-';

                    if (inventory[AMMO_SLOT + i]) {
                        switch (inventory[AMMO_SLOT + i]->symbol) {
                            case 450:
                                style = TextStyle{ TextStyle::Bold, TerminalColor{ Color::Black } };
                                symbol = ',';
                                break;
                            case 451:
                                style = TextStyle{ TextStyle::Bold, TerminalColor{ Color::Red } };
                                symbol = ',';
                                break;
                        }
                    }
                    if (choice == i)
                        style += TextStyle::Underlined;

                    termRend
                        .setCursorPosition(Vec2i{ FIELD_COLS + num + 12, 1 })
                        .put(symbol, style);
                }
                char input = termRead.readChar();
                switch (input) {
                    case CONTROL_LEFT: {
                        if (choice > 0)
                            choice--;
                        break;
                    }
                    case CONTROL_RIGHT: {
                        if (choice < BANDOLIER - 1)
                            choice++;
                        break;
                    }
                    case CONTROL_EXCHANGE: {
                        if (takenFrom) {
                            std::swap(inventory[*takenFrom], inventory[AMMO_SLOT + choice]);
                            takenFrom = std::nullopt;
                        } else if (inventory[AMMO_SLOT + choice]) {
                            takenFrom = AMMO_SLOT + choice;
                        }
                        break;
                    }
                    case '\033': {
                        return;
                        break;
                    }
                }
            }
            break;
        }
        case CONTROL_RELOAD: {
            clearRightPane();
            if (heroWeapon == nullptr) {
                return;
            }
            termRend
                .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                .put("Now you can load your weapon");
            while (true) {
                for (int i = 0; i < heroWeapon->cartridgeSize; i++) {
                    TextStyle style{ TerminalColor{} };
                    char symbol = 'i';
                    if (heroWeapon->cartridge[i]) {
                        switch (heroWeapon->cartridge[i]->symbol) {
                            case 450:
                                style = TextStyle{ TextStyle::Bold, Color::Black };
                                break;
                            case 451:
                                style = TextStyle{ TextStyle::Bold, Color::Red };
                                break;
                            default:
                                symbol = '?';
                        }
                    } else {
                        symbol = '_';
                    }
                    termRend
                        .setCursorPosition(Vec2i{ FIELD_COLS + i + 10, 1 })
                        .put(symbol, style);
                }
                
                std::string loadString = "";
                
                for (int i = 0; i < BANDOLIER; i++) {
                    loadString += "[{}| "_format(i + 1);
                    auto & ammo = inventory[AMMO_SLOT + i];
                    if (not ammo) {
                        loadString += "nothing ]";
                        continue;
                    }
                    switch (ammo->symbol) {
                        case 450:
                            loadString += "steel bullets";
                            break;
                        case 451:
                            loadString += "shotgun shells";
                            break;
                        default:
                            loadString += "omgwth?";
                    }
                    loadString += " ]";
                }
                
                loadString += "   [u] - unload ";
                
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10, 2 })
                    .put(loadString);
                
                char in = termRead.readChar();
                if (in == '\033')
                    return;

                if (in == 'u') {
                    if (heroWeapon->currentCS == 0) {
                        continue;
                    }
                    bool found = false;
                    for (int j = 0; j < BANDOLIER; j++) {
                        auto & item = inventory[AMMO_SLOT + j];
                        if (item and item->symbol == heroWeapon->cartridge[heroWeapon->currentCS - 1]->symbol) {
                            heroWeapon->cartridge[heroWeapon->currentCS - 1].reset();
                            heroWeapon->currentCS--;
                            item->count++;
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        continue;
                    for (int j = 0; j < BANDOLIER; j++) {
                        auto & item = inventory[AMMO_SLOT + j];
                        if (not item) {
                            item = std::move(heroWeapon->cartridge[heroWeapon->currentCS - 1]);
                            heroWeapon->currentCS--;
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        continue;
                    for (auto & item : itemsMap[posH][posL]) {
                        if (item->symbol == heroWeapon->cartridge[heroWeapon->currentCS - 1]->symbol) {
                            item->count++;
                            heroWeapon->cartridge[heroWeapon->currentCS - 1].reset();
                            heroWeapon->currentCS--;
                            found = true;
                        }
                    }
                    if (not found) {
                        itemsMap[posH][posL].push_back(std::move(heroWeapon->cartridge[heroWeapon->currentCS - 1]));
                        heroWeapon->currentCS--;
                    }
                } else {
                    int intin = in - '1';
                    auto & item = inventory[AMMO_SLOT + intin];
                    if (item) {
                        if (heroWeapon->currentCS >= heroWeapon->cartridgeSize) {
                            message += "Weapon is loaded ";
                            return;
                        }
                        auto & slot = heroWeapon->cartridge[heroWeapon->currentCS];
                        if (item->count > 1) {
                            slot = std::make_unique<Ammo>(dynamic_cast<Ammo &>(*item));
                            slot->count = 1;
                            --item->count;
                        } else {
                            slot.reset(dynamic_cast<Ammo *>(item.release()));
                        }
                        heroWeapon->currentCS++;
                    }
                }
            }
            break;    
        }
    }
}

void Hero::attackEnemy(int row, int col) {
    if (heroWeapon) {
        unitMap[row][col].unit.uEnemy.dealDamage(heroWeapon->damage);
    }
    //} else if (heroWeapon->type == ItemTools) {
        //unitMap[posH + a1][posL + a2].getUnit().health -= heroWeapon->item.invTools.damage;
    //}
    if (unitMap[row][col].getUnit().health <= 0) {
        unitMap[row][col].getUnit().dropInventory();
        unitMap[row][col].type = UnitEmpty;
        xp += unitMap[row][col].unit.uEnemy.xpIncreasing;
    }
}

void Hero::throwAnimated(Item::Ptr item, Direction direction) {
    int throwDist = 0;
    auto offset = toVec2i(direction);
    char sym = getProjectileSymbol(direction);
    for (int i = 0; i < 12 - item->weight / 3; i++) {                        // 12 is "strength"
        int row = posH + offset.y * (i + 1);
        int col = posL + offset.x * (i + 1);

        if (map[row][col] == 2)
            break;

        if (unitMap[row][col].type != UnitEmpty) {
            unitMap[row][col].getUnit().health -= item->weight / 2;
            if (unitMap[row][col].getUnit().health <= 0) {
                unitMap[row][col].getUnit().dropInventory();
                unitMap[row][col].type = UnitEmpty;
                xp += unitMap[row][col].unit.uEnemy.xpIncreasing;
            }
            break;
        }
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
        throwDist++;
        sleep(DELAY);
    }
    itemsMap[posH + offset.y * throwDist][posL + offset.x * throwDist].push_back(std::move(item));
}

void Hero::shoot() {
    if (heroWeapon == nullptr or not heroWeapon->Ranged) {
        message += "You have no ranged weapon in hands. ";
        return;
    }
    if (heroWeapon->currentCS == 0) {
        message += "You have no bullets. ";
        g_stop = true;
        return;
    }
    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
        .put("In what direction? ");

    char choice = termRead.readChar();
    auto optdir = getDirectionByControl(choice);
    if (not optdir) {
        g_stop = true;
        return;
    }
    auto direction = *optdir;
    auto offset = toVec2i(direction);
    char sym = getProjectileSymbol(direction);
    int bulletPower = heroWeapon->cartridge[heroWeapon->currentCS - 1]->damage + g_hero.heroWeapon->damageBonus;

    for (int i = 1; i < heroWeapon->range + heroWeapon->cartridge[heroWeapon->currentCS - 1]->range; i++) {
        int row = posH + offset.y * i;
        int col = posL + offset.x * i; 

        if (map[row][col] == 2)
            break;

        if (unitMap[row][col].type != UnitEmpty) {
            unitMap[row][col].getUnit().dealDamage(bulletPower - i / 3);
            if (unitMap[row][col].getUnit().health <= 0) {
                unitMap[row][col].getUnit().dropInventory();
                unitMap[row][col].type = UnitEmpty;
                xp += unitMap[row][col].unit.uEnemy.xpIncreasing;
            }
        }
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
        sleep(DELAY / 3);
    }
    heroWeapon->cartridge[heroWeapon->currentCS - 1].reset();
    heroWeapon->currentCS--;
}

void Hero::moveHeroImpl(int row, int col) {
    if (row < 0 or row >= FIELD_ROWS or col < 0 or col >= FIELD_COLS)
        return;
    if (map[row][col] != 2 || canMoveThroughWalls) {
        if (unitMap[row][col].type == UnitEmpty) {
            unitMap[row][col] = unitMap[posH][posL];
            unitMap[posH][posL].type = UnitEmpty;
            posH = row;
            posL = col;
        } else if (unitMap[row][col].type == UnitEnemy) {
            attackEnemy(row, col);
        }
    } else if (map[row][col] == 2) {
        /*if (heroWeapon->type == ItemTools) {
            if (heroWeapon->item.invTools.possibility == 1) {
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
                    .put("Do you want to dig this wall? [yn]");

                char inpChar = termRead.readChar();
                if (inpChar == 'y' || inpChar == 'Y') {
                    map[row][col] = 1;
                    heroWeapon->item.invTools.uses--;
                    if (heroWeapon->item.invTools.uses <= 0) {
                        message += "Your {} is broken. "_format(heroWeapon->getItem().getName());
                        heroWeapon->type = ItemEmpty;
                        checkVisibleCells();
                    }
                    return;
                }
            }
        }*/
        message += "The wall is the way. ";
        g_stop = true;
    }
    checkVisibleCells();
}

PossibleUnit unitMap[FIELD_ROWS][FIELD_COLS];

PossibleUnit::PossibleUnit(UnitedUnits u, UnitType t): type(t) {
    switch (type) {
        case UnitEmpty:
            unit.uEmpty = u.uEmpty;
            break;
        case UnitHero:
            unit.uHero = u.uHero;
            break;
        case UnitEnemy:
            unit.uEnemy = u.uEnemy;
    }
}

PossibleUnit::PossibleUnit(const PossibleUnit& p) {
    type = p.type;
    switch (type) {
        case UnitEmpty:
            unit.uEmpty = p.unit.uEmpty;
            break;
        case UnitHero:
            unit.uHero = p.unit.uHero;
            break;
        case UnitEnemy:
            unit.uEnemy = p.unit.uEnemy;
    }
}

PossibleUnit& PossibleUnit::operator=(const PossibleUnit& p) {
    type = p.type;
    switch (type) {
        case UnitEmpty:
            unit.uEmpty = p.unit.uEmpty;
            break;
        case UnitHero:
            unit.uHero = p.unit.uHero;
            break;
        case UnitEnemy:
            unit.uEnemy = p.unit.uEnemy;
    }        
    return *this;
}

Unit& PossibleUnit::getUnit() {
    switch (type) {
        case UnitEmpty:
            return unit.uEmpty;
        case UnitHero:
            return unit.uHero;
        case UnitEnemy:
            return unit.uEnemy;
    }        
}

const Unit& PossibleUnit::getUnit() const {
    switch (type) {
        case UnitEmpty:
            return unit.uEmpty;
        case UnitHero:
            return unit.uHero;
        case UnitEnemy:
            return unit.uEnemy;
    }        
}
