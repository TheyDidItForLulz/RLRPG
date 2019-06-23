#include<units/hero.hpp>
#include<units/enemy.hpp>
#include<globals.hpp>

#include<fmt/core.h>
#include<fmt/printf.h>
#include <item_list_formatters.hpp>
#include <numeric>

using namespace fmt::literals;
using fmt::format;

Hero::Hero() {
    maxHealth = 15;
    health = 15;
    symbol = 200;
    vision = DEFAULT_VISION;

    inventory.add(std::make_unique<Armor>(armorTypes[1]));
    armor = dynamic_cast<Armor *>(&inventory['a']);
    if (std::rand() % (500 / luck) == 0)
        armor->mdf = 2;
}

int Hero::getLevelUpXP() const{
    return level * level + 4;
}

bool Hero::tryLevelUp() {
    if (xp < getLevelUpXP())
        return false;
    levelUp();
    return true;
}

void Hero::levelUp() {
    level++;
    message += format("Now you are level {}. ", level);
    maxBurden += maxBurden / 4;
    maxHealth += maxHealth / 4;
    health = maxHealth;
}

bool Hero::isInvisible() const {
    return turnsInvisible > 0;
}

void Hero::checkVisibleCells() {
    seenUpdated.forEach([this] (Coord2i pos, bool & see) {
        see = canSee(pos);
    });
}

int Hero::getInventoryItemsWeight() const {
    int totalWeight = 0;
    for (const auto & entry : inventory) {
        totalWeight += entry.second->getTotalWeight();
    }
    return totalWeight;
}

template<class Formatter>
void printList(std::string_view title, const std::vector<const Item *> & items, Formatter formatter) {
    termRend
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
            .put(title);

    int lineNo = 2;
    for (int i = 0; i < items.size(); i++) {
        termRend
                .setCursorPosition(Coord2i{ LEVEL_COLS + 10, lineNo })
                .put(formatter(i, *items[i]));
        lineNo ++;
    }
}

std::pair<Hero::SelectStatus, char> Hero::selectOneFromInventory(std::string_view title, std::function<bool(const Item &)> filter) const {
    std::vector<const Item *> items;
    for (const auto & entry : inventory)
        if (filter(*entry.second))
            items.push_back(entry.second);

    if (items.empty())
        return { NothingToSelect, 0 };

    std::sort(items.begin(), items.end(), [] (const Item * a, const Item * b) {
        return a->inventorySymbol < b->inventorySymbol;
    });

    printList(title, items, formatters::FromInventory{ weapon, armor });

    while (true) {
        char choice = termRead.readChar();
        if (choice == '\033')
            return { Cancelled, 0 };
        for (const Item * item : items)
            if (item->inventorySymbol == choice)
                return { Success, choice };
    }
}

std::pair<Hero::SelectStatus, std::vector<char>> Hero::selectMultipleFromInventory(
        std::string_view title,
        std::function<bool(const Item &)> filter) const {
    std::vector<const Item *> items;
    for (const auto & entry : inventory)
        if (filter(*entry.second))
            items.push_back(entry.second);

    if (items.empty())
        return { NothingToSelect, {} };

    std::sort(items.begin(), items.end(), [] (const Item * a, const Item * b) {
        return a->inventorySymbol < b->inventorySymbol;
    });

    std::vector<bool> selected(items.size());

    while (true) {
        printList(title, items, formatters::SelectMultipleFromInventory{selected, weapon, armor});

        char choice = termRead.readChar();
        if (choice == '\033')
            return {Cancelled, {}};
        if (choice == '\n') {
            std::vector<char> selectedIDs;
            for (int i = 0; i < items.size(); ++i)
                if (selected[i])
                    selectedIDs.push_back(items[i]->inventorySymbol);
            return {Success, std::move(selectedIDs)};
        }
        for (int i = 0; i < items.size(); ++i)
            if (items[i]->inventorySymbol == choice)
                selected[i] = not selected[i];
    }
}

