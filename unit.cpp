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
    if (std::abs(dx) > std::abs(dy)) {
        double k = dy / dx;
        int s = sgn(dx);
        for (int i = 0; i * s < dx * s; i += s) {
            int x = fromX + i;
            int y = fromY + i * k;
            if (map[y][x] == 2) {
                return false;
            }
        }
    } else {
        double k = dx / dy;
        int s = sgn(dy);
        for (int i = 0; i * s < dy * s; i += s) {
            int x = fromX + i * k;
            int y = fromY + i;
            if (map[y][x] == 2) {
                return false;
            }
        }
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
    for (int i = 0; i < UNITINVENTORY; i++) {                                        /* Here are some changes, that we need to test */
        if (unitInventory[i].type != ItemEmpty) {
            if (unitInventory[i].getItem().isStackable) {
                auto it = findItemAtCell(posH, posL, unitInventory[i].getItem().symbol);
                if (it != end(itemsMap[posH][posL])) {
                    it->getItem().count += unitInventory[i].getItem().count;
                } else {
                    itemsMap[posH][posL].push_back(unitInventory[i]);
                }
            } else {
                itemsMap[posH][posL].push_back(unitInventory[i]);
            }
        }
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

Enemy differentEnemies[Enemy::TYPES_COUNT];

Enemy::Enemy(int eType) {
    switch (eType) {
        case 0: {
            health = 7;
            maxHealth = 7;
            unitInventory[0] = foodTypes[0];
            unitInventory[1] = weaponTypes[0];
            unitWeapon = &unitInventory[1];
            inventoryVol = 2;
            symbol = 201;
            vision = 16;
            xpIncreasing = 3;
            break;
        }
        case 1: {
            health = 10;
            maxHealth = 10;
            unitInventory[0] = weaponTypes[3];
            unitWeapon = &unitInventory[0];
            inventoryVol = 1;
            symbol = 202;
            vision = 10;
            xpIncreasing = 2;
            break;
        }
        case 2: {
            health = 5;
            maxHealth = 5;
            unitInventory[0] = weaponTypes[5];
            unitInventory[1] = ammoTypes[0];
            unitWeapon = &unitInventory[0];
            unitAmmo = &unitInventory[1];
            unitAmmo->item.invAmmo.count = rand() % 30 + 4;
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

void getProjectileDirectionsAndSymbol(Direction direction, int & dx, int & dy, char & sym) {
    switch (direction) {
    case Direction::Up:
        dy = -1;
        dx = 0;
        sym = '|';
        break;
    case Direction::UpRight:
        dy = -1;
        dx = 1;
        sym = '/';
        break;
    case Direction::Right:
        dx = 1;
        dy = 0;
        sym = '-';
        break;
    case Direction::DownRight:
        dx = 1;
        dy = 1;
        sym = '\\';
        break;
    case Direction::Down:
        dy = 1;
        dx = 0;
        sym = '|';
        break;
    case Direction::DownLeft:
        dy = 1;
        dx = -1;
        sym = '/';
        break;
    case Direction::Left:
        dx = -1;
        dy = 0;
        sym = '-';
        break;
    case Direction::UpLeft:
        dx = -1;
        dy = -1;
        sym = '\\';
    }
}

void Enemy::shoot() {
    if (posH == g_hero.posH && posL < g_hero.posL)
        dir = Direction::Right;
    else if (posH == g_hero.posH && posL > g_hero.posL)
        dir = Direction::Left;
    else if (posL == g_hero.posL && posH > g_hero.posH)
        dir = Direction::Up;
    else if (posL == g_hero.posL && posH < g_hero.posH)
        dir = Direction::Down;
    else if (posL > g_hero.posL && posH > g_hero.posH)
        dir = Direction::UpLeft;
    else if (posL > g_hero.posL && posH < g_hero.posH)
        dir = Direction::DownLeft;
    else if (posL < g_hero.posL && posH < g_hero.posH)
        dir = Direction::DownRight;
    else if (posL < g_hero.posL && posH > g_hero.posH)
        dir = Direction::UpRight;
    int dx;
    int dy;
    char sym;
    getProjectileDirectionsAndSymbol(dir, dx, dy, sym);
    for (int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++) {
        int row = posH + dy * i;
        int col = posL + dx * i;

        if (map[row][col] == 2)
            break;

        if (unitMap[row][col].type == UnitHero) {
            g_hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - g_hero.heroArmor->item.invArmor.defence) / 100.0);
            break;
        }
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
        sleep(DELAY / 3);
    }

    unitAmmo->item.invAmmo.count--;
    if (unitAmmo->item.invAmmo.count <= 0) {
        unitAmmo->type = ItemEmpty;
    }
}

Vec2i fromDirection(Direction dir) {
    switch (dir) {
        case Direction::Up: return Vec2i{ 0, -1 };
        case Direction::UpRight: return Vec2i{ 1, -1 };
        case Direction::Right: return Vec2i{ 1, 0 };
        case Direction::DownRight: return Vec2i{ 1, 1 };
        case Direction::Down: return Vec2i{ 0, 1 };
        case Direction::DownLeft: return Vec2i{ -1, 1 };
        case Direction::Left: return Vec2i{ -1, 0 };
        case Direction::UpLeft: return Vec2i{ -1, -1 };
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
        fromDirection(Direction::Up),
        fromDirection(Direction::Down),
        fromDirection(Direction::Right),
        fromDirection(Direction::Left)
    };
    if (g_mode == 2) {
        dirs.push_back(fromDirection(Direction::UpRight));
        dirs.push_back(fromDirection(Direction::UpLeft));
        dirs.push_back(fromDirection(Direction::DownRight));
        dirs.push_back(fromDirection(Direction::DownLeft));
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
    //int v_y = targetH, v_x = targetL;
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
                and unitWeapon->item.invWeapon.Ranged
                and unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range >= std::abs(g_hero.posH - posH) + std::abs(g_hero.posL - posL)) {
            shoot();
        } else {
            targetH = g_hero.posH;
            targetL = g_hero.posL;

            if (bfs(g_hero.posH, g_hero.posL, posH, posL, pH, pL) == -1) {
                canSeeHero = false;
            } else {
                if (unitMap[pH][pL].type == UnitEnemy) {
                    return;
                } else if (unitMap[pH][pL].type == UnitHero) {
                    if (unitWeapon->type == ItemWeapon) {
                        if (g_hero.heroArmor->item.invArmor.mdf != 2) {
                            g_hero.health -= unitWeapon->item.invWeapon.damage * ((100 - g_hero.heroArmor->item.invArmor.defence) / 100.0);
                        } else {
                            health -= unitWeapon->item.invWeapon.damage;
                        }
                    } else if (unitWeapon->type == ItemTools) {
                        if (g_hero.heroArmor->item.invArmor.mdf != 2) {
                            g_hero.health -= unitWeapon->item.invTools.damage * ((100 - g_hero.heroArmor->item.invArmor.defence) / 100.0);
                        } else {
                            health -= unitWeapon->item.invTools.damage;
                        }
                    }
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
                    if (unitMap[pH][pL].type == UnitHero) {
                        if (unitWeapon->type == ItemWeapon) {
                            if (g_hero.heroArmor->item.invArmor.mdf != 2) {
                                g_hero.health -= unitWeapon->item.invWeapon.damage * ((100 - g_hero.heroArmor->item.invArmor.defence) / 100.0);
                            } else {
                                health -= unitWeapon->item.invWeapon.damage;
                            }
                        } else if (unitWeapon->type == ItemTools) {
                            if (g_hero.heroArmor->item.invArmor.mdf != 2) {
                                g_hero.health -= unitWeapon->item.invTools.damage * ((100 - g_hero.heroArmor->item.invArmor.defence) / 100.0);
                            } else {
                                health -= unitWeapon->item.invTools.damage;
                            }
                        }
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
                if (unitMap[pH][pL].type == UnitHero) {
                    if (unitWeapon->type == ItemWeapon) {
                        if (g_hero.heroArmor->item.invArmor.mdf != 2) {
                            g_hero.health -= unitWeapon->item.invWeapon.damage * ((100 - g_hero.heroArmor->item.invArmor.defence) / 100.0);
                        } else {
                            health -= unitWeapon->item.invWeapon.damage;
                        }
                    } else if (unitWeapon->type == ItemTools) {
                        if (g_hero.heroArmor->item.invArmor.mdf != 2) {
                            g_hero.health -= unitWeapon->item.invTools.damage * ((100 - g_hero.heroArmor->item.invArmor.defence) / 100.0);
                        } else {
                            health -= unitWeapon->item.invTools.damage;
                        }
                    }
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
    heroWeapon = &inventory[Hero::EMPTY_SLOT];
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
        if (inventory[i].type != ItemEmpty)
            return false;
    }
    return true;
}

int Hero::findEmptyInventoryCell() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i].type == ItemEmpty)
            return i;
    }
    return 101010;                                            // Magic constant, means "Inventory is full".
}

int Hero::getInventoryItemsWeight() const {
    int toReturn = 0;
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i].type != ItemEmpty) {
            toReturn += inventory[i].getItem().weight;
        }
    }
    return toReturn;
}

