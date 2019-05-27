#include<hero.hpp>
#include<enemy.hpp>
#include<globals.hpp>

#include<fmt/core.h>
#include<fmt/printf.h>

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

void Hero::dealDamage(int damage) {
    int defence = 0;
    if (armor != nullptr)
        defence = armor->defence;
    health -= damage * (100 - defence) / 100.f;
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
        totalWeight += entry.second->weight * entry.second->count;
    }
    return totalWeight;
}

void Hero::printList(std::vector<Item *> items, std::string_view msg, int mode) const {
    int lineNo = 0;

    termRend
        .setCursorPosition(Coord2i{ LEVEL_COLS + 10, lineNo })
        .put(msg);
    lineNo ++;

    if (mode == 1) {
        std::sort(items.begin(), items.end(), [] (Item * a, Item * b) {
            return a->inventorySymbol < b->inventorySymbol;
        });
    }
    for (int i = 0; i < items.size(); i++) {
        char charid;
        if (mode == 1) {
            charid = items[i]->inventorySymbol;
        } else if (i < 26) {
            charid = 'a' + i;
        } else if (i < 52) {
            charid = 'A' + (i - 26);
        } else {
            termRend
                .setCursorPosition(Coord2i{ LEVEL_COLS + 10, lineNo })
                .put("... and others ...");
            break;
        }
        std::string id = format("{} -", charid);
        std::string name = format(" {}", items[i]->getName());

        std::string count;
        if (items[i]->count > 1)
            count = format(" {}x", items[i]->count);
        
        std::string modifier;
        if (items[i]->showMdf)
            modifier = format(" {{{}}}", items[i]->getMdf());
        
        std::string equipped;
        if (mode == 1) {
            if (items[i] == weapon)
                equipped = " (being wielded)";
            else if (items[i] == armor)
                equipped = " (being worn)";
        }
        
        termRend
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10, lineNo })
            .put(id + count + name + modifier + equipped);
        lineNo ++;
    }
}

bool Hero::isMapInInventory() const {
    for (const auto & entry : inventory)
        if (entry.second->symbol == 500)
            return true;
    return false;
}

std::optional<char> Hero::findAmmoInInventory() const {
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Ammo)
            return entry.first;
    return std::nullopt;
}

std::optional<char> Hero::findScrollInInventory() const {
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Scroll)
            return entry.first;
    return std::nullopt;
}

void Hero::pickUp() {
    if (itemsMap[pos].empty()) {
        message += "There is nothing here to pick up. ";
        g_stop = true;
        return;
    }

    auto it = itemsMap[pos].begin();
    if (itemsMap[pos].size() > 1) {
        std::vector<Item *> list;
        for (const auto & item : itemsMap[pos])
            list.push_back(item.get());

        printList(list, "What do you want to pick up? ", 2);

        int intch;
        while (true) {
            char choice = termRead.readChar();
            if (choice == '\033')
                return;

            intch = choice - 'a';
            if (intch >= 0 or intch < itemsMap[pos].size())
                break;
        }

        std::advance(it, intch);
    }
    auto & itemToPick = *it;

    inventory.add(std::move(itemToPick)).doIf<AddStatus::New>([this, it] (AddStatus::New added) {
        itemsMap[pos].erase(it);
        auto & item = inventory[added.at];
        if (item.isStackable and item.count > 1)
            message += format("You picked up {}x {} ({}). ", item.count, item.getName(), added.at);
        else
            message += format("You picked up {} ({}). ", item.getName(), added.at);
    }).doIf<AddStatus::Stacked>([this, it] (AddStatus::Stacked stacked) {
        itemsMap[pos].erase(it);
        auto & item = inventory[stacked.at];

        std::string pickedCount;
        if (stacked.pickedCount > 1)
            pickedCount = fmt::format("{}x ", stacked.pickedCount);

        message += fmt::format("You picked up {}{} ({}), now you have {}. ",
            pickedCount, item.getName(), stacked.at, item.count);
    }).doIf<AddStatus::FullInvError>([&itemToPick] (auto & err) {
        itemToPick = std::move(err.item);
        message += "Your inventory is full. ";
    });

    if (getInventoryItemsWeight() > maxBurden and !isBurdened) {
        message += "You're burdened. ";
        isBurdened = true;
    }

    return;
}