std::pair<Hero::SelectStatus, int> Hero::selectOneFromList(std::string_view title, const std::vector<const Item *> & items) const {
    if (items.empty())
        return { NothingToSelect, 0 };

    printList(title, items, formatters::FromList{});

    while (true) {
        char choice = termRead.readChar();
        if (choice == '\033')
            return { Cancelled, 0 };
        if (not std::isalpha(choice))
            continue;
        int index = std::islower(choice) ? choice - 'a' : choice - 'A' + 26;
        if (index < items.size())
            return { Success, index };
    }
}

std::pair<Hero::SelectStatus, std::vector<int>> Hero::selectMultipleFromList(std::string_view title, const std::vector<const Item *> & items) const {
    if (items.empty())
        return { NothingToSelect, {} };

    std::vector<bool> selected(items.size());

    while (true) {
        printList(title, items, formatters::SelectMultipleFromList{ selected });

        char choice = termRead.readChar();
        if (choice == '\033')
            return { Cancelled, {} };
        if (choice == '\n') {
            std::vector<int> selectedIndices;
            for (int i = 0; i < items.size(); ++i)
                if (selected[i])
                    selectedIndices.push_back(i);
            return { Success, std::move(selectedIndices) };
        }
        if (not std::isalpha(choice))
            continue;
        int index = std::islower(choice) ? choice - 'a' : choice - 'A' + 26;
        if (index < items.size())
            selected[index] = not selected[index];
    }
}

bool Hero::isMapInInventory() const {
    for (const auto & entry : inventory)
        if (entry.second->symbol == 500)
            return true;
    return false;
}

void Hero::pickUp() {
    if (itemsMap[pos].empty()) {
        message += "There is nothing here to pick up. ";
        g_stop = true;
        return;
    }

    std::vector<ItemPileIter> iters;
    if (itemsMap[pos].size() == 1) {
        iters.push_back(itemsMap[pos].begin());
    } else {
        std::vector<const Item *> list;
        for (const auto & item : itemsMap[pos])
            list.push_back(item.get());

        auto [status, indices] = selectMultipleFromList("What do you want to pick up? ", list);
        if (status == Cancelled or indices.empty()) {
            g_stop = true;
            return;
        }
        std::vector<int> shifts(indices.size());
        std::adjacent_difference(indices.begin(), indices.end(), shifts.begin());

        auto it = itemsMap[pos].begin();
        for (int shift : shifts) {
            std::advance(it, shift);
            iters.push_back(it);
        }
    }

    bool fullInventory = false;
    bool firstPickUp = true;
    for (auto it : iters) {
        auto & itemToPick = *it;
        std::string pickUpString;

        inventory.add(std::move(itemToPick)).doIf<AddStatus::New>(
                [this, it, &pickUpString](AddStatus::New added) {
            itemsMap[pos].erase(it);
            auto & item = inventory[added.at];
            if (item.isStackable and item.count > 1)
                pickUpString = format("{}x {} ({})", item.count, item.getName(), added.at);
            else
                pickUpString = format("{} ({})", item.getName(), added.at);
        }).doIf<AddStatus::Stacked>([this, it, &pickUpString](AddStatus::Stacked stacked) {
            itemsMap[pos].erase(it);
            auto & item = inventory[stacked.at];

            std::string pickedCount;
            if (stacked.pickedCount > 1)
                pickedCount = fmt::format("{}x ", stacked.pickedCount);

            pickUpString = fmt::format("{}{} ({}), now you have {}",
                                 pickedCount, item.getName(), stacked.at, item.count);
        }).doIf<AddStatus::FullInvError>([&itemToPick, &fullInventory](auto & err) {
            itemToPick = std::move(err.item);
            fullInventory = true;
            message += "Your inventory is full";
        });

        if (fullInventory)
            break;

        if (firstPickUp) {
            message += "You picked up ";
        } else {
            message += ", ";
        }

        message += pickUpString;
        firstPickUp = false;
    }
    message += ". ";

    if (getInventoryItemsWeight() > maxBurden and !isBurdened) {
        message += "You're burdened. ";
        isBurdened = true;
    }
}