void Hero::printList(const std::vector<PossibleItem> & items, std::string_view msg, int mode) const {
    int num = 0;

    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, num })
        .put(msg);

    num ++;
    switch (mode) {
        case 1: {
            for (int i = 0; i < items.size(); i++) {
                termRend.setCursorPosition(Vec2i{ FIELD_COLS + 10, num });
                if (items[i].getItem().showMdf == true && items[i].getItem().count == 1) {
                    if (items[i].getItem().attribute == 100) {
                        termRend.put("[{}] {} {{{}}}. "_format(
                                items[i].getItem().inventorySymbol,
                                items[i].getItem().getName(),
                                items[i].getItem().getMdf()));
                    } else {
                        termRend.put("[{}] {} ({}) {{{}}}. "_format(
                                items[i].getItem().inventorySymbol,
                                items[i].getItem().getName(),
                                items[i].getItem().getAttribute(),
                                items[i].getItem().getMdf()));
                    }
                } else if (items[i].getItem().count > 1) {
                    if (items[i].getItem().attribute == 100) {
                        termRend.put("[{}] {} {{{}}}. "_format(
                                items[i].getItem().inventorySymbol,
                                items[i].getItem().getName(),
                                items[i].getItem().count));
                    } else {
                        termRend.put("[{}] {} ({}) {{{}}}. "_format(
                                items[i].getItem().inventorySymbol,
                                items[i].getItem().getName(),
                                items[i].getItem().getAttribute(),
                                items[i].getItem().count));
                    }
                } else if (items[i].getItem().attribute == 100) {
                    termRend.put("[{}] {}. "_format(
                            items[i].getItem().inventorySymbol,
                            items[i].getItem().getName()));
                } else {
                    termRend.put("[{}] {} ({}). "_format(
                            items[i].getItem().inventorySymbol,
                            items[i].getItem().getName(),
                            items[i].getItem().getAttribute()));
                }
                num ++;
            }
            break;
        }
        case 2: {
            for (int i = 0; i < items.size(); i++)
            {
                termRend.setCursorPosition(Vec2i{ FIELD_COLS + 10, num });
                if (items[i].getItem().showMdf == true) {
                    termRend.put("[{}] {} ({}) {{{}}}. "_format(
                            i + 'a',
                            items[i].getItem().getName(),
                            items[i].getItem().getAttribute(),
                            items[i].getItem().getMdf()));
                } else {
                    termRend.put("[{}] {} ({}). "_format(
                            i + 'a',
                            items[i].getItem().getName(),
                            items[i].getItem().getAttribute()));
                }
                num ++;
            }
            break;

        }
    }
}