bool Hero::isFoodInInventory() const {
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Food)
            return true;
    return false;
}

bool Hero::isArmorInInventory() const {
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Armor)
            return true;
    return false;
}

bool Hero::isWeaponInInventory() const {
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Weapon)
            return true;
    return false;
}

bool Hero::isPotionInInventory() const {
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Potion)
            return true;
    return false;
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
    if (not isFoodInInventory()) {
        message += "You don't have anything to eat. ";
        return;
    }

    std::vector<Item *> list;
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Food)
            list.push_back(entry.second);

    printList(list, "What do you want to eat?", 1);
    char choice = termRead.readChar();
    if (choice == '\033')
        return;
    auto & item = inventory[choice];
    if (item.getType() == Item::Type::Food) {
        int prob = rand() % g_hero->luck;
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
    std::vector<Item *> list;
    for (const auto & entry : inventory)
        list.push_back(entry.second);
    
    printList(list, "Here is your inventory.", 1);
    termRead.readChar();
}

void Hero::wearArmor() {
    std::vector<Item *> list;
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Armor)
            list.push_back(entry.second);
    if (list.empty()) {
        message += "You don't have anything to wear. ";
        g_stop = true;
        return;
    }

    printList(list, "What do you want to wear?", 1);
    char choice = termRead.readChar();
    if (choice == '\033')
        return;
    auto & item = inventory[choice];
    if (item.getType() == Item::Type::Armor) {
        message += format("Now you wearing {}. ", item.getName());

        armor = dynamic_cast<Armor *>(&item);
    }
}

void Hero::dropItems() {
    if (inventory.isEmpty()) {
        message += "You don't have anything to drop. ";
        g_stop = true;
        return;
    }
    std::vector<Item *> list;
    for (const auto & entry : inventory)
        list.push_back(entry.second);

    printList(list, "What do you want to drop?", 1);
    char choice;
    while (true) {
        choice = termRead.readChar();
        if (choice == '\033')
            return;
        if (inventory.hasID(choice))
            break;
    }
    if (armor != nullptr and choice == armor->inventorySymbol)
        takeArmorOff();
    if (weapon != nullptr and choice == weapon->inventorySymbol)
        unequipWeapon();
    auto & item = inventory[choice];
    if (not item.isStackable or item.count == 1) {
        drop(inventory.remove(choice), pos);
    } else {
        clearRightPane();
        termRend
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10 })
            .put("How much items do you want to drop? [1-9]");

        int dropCount = clamp(1, termRead.readChar() - '0', item.count);

        auto iter = findItemAt(pos, item.symbol);
        if (iter != end(itemsMap[pos])) {        
            (*iter)->count += dropCount;
        } else {            
            itemsMap[pos].push_back(item.clone());
            itemsMap[pos].back()->count = dropCount;
        }
        item.count -= dropCount;
        if (item.count == 0) {
            inventory.remove(choice);
        }
    }

    if (getInventoryItemsWeight() <= maxBurden and isBurdened) {
        message += "You are burdened no more. ";
        isBurdened = false;
    }
}

void Hero::wieldWeapon() {
    std::vector<Item *> list;
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Weapon)
            list.push_back(entry.second);
    if (list.empty()) {
        message += "You don't have anything to wield. ";
        g_stop = true;
        return;
    }

    printList(list, "What do you want to wield?", 1);
    
    char choice;
    while (true) {
        choice = termRead.readChar();
        if (choice == '\033')
            return;
        if (inventory.hasID(choice))
            break;
    }
    auto & item = inventory[choice];
    if (item.getType() == Item::Type::Weapon) {
        message += format("You wield {}. ", item.getName());

        weapon = dynamic_cast<Weapon *>(&item);
    }
}