void Hero::clearRightPane() const {
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            termRend
                .setCursorPosition(Coord2i{ LEVEL_COLS + j + 10, i })
                .put(' ');
        }
    }
}

void Hero::eat() {
    auto [status, choice] = selectOneFromInventory("What do you want to eat?", [] (const Item & item) {
        return item.getType() == Item::Type::Food;
    });
    switch (status) {
        case NothingToSelect:
            message += "You don't have anything to eat. ";
            g_stop = true;
            return;
        case Cancelled:
            g_stop = true;
            return;
        default:break;
    }

    auto & item = inventory[choice];
    int prob = std::rand() % g_hero->luck;
    if (prob == 0) {
        hunger += dynamic_cast<Food &>(item).nutritionalValue / 3;
        health --;
        message += "Fuck! This food was rotten! ";
    } else {
        hunger += dynamic_cast<Food &>(item).nutritionalValue;
    }
    if (item.count == 1) {
        inventory.remove(choice);
    } else {
        item.count--;
    }
}

void Hero::processInput(char inp) {
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
            moveTo(pos + offset);
            break;
        }
        case CONTROL_PICKUP:
            pickUp();
            break;
        case CONTROL_EAT:
            eat();
            break;
        case CONTROL_SHOWINVENTORY:
            showInventory();
            break;                
        case CONTROL_WEAR:
            wearArmor();
            break;
        case CONTROL_WIELD:
            wieldWeapon();
            break;
        case CONTROL_TAKEOFF:
            if (armor == nullptr)
                g_stop = true;
            else
                takeArmorOff();
            break;
        case CONTROL_UNEQUIP:
            if (weapon == nullptr)
                g_stop = true;
            else
                unequipWeapon();
            break;
        case CONTROL_DROP:
            dropItems();
            break;
        case CONTROL_THROW:
            throwItem();
            break;
        case CONTROL_SHOOT:
            shoot();
            break;
        case CONTROL_DRINK:
            drinkPotion();
            break;
        case CONTROL_RELOAD:
            reloadWeapon();
            break;
        case CONTROL_READ:
            readScroll();
            break;
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
                    itemsMap.at(1, 1).push_back(std::make_unique<Food>(foodTypes[0]));
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
        default:
            break;
    }
}

void Hero::reloadWeapon() {
    if (weapon == nullptr or not weapon->isRanged) {
        message += "You have no ranged weapon in hands. ";
        g_stop = true;
        return;
    }

    clearRightPane();
    termRend
        .setCursorPosition(Coord2i{ LEVEL_COLS + 10 })
        .put("Now you can load your weapon");

    while (true) {
        clearRightPane();
        termRend
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 1 })
            .put('[');

        for (int i = 0; i < weapon->cartridge.getCapacity(); i++) {
            TextStyle style{ TerminalColor{} };
            char symbol = 'i';
            if (weapon->cartridge[i]) {
                switch (weapon->cartridge[i]->symbol) {
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
            termRend.put(symbol, style);
        }
        termRend.put(']');

        int lineY = 2;
        for (auto entry : inventory) {
            if (entry.second->getType() != Item::Type::Ammo)
                continue;
            
            std::string line = format("{} - {} (x{})",
                entry.first,
                entry.second->getName(),
                entry.second->count);

            termRend
                .setCursorPosition(Coord2i{ LEVEL_COLS + 10, lineY })
                .put(line);
            
            ++lineY;
        }

        ++lineY;
        termRend
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10, lineY })
            .put("Press '-' to unload one.");
        
        char chToLoad;
        while (true) {
            chToLoad = termRead.readChar();
            if (chToLoad == '\033')
                return;
            if (chToLoad == '-' or inventory.hasID(chToLoad))
                break;
        }

        if (chToLoad == '-') {
            auto bullet = weapon->cartridge.unloadOne();
            if (bullet) {
                inventory.add(std::move(bullet)).doIf<AddStatus::FullInvError>([&] (auto & err) {
                    drop(std::move(err.item), pos);
                });
            }
        } else {
            auto & item = inventory[chToLoad];
            if (item.getType() == Item::Type::Ammo) {
                if (weapon->cartridge.isFull()) {
                    message += "Weapon is loaded ";
                    return;
                }

                Ammo::Ptr bullet;
                if (item.count == 1) {
                    auto itemptr = inventory.remove(item.inventorySymbol).release();
                    bullet.reset(dynamic_cast<Ammo *>(itemptr));
                } else {
                    bullet = std::make_unique<Ammo>(dynamic_cast<Ammo &>(item));
                    bullet->count = 1;
                    --item.count;
                }
                weapon->cartridge.load(std::move(bullet));
            }
        }
    }
}