bool Hero::isMapInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i].type != ItemEmpty && inventory[i].getItem().symbol == 500)
            return true;
    }
    return false;
}

// 101010 something went wrong

int Hero::findAmmoInInventory() const {
    for (int i = 0; i < BANDOLIER; i++) {
        if (inventory[AMMO_SLOT + i].type == ItemAmmo) {
            return i;
        }
    }
    return 101010;
}

int Hero::findScrollInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i].type == ItemScroll) {
            return i;
        }
    }
    return 101010;
}

void Hero::printAmmoList(PossibleItem& pAmmo) {                                        // Picked ammo
    clearRightPane();
    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
        .put("In what slot do vou want to pull your ammo?");
    int choice = 0;
    int num = 0;
    while (1) {
        num = 0;
        for (int i = 0; i < BANDOLIER; i++) {
            termRend.setCursorPosition(Vec2i{ FIELD_COLS + num + 12, 1 });
            num += 2;
            char symbol = '-';
            TextStyle style{ TerminalColor{} };
            if (inventory[AMMO_SLOT + i].type == ItemAmmo) {
                switch (inventory[AMMO_SLOT + i].getItem().symbol) {
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
            case CONTROL_CONFIRM:
                if (inventory[AMMO_SLOT + choice].item.invAmmo.symbol != pAmmo.item.invAmmo.symbol && inventory[AMMO_SLOT + choice].type == ItemAmmo) {
                    PossibleItem buffer;
                    buffer = pAmmo;
                    pAmmo = inventory[AMMO_SLOT + choice];
                    inventory[AMMO_SLOT + choice] = buffer;
                } else if (inventory[AMMO_SLOT + choice].item.invAmmo.symbol == pAmmo.item.invAmmo.symbol && inventory[AMMO_SLOT + choice].type == ItemAmmo) {
                    inventory[AMMO_SLOT + choice].item.invAmmo.count += pAmmo.item.invAmmo.count;
                    pAmmo.type = ItemEmpty;
                } else if (inventory[AMMO_SLOT + choice].type == ItemEmpty) {
                    inventory[AMMO_SLOT + choice] = pAmmo;
                    pAmmo.type = ItemEmpty;
                }
                return;
                break;
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
        auto & itemToPick = itemsMap[posH][posL].front();
        message += "You picked up {}. "_format(itemToPick.getItem().getName());

        if (itemToPick.type == ItemAmmo) {
            printAmmoList(itemToPick);
            return;
        }

        bool canStack = false;

        if (itemToPick.getItem().isStackable) {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i) {
                if (inventory[i].type != ItemEmpty && inventory[i].getItem().symbol == itemToPick.getItem().symbol) {
                    canStack = true;
                    inventory[i].getItem().count += itemToPick.getItem().count;
                    itemsMap[posH][posL].pop_back();
                }
            }
        }

        if (not canStack) {
            int eic = findEmptyInventoryCell();
            if (eic != 101010) {
                inventory[eic] = itemToPick;
                inventory[eic].getItem().inventorySymbol = eic + 'a';
                log("Item index: '%c'\n", inventory[eic].getItem().inventorySymbol);
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
    
    std::vector<PossibleItem> list(
            std::begin(itemsMap[posH][posL]),
            std::end(itemsMap[posH][posL]));
    
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

    auto selected = std::begin(itemsMap[posH][posL]);
    std::advance(selected, intch);
    
    message += "You picked up %s. "_format(selected->getItem().getName());
    
    if (selected->type == ItemAmmo) {
        printAmmoList(*selected);
        return;
    }

    bool canStack = false;

    if (selected->getItem().isStackable) {
        for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i) {
            if (inventory[i].type != ItemEmpty && inventory[i].getItem().symbol == selected->getItem().symbol) {
                canStack = true;
                inventory[i].getItem().count += selected->getItem().count;
                itemsMap[posH][posL].erase(selected);
            }
        }
    }

    if (not canStack) {
        int eic = findEmptyInventoryCell();
        if (eic != 101010) {
            inventory[eic] = *selected;
            inventory[eic].getItem().inventorySymbol = eic + 'a';
            itemsMap[posH][posL].erase(selected);
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
        if (inventory[i].type == ItemFood)
            return true;
    }
    return false;
}

bool Hero::isArmorInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i].type == ItemArmor)
            return true;
    }
    return false;
}

bool Hero::isWeaponOrToolsInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i].type == ItemWeapon || inventory[i].type == ItemTools)
            return true;
    }
    return false;
}