void Hero::throwItem() {
    if (inventory.isEmpty()) {
        message += "You don't have anything to throw. ";
        g_stop = true;
        return;
    }
    std::vector<Item *> list;
    for (const auto & entry : inventory)
        list.push_back(entry.second);

    printList(list, "What do you want to throw?", 1);

    char choice;
    while (true) {
        choice = termRead.readChar();
        if (choice == '\033')
            return;
        if (inventory.hasID(choice))
            break;
    }

    auto & item = inventory[choice];

    clearRightPane();
    termRend
        .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
        .put("In what direction?");
    
    Direction throwDir;
    while (true) {
        char choice = termRead.readChar();
        if (choice == '\033')
            return;
        auto optdir = getDirectionByControl(choice);
        if (optdir) {
            throwDir = optdir.value();
            break;
        }
    }

    if (armor != nullptr and item.inventorySymbol == armor->inventorySymbol)
        takeArmorOff();
    else if (weapon != nullptr and item.inventorySymbol == weapon->inventorySymbol)
        unequipWeapon();

    throwAnimated(inventory.remove(choice), throwDir);
}

void Hero::drinkPotion() {
    std::vector<Item *> list;
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Potion)
            list.push_back(entry.second);
    if (list.empty()) {
        message += "You don't have anything to drink. ";
        g_stop = true;
        return;
    }

    printList(list, "What do you want to drink?", 1);

    char choice;
    while (true) {
        choice = termRead.readChar();
        if (choice == '\033')
            return;
        if (inventory.hasID(choice))
            break;
    }

    auto & item = inventory[choice];
    if (item.getType() == Item::Type::Potion) {
        auto & potion = dynamic_cast<Potion &>(item);
        switch (potion.effect) {
            case 1: {
                heal(3);
                message += "Now you feeling better. ";
                break;
            }
            case 2: {
                g_hero->turnsInvisible = 150;
                message += "Am I invisible? Oh, lol! ";
                break;
            }
            case 3: {
                while (true) {
                    Coord2i pos = { std::rand() % LEVEL_ROWS, std::rand() % LEVEL_COLS };
                    if (::level[pos] != 2 and not unitMap[pos]) {
                        setTo(pos);
                        break;
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
                vision = 1;
                g_hero->turnsBlind = 50;
                message += "My eyes!! ";
                break;
            }
        }
        potionTypeKnown[item.symbol - 600] = true;
        if (item.count == 1) {
            inventory.remove(choice);
        } else {
            --item.count;
        }
    }
}

void Hero::readScroll() {
    std::vector<Item *> list;
    for (const auto & entry : inventory)
        if (entry.second->getType() == Item::Type::Scroll)
            list.push_back(entry.second);
    if (list.empty()) {
        message += "You don't have anything to read. ";
        g_stop = true;
        return;
    }

    printList(list, "What do you want to read?", 1);

    char choice;
    while (true) {
        choice = termRead.readChar();
        if (choice == '\033')
            return;
        if (inventory.hasID(choice))
            break;
    }

    auto & item = inventory[choice];
    if (item.getType() == Item::Type::Scroll) {
        switch (dynamic_cast<Scroll &>(item).effect) {
            case 1: {
                message += "You wrote this map. Why you read it, I don't know. ";
                break;
            }
            case 2: {
                clearRightPane();
                list.clear();
                for (const auto & entry : inventory) {
                    if (entry.second->getType() == Item::Type::Potion) {
                        if (not potionTypeKnown[entry.second->symbol - 600])
                            list.push_back(entry.second);
                    } else if (not entry.second->showMdf) {
                        list.push_back(entry.second);
                    }
                }

                printList(list, "What do you want to identify?", 1);

                char chToApply;
                while (true) {
                    chToApply = termRead.readChar();
                    if (chToApply == '\033')
                        return;
                    if (inventory.hasID(chToApply))
                        break;
                }

                auto & item2 = inventory[chToApply];
                if (item2.getType() == Item::Type::Potion) {
                    potionTypeKnown[item2.symbol - 600] = true;
                } else {
                    item2.showMdf = true;
                }    
            
                if (item.count == 1) {
                    inventory.remove(choice);
                } else {
                    --item.count;
                }
                break;
            }
        }
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
    for (int i = 0; i < 12 - item->weight / 3; i++) {                        // 12 is "strength"
        auto cell = pos + offset * (i + 1);

        if (::level[cell] == 2)
            break;

        if (unitMap[cell]) {
            unitMap[cell]->dealDamage(item->weight / 2);
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