void Hero::showInventory() {
    g_stop = true;
    if (inventory.isEmpty()) {
        message += "Your inventory is empty.";
        return;
    }
    std::vector<const Item *> list;
    for (const auto & entry : inventory)
        list.push_back(entry.second);

    printList("Here is your inventory.", list, formatters::FromInventory{ weapon, armor });
    termRead.readChar();
}

void Hero::wearArmor() {
    auto [status, choice] = selectOneFromInventory("What do you want to wear?");
    switch (status) {
        case NothingToSelect:
            message += "You don't have anything to wear. ";
            g_stop = true;
            return;
        case Cancelled:
            g_stop = true;
            return;
        default:break;
    }
    auto & item = inventory[choice];
    message += format("Now you wearing {}. ", item.getName());
    armor = dynamic_cast<Armor *>(&item);
}

void Hero::dropItems() {
    auto [status, choice] = selectOneFromInventory("What do you want to drop?");
    switch (status) {
        case NothingToSelect:
            message += "You don't have anything to drop. ";
            g_stop = true;
            return;
        case Cancelled:
            g_stop = true;
            return;
        default:break;
    }

    if (armor != nullptr and choice == armor->inventorySymbol)
        takeArmorOff();
    if (weapon != nullptr and choice == weapon->inventorySymbol)
        unequipWeapon();

    auto & item = inventory[choice];
    int dropCount = 1;
    if (item.count != 1) {
        clearRightPane();
        int maxCount = std::min(item.count, 9);
        termRend
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
            .put(format("How much items do you want to drop? [1-{}]", maxCount))
            .display();

        dropCount = clamp(1, termRead.readChar() - '0', item.count);
    }
    drop(item.splitStack(dropCount), pos);
    if (item.count == 0)
        inventory.remove(choice);

    if (getInventoryItemsWeight() <= maxBurden and isBurdened) {
        message += "You are burdened no more. ";
        isBurdened = false;
    }
}

void Hero::wieldWeapon() {
    auto [status, itemID] = selectOneFromInventory("What do you want to wield?", [] (const Item & item) {
        return item.getType() == Item::Type::Weapon;
    });
    switch (status) {
        case NothingToSelect:
            message += "You don't have anything to wield. ";
            g_stop = true;
            return;
        case Success: {
            auto & item = inventory[itemID];
            message += format("You wield {}. ", item.getName());
            weapon = dynamic_cast<Weapon *>(&item);
            break;
        }
        case Cancelled:
            g_stop = true;
            break;
        default:break;
    }
}