bool Hero::isPotionInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i].type == ItemPotion)
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
        case CONTROL_UP: {
            a1 --;
            mHLogic(a1, a2);
            break;
        }
        case CONTROL_DOWN: {
            a1 ++;
            mHLogic(a1, a2);
            break;    
        }
        case CONTROL_LEFT: {
            a2 --;
            mHLogic(a1, a2);
            break;
        }
        case CONTROL_RIGHT: {
            a2 ++;
            mHLogic(a1, a2);
            break;
        }
        case CONTROL_UPLEFT: {
            a1 --;
            a2 --;
            mHLogic(a1, a2);
            break;    
        }
        case CONTROL_UPRIGHT: {
            a2 ++;
            a1 --;
            mHLogic(a1, a2);
            break;
        }
        case CONTROL_DOWNLEFT: {
            a1 ++;
            a2 --;
            mHLogic(a1, a2);
            break;
        }
        case CONTROL_DOWNRIGHT: {
            a1 ++;
            a2 ++;
            mHLogic(a1, a2);
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
            if (not heroWeapon->item.invWeapon.Ranged) {
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
                    itemsMap[1][1].push_back(foodTypes[0]);
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
    std::vector<PossibleItem> list;
    auto listInserter = std::back_inserter(list);
    switch (inp) {    
        case CONTROL_SHOWINVENTORY: {
            std::copy_if(inventory, inventory + MAX_USABLE_INV_SIZE, listInserter, [&] (const PossibleItem & item) {
                return item.type != ItemEmpty;
            });
            
            printList(list, "Here is your inventory.", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            break;
        }
        case CONTROL_EAT: {
            std::copy_if(inventory, inventory + MAX_USABLE_INV_SIZE, listInserter, [&] (const PossibleItem & item) {
                return item.type == ItemFood;
            });

            printList(list, "What do you want to eat?", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            if (inventory[intch].type == ItemFood) {
                int prob = rand() % g_hero.luck;
                if (prob == 0) {
                    hunger += inventory[intch].item.invFood.FoodHeal / 3;
                    health --;
                    message += "Fuck! This food was rotten! ";
                } else {
                    hunger += inventory[intch].item.invFood.FoodHeal;
                }
                if (inventory[intch].getItem().count == 1) {
                    inventory[intch].type = ItemEmpty;
                } else {
                    inventory[intch].getItem().count--;
                }
            }
            break;
        }    
        case CONTROL_WEAR: {
            std::copy_if(inventory, inventory + MAX_USABLE_INV_SIZE, listInserter, [&] (const PossibleItem & item) {
                return item.type == ItemArmor;
            });

            printList(list, "What do you want to wear?", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            if (inventory[intch].type == ItemArmor) {
                message += "Now you wearing {}. "_format(inventory[intch].getItem().getName());

                if (heroArmor->type != ItemEmpty) {
                    heroArmor->getItem().attribute = 100;
                }
                heroArmor = &inventory[intch];
                inventory[intch].getItem().attribute = 201;
            }
            break;
        }
        case CONTROL_DROP: {
            std::copy_if(inventory, inventory + MAX_USABLE_INV_SIZE, listInserter, [&] (const PossibleItem & item) {
                return item.type != ItemEmpty;
            });

            printList(list, "What do you want to drop?", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            if (choice == heroArmor->getItem().inventorySymbol)
                showInventory(CONTROL_TAKEOFF);
            if (choice == heroWeapon->getItem().inventorySymbol)
                showInventory(CONTROL_UNEQUIP);
            if (inventory[intch].getItem().isStackable && inventory[intch].getItem().count > 1) {
                clearRightPane();
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                    .put("How much items do you want to drop? [1-9]");

                int dropCount = clamp(1, termRead.readChar() - '0', inventory[intch].getItem().count);

                auto iter = findItemAtCell(posH, posL, inventory[intch].getItem().symbol);
                if (iter != end(itemsMap[posH][posL])) {        
                    iter->getItem().count += dropCount;
                } else {            
                    itemsMap[posH][posL].push_back(inventory[intch]);
                    itemsMap[posH][posL].back().getItem().count = dropCount;
                }
                inventory[intch].getItem().count -= dropCount;
                if (inventory[intch].getItem().count == 0) {
                    inventory[intch].type = ItemEmpty;
                }
            } else if (inventory[intch].getItem().isStackable && inventory[intch].getItem().count == 1) {
                auto iter = findItemAtCell(posH, posL, inventory[intch].getItem().symbol);
                if (iter != end(itemsMap[posH][posL])) {
                    iter->getItem().count++;
                    inventory[intch].getItem().count--;
                } else {            
                    itemsMap[posH][posL].push_back(inventory[intch]);
                }
                inventory[intch].type = ItemEmpty;
            } else {
                itemsMap[posH][posL].push_back(inventory[intch]);
                inventory[intch].type = ItemEmpty;
            }

            if (getInventoryItemsWeight() <= g_maxBurden && isBurdened) {
                message += "You are burdened no more. ";
                isBurdened = false;
            }

            break;
        }
        case CONTROL_TAKEOFF: {
            
            heroArmor->getItem().attribute = 100;
            heroArmor = &inventory[Hero::EMPTY_SLOT];
            break;
        
        }
        case CONTROL_WIELD: {
            std::copy_if(inventory, inventory + MAX_USABLE_INV_SIZE, listInserter, [&] (const PossibleItem & item) {
                return item.type == ItemWeapon or item.type == ItemTools;
            });

            printList(list, "What do you want to wield?", 1);
            
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            if (inventory[intch].type == ItemWeapon || inventory[intch].type == ItemTools) {
                message += "You wield {}. "_format(inventory[intch].getItem().getName());

                if (heroWeapon->type != ItemEmpty) {
                    heroWeapon->getItem().attribute = 100;
                }
                heroWeapon = &inventory[intch];
                inventory[intch].getItem().attribute = 301;
            }
    
            break;
        
        }
        case CONTROL_UNEQUIP: {
            heroWeapon->getItem().attribute = 100;
            heroWeapon = &inventory[Hero::EMPTY_SLOT];
            break;
        }
        case CONTROL_THROW: {
            std::copy_if(inventory, inventory + MAX_USABLE_INV_SIZE, listInserter, [&] (const PossibleItem & item) {
                return item.type != ItemEmpty;
            });

            printList(list, "What do you want to throw?", 1);

            char choice = termRead.readChar();
            if (choice == '\033') return;
            int intch = choice - 'a';

            if (inventory[intch].type != ItemEmpty) {
                clearRightPane();
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
                    .put("In what direction?");
                char secondChoise = termRead.readChar();

                if (inventory[intch].getItem().inventorySymbol == heroArmor->getItem().inventorySymbol)
                    showInventory(CONTROL_TAKEOFF);
                else if (inventory[intch].getItem().inventorySymbol == heroWeapon->getItem().inventorySymbol)
                    showInventory(CONTROL_UNEQUIP);

                throwAnimated(inventory[intch], getDirectionByControl(secondChoise));
            }
            break;
        }
        case CONTROL_DRINK: {
            std::copy_if(inventory, inventory + MAX_USABLE_INV_SIZE, listInserter, [&] (const PossibleItem & item) {
                return item.type == ItemPotion;
            });

            printList(list, "What do you want to drink?", 1);

            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';

            if (inventory[intch].type == ItemPotion) {
                switch (inventory[intch].item.invPotion.effect) {
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
                potionTypeKnown[inventory[intch].item.invPotion.symbol - 600] = true;
                if (inventory[intch].getItem().count == 1) {
                    inventory[intch].type = ItemEmpty;
                } else {
                    --inventory[intch].getItem().count;
                }
            }
            break;
        }
        case CONTROL_READ: {
            std::copy_if(inventory, inventory + MAX_USABLE_INV_SIZE, listInserter, [&] (const PossibleItem & item) {
                return item.type == ItemScroll;
            });

            printList(list, "What do you want to read?", 1);

            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';

            if (inventory[intch].type == ItemScroll) {
                switch (inventory[intch].item.invPotion.effect) {
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
                        if (inventory[intin].type != ItemEmpty) {
                            if (inventory[intin].type != ItemPotion) {
                                inventory[intin].getItem().showMdf = true;
                            } else if (inventory[intin].type == ItemPotion) {
                                potionTypeKnown[inventory[intin].getItem().symbol - 600] = true;
                            }    
                        
                            if (inventory[intch].getItem().count == 1) {
                                inventory[intch].type = ItemEmpty;
                            } else {
                                --inventory[intch].getItem().count;
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
            PossibleItem buffer;
            int pos;
            while (true) {
                num = 0;
                for (int i = 0; i < BANDOLIER; i++) {
                    num += 2;

                    TextStyle style = TextStyle{ TerminalColor{} };
                    char symbol = '-';

                    if (inventory[AMMO_SLOT + i].type == ItemAmmo) {
                        switch (inventory[AMMO_SLOT + i].getItem().symbol) {
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
                        if (buffer.type != ItemEmpty) {
                            inventory[pos] = inventory[AMMO_SLOT + choice];
                            inventory[AMMO_SLOT + choice] = buffer;
                            buffer.type = ItemEmpty;
                        } else {
                            buffer = inventory[AMMO_SLOT + choice];
                            inventory[AMMO_SLOT + choice].type = ItemEmpty;
                            pos = AMMO_SLOT + choice;
                        }
                        break;
                    }
                    case '\033': {
                        if (buffer.type != ItemEmpty) {
                            inventory[pos].type = ItemAmmo;
                            buffer.type = ItemEmpty;
                        }
                        return;
                        break;
                    }
                }
            }
            break;
        }
        case CONTROL_RELOAD: {
            clearRightPane();
            termRend
                .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                .put("Now you can load your weapon");
            while (true) {
                for (int i = 0; i < heroWeapon->item.invWeapon.cartridgeSize; i++) {
                    TextStyle style{ TerminalColor{} };
                    char symbol = 'i';
                    if (heroWeapon->item.invWeapon.cartridge[i].count == 1) {
                        switch (heroWeapon->item.invWeapon.cartridge[i].symbol) {
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
                    int ac = inventory[AMMO_SLOT + i].item.invArmor.count;
                    loadString += "[{}|"_format(i + 1);
                    if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
                        switch (inventory[AMMO_SLOT + i].getItem().symbol) {
                            case 450:
                                loadString += " steel bullets ";
                                break;
                            case 451:
                                loadString += " shotgun shells ";
                                break;
                            default:
                                loadString += " omgwth? ";
                        }
                        loadString += "]";
                    }
                    else loadString += " nothing ]";
                }
                
                loadString += "   [u] - unload ";
                
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10, 2 })
                    .put(loadString);
                
                char in = termRead.readChar();
                if (in == '\033')
                    return;

                if (in == 'u') {
                    if (heroWeapon->item.invWeapon.currentCS == 0) {
                        continue;
                    } else {
                        bool found = false;
                        for (int j = 0; j < BANDOLIER; j++) {
                            if (inventory[AMMO_SLOT + j].type == ItemAmmo && 
                                    inventory[AMMO_SLOT + j].getItem().symbol == 
                                    heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].symbol) {
                                heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
                                inventory[AMMO_SLOT + j].item.invAmmo.count++;
                                heroWeapon->item.invWeapon.currentCS--;
                                found = true;
                                break;
                            }
                        }
                        if (not found) {
                            for (int j = 0; j < BANDOLIER; j++) {
                                if (inventory[AMMO_SLOT + j].type == ItemEmpty) {
                                    inventory[AMMO_SLOT + j] = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1];
                                    inventory[AMMO_SLOT + j].type = ItemAmmo;
                                    heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
                                    heroWeapon->item.invWeapon.currentCS--;
                                    found = true;
                                    break;
                                }
                            }
                        }
                        if (not found) {
                            bool canStack = false;
                            for (auto & item : itemsMap[posH][posL]) {
                                if (item.type == ItemAmmo
                                        and item.getItem().symbol == heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].symbol) {
                                    item.item.invAmmo.count++;
                                    heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
                                    heroWeapon->item.invWeapon.currentCS--;
                                    canStack = true;
                                    found = true;
                                }
                            }
                            if (not canStack) {
                                itemsMap[posH][posL].push_back(heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1]);
                                heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
                                heroWeapon->item.invWeapon.currentCS--;
                                found = true;
                            }
                        }
                        if (not found) {
                            message += "You can`t unload your weapon. Idk, why. ";
                        }
                    }
                } else {
                    int intin = in - '1';
                    if (inventory[AMMO_SLOT + intin].type != ItemEmpty) {
                        if (heroWeapon->item.invWeapon.currentCS >= heroWeapon->item.invWeapon.cartridgeSize) {
                            message += "Weapon is loaded ";
                            return;
                        }
                        heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS] = inventory[AMMO_SLOT + intin].item.invAmmo;
                        heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS].count = 1;
                        heroWeapon->item.invWeapon.currentCS++;
                        if (inventory[AMMO_SLOT + intin].item.invAmmo.count > 1) {
                            inventory[AMMO_SLOT + intin].item.invAmmo.count --;
                        } else {
                            inventory[AMMO_SLOT + intin].type = ItemEmpty;
                        }
                    }
                }
            }
            break;    
        }
    }
}

void Hero::attackEnemy(int& a1, int& a2) {
    if (heroWeapon->type == ItemWeapon) {
        unitMap[posH + a1][posL + a2].getUnit().health -= heroWeapon->item.invWeapon.damage;
    } else if (heroWeapon->type == ItemTools) {
        unitMap[posH + a1][posL + a2].getUnit().health -= heroWeapon->item.invTools.damage;
    }
    if (unitMap[posH + a1][posL + a2].getUnit().health <= 0) {
        unitMap[posH + a1][posL + a2].getUnit().dropInventory();
        unitMap[posH + a1][posL + a2].type = UnitEmpty;
        xp += unitMap[posH + a1][posL + a2].unit.uEnemy.xpIncreasing;
    }
}

void Hero::throwAnimated(PossibleItem& item, Direction direction) {
    int throwDist = 0;
    int dx = 0;
    int dy = 0;
    char sym;
    getProjectileDirectionsAndSymbol(direction, dx, dy, sym);
    for (int i = 0; i < 12 - item.getItem().weight / 3; i++) {                        // 12 is "strength"
        int row = posH + dy * (i + 1);
        int col = posL + dx * (i + 1);

        if (map[row][col] == 2)
            break;

        if (unitMap[row][col].type != UnitEmpty) {
            unitMap[row][col].getUnit().health -= item.getItem().weight / 2;
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
    itemsMap[posH + dy * throwDist][posL + dx * throwDist].push_back(item);
    item.type = ItemEmpty;
}

void Hero::shoot() {
    if (heroWeapon->item.invWeapon.Ranged == false) {
        message += "You have no ranged weapon in hands. ";
        return;
    }
    if (heroWeapon->item.invWeapon.currentCS == 0) {
        message += "You have no bullets. ";
        g_stop = true;
        return;
    }
    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
        .put("In what direction? ");

    char choice = termRead.readChar();
    if (choice != CONTROL_UP 
            && choice != CONTROL_DOWN 
            && choice != CONTROL_LEFT 
            && choice != CONTROL_RIGHT 
            && choice != CONTROL_UPLEFT 
            && choice != CONTROL_UPRIGHT 
            && choice != CONTROL_DOWNLEFT
            && choice != CONTROL_DOWNRIGHT) {
        g_stop = true;
        return;
    }
    int dx = 0;
    int dy = 0;
    char sym;
    getProjectileDirectionsAndSymbol(getDirectionByControl(choice), dx, dy, sym);
    int bulletPower = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].damage + g_hero.heroWeapon->item.invWeapon.damageBonus;

    for (int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++) {
        int row = posH + dy * i;
        int col = posL + dx * i; 

        if (map[row][col] == 2)
            break;

        if (unitMap[row][col].type != UnitEmpty) {
            unitMap[row][col].getUnit().health -= bulletPower - i / 3;
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
    heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count = 0;
    heroWeapon->item.invWeapon.currentCS--;
}

void Hero::mHLogic(int& a1, int& a2) {
    if (map[posH + a1][posL + a2] != 2 || 
            (map[posH + a1][posL + a2] == 2 && canMoveThroughWalls) && 
            (posH + a1 > 0 && posH + a1 < FIELD_ROWS - 1 && posL + a2 > 0 && posL + a2 < FIELD_COLS - 1)) {
        if (unitMap[posH + a1][posL + a2].type == UnitEmpty) {
            unitMap[posH + a1][posL + a2] = unitMap[posH][posL];
            unitMap[posH][posL].type = UnitEmpty;
            posH += a1;
            posL += a2;
        } else if (unitMap[posH + a1][posL + a2].type == UnitEnemy) {
            attackEnemy(a1, a2);
        }
    } else if (map[posH + a1][posL + a2] == 2) {
        if (heroWeapon->type == ItemTools) {
            if (heroWeapon->item.invTools.possibility == 1) {
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
                    .put("Do you want to dig this wall? [yn]");

                char inpChar = termRead.readChar();
                if (inpChar == 'y' || inpChar == 'Y') {
                    map[posH + a1][posL + a2] = 1;
                    heroWeapon->item.invTools.uses--;
                    if (heroWeapon->item.invTools.uses <= 0) {
                        message += "Your {} is broken. "_format(heroWeapon->getItem().getName());
                        heroWeapon->type = ItemEmpty;
                        checkVisibleCells();
                    }
                    return;
                }
            }
        }
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