void Hero::throwItem() {
    auto [status, itemID] = selectOneFromInventory("What do you want to throw?");
    switch (status) {
        case NothingToSelect:
            message += "You don't have anything to throw. ";
            g_stop = true;
            return;
        case Cancelled:
            g_stop = true;
            return;
        default:
            break;
    }
    auto & item = inventory[itemID];
    int throwCount = 1;
    if (item.count > 1) {
        clearRightPane();
        int maxCount = std::min(item.count, 9);
        termRend
                .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
                .put(format("How many items do you want to throw? [1-{}]", maxCount));

        while (true) {
            char countChoice = termRead.readChar();
            if (countChoice == '\033')
                return;

            throwCount = countChoice - '0';
            if (throwCount > 0 and throwCount <= maxCount)
                break;
        }
    }

    clearRightPane();
    termRend
        .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
        .put("In what direction?");
    
    Direction throwDir;
    while (true) {
        char dirChoice = termRead.readChar();
        if (dirChoice == '\033')
            return;

        auto optdir = getDirectionByControl(dirChoice);
        if (optdir) {
            throwDir = optdir.value();
            break;
        }
    }

    if (armor != nullptr and item.inventorySymbol == armor->inventorySymbol)
        takeArmorOff();
    else if (weapon != nullptr and item.inventorySymbol == weapon->inventorySymbol)
        unequipWeapon();

    auto itemToThrow = item.splitStack(throwCount);
    if (item.count == 0)
        inventory.remove(itemID);

    throwAnimated(std::move(itemToThrow), throwDir);
}

void Hero::drinkPotion() {
    auto [status, itemID] = selectOneFromInventory("What do you want to drink?", [] (const Item & item) {
        return item.getType() == Item::Type::Potion;
    });
    switch (status) {
        case NothingToSelect:
            message += "You don't have anything to drink. ";
            g_stop = true;
            return;
        case Cancelled:
            g_stop = true;
            return;
        default:break;
    }

    auto & item = inventory[itemID];
    auto & potion = dynamic_cast<Potion &>(item);
    switch (potion.effect) {
        case 1:
            heal(3);
            message += "Now you feeling better. ";
            break;
        case 2:
            g_hero->turnsInvisible = 150;
            message += "Am I invisible? Oh, lol! ";
            break;
        case 3:
            while (true) {
                Coord2i pos = { std::rand() % LEVEL_COLS, std::rand() % LEVEL_ROWS };
                if (::level[pos] != 2 and not unitMap[pos]) {
                    setTo(pos);
                    break;
                }
            }
            message += "Teleportation is so straaange thing! ";
            break;
        case 4:
            message += "Well.. You didn't die. Nice. ";
            break;
        case 5:
            vision = 1;
            g_hero->turnsBlind = 50;
            message += "My eyes!! ";
            break;
        default:
            throw std::logic_error("Unknown potion id");
    }
    potionTypeKnown[item.symbol - 600] = true;
    if (item.count == 1) {
        inventory.remove(itemID);
    } else {
        --item.count;
    }
}

void Hero::readScroll() {
    auto [status, itemID] = selectOneFromInventory("What do you want to read?", [] (const Item & item) {
        return item.getType() == Item::Type::Scroll;
    });
    switch (status) {
        case NothingToSelect:
            message += "You don't have anything to read. ";
            g_stop = true;
            return;
        case Cancelled:
            g_stop = true;
            return;
        default:
            break;
    }

    auto & item = inventory[itemID];
    switch (dynamic_cast<Scroll &>(item).effect) {
        case 1:
            message += "You wrote this map. Why you read it, I don't know. ";
            break;
        case 2: {
            auto [status, chToApply] = selectOneFromInventory("What do you want to identify?", [] (const Item & item) {
                if (item.getType() == Item::Type::Potion) {
                    if (not potionTypeKnown[item.symbol - 600])
                        return true;
                } else if (not item.showMdf){
                    return true;
                }
                return false;
            });
            switch (status) {
                case NothingToSelect:
                    message += "You have nothing to identify. ";
                    g_stop = true;
                    return;
                case Cancelled:
                    g_stop = true;
                    return;
                default:
                    break;
            }

            auto & item2 = inventory[chToApply];
            if (item2.getType() == Item::Type::Potion) {
                potionTypeKnown[item2.symbol - 600] = true;
            } else {
                item2.showMdf = true;
            }

            if (item.count == 1) {
                inventory.remove(itemID);
            } else {
                --item.count;
            }
            break;
        }
        default:
            throw std::logic_error("Unknown scroll id");
    }
}

void Hero::attackEnemy(Coord2i cell) {
    auto & enemy = dynamic_cast<Enemy &>(*unitMap[cell]);
    if (weapon) {
        enemy.dealDamage(weapon->damage);
    }
    if (enemy.health <= 0) {
        enemy.dropInventory();
        xp += enemy.xpCost;
        unitMap[cell].reset();
    }
}

void Hero::throwAnimated(Item::Ptr item, Direction direction) {
    int throwDist = 0;
    auto offset = toVec2i(direction);
    char sym = toChar(direction);
    for (int i = 0; i < 12 - item->getTotalWeight() / 3; i++) {                        // 12 is "strength"
        auto cell = pos + offset * (i + 1);

        if (::level[cell] == 2)
            break;

        if (unitMap[cell]) {
            unitMap[cell]->dealDamage(item->getTotalWeight() / 2);
            if (unitMap[cell]->health <= 0) {
                auto & enemy = dynamic_cast<Enemy &>(*unitMap[cell]);
                enemy.dropInventory();
                xp += enemy.xpCost;
                unitMap[cell].reset();
            }
            break;
        }
        termRend
            .setCursorPosition(cell)
            .put(sym)
            .display();
        throwDist++;
        sleep(DELAY);
    }
    drop(std::move(item), pos + offset * throwDist);
}

void Hero::shoot() {
    if (weapon == nullptr or not weapon->isRanged) {
        message += "You have no ranged weapon in hands. ";
        return;
    }
    if (weapon->cartridge.isEmpty()) {
        message += "You have no bullets. ";
        g_stop = true;
        return;
    }
    termRend
        .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
        .put("In what direction? ");

    char choice = termRead.readChar();
    auto optdir = getDirectionByControl(choice);
    if (not optdir) {
        g_stop = true;
        return;
    }
    auto direction = *optdir;
    auto offset = toVec2i(direction);
    char sym = toChar(direction);
    int bulletPower = weapon->cartridge.next().damage + g_hero->weapon->damageBonus;

    for (int i = 1; i < weapon->range + weapon->cartridge.next().range; i++) {
        auto cell = pos + offset * i;

        if (::level[cell] == 2)
            break;

        if (unitMap[cell]) {
            unitMap[cell]->dealDamage(bulletPower - i / 3);
            if (unitMap[cell]->health <= 0) {
                auto & enemy = dynamic_cast<Enemy &>(*unitMap[cell]);
                enemy.dropInventory();
                xp += enemy.xpCost;
                unitMap[cell].reset();
            }
        }
        termRend
            .setCursorPosition(cell)
            .put(sym)
            .display();
        sleep(DELAY / 3);
    }
    weapon->cartridge.unloadOne();
}

void Hero::moveTo(Coord2i cell) {
    if (not ::level.isIndex(cell))
        return;
    if (::level[cell] != 2 or canMoveThroughWalls) {
        if (unitMap[cell] and unitMap[cell]->getType() == Unit::Type::Enemy) {
            attackEnemy(cell);
        } else if (not unitMap[cell]) {
            setTo(cell);
        }
    } else if (::level[cell] == 2) {
        if (weapon != nullptr and weapon->canDig) {
            termRend
                .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
                .put("Do you want to dig this wall? [yn]");

            char inpChar = termRead.readChar();
            if (inpChar == 'y' or inpChar == 'Y') {
                ::level[cell] = 1;
                if (std::rand() % 100 <= Hero::MAX_LUCK - luck) {
                    message += format("You've broken your {}. ", weapon->getName());
                    char weaponID = weapon->inventorySymbol;
                    unequipWeapon();
                    inventory.remove(weaponID);
                }
                return;
            }
        }
        message += "The wall is the way. ";
        g_stop = true;
    }
}
