#include<units/hero.hpp>

#include<units/enemy.hpp>
#include<game.hpp>
#include<item_list_formatters.hpp>
#include<items/food.hpp>
#include<items/armor.hpp>
#include<items/weapon.hpp>
#include<items/ammo.hpp>
#include<items/potion.hpp>
#include<items/scroll.hpp>

#include<fmt/core.h>
#include<fmt/printf.h>

#include<effolkronium/random.hpp>

#include<numeric>

using namespace fmt::literals;
using fmt::format;
using Random = effolkronium::random_static;

int Hero::getLevelUpXP() const {
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
    g_game.addMessage(format("Now you are level {}.", level));
    maxBurden += maxBurden / 4;
    maxHealth += maxHealth / 4;
    health = maxHealth;
}

bool Hero::isInvisible() const {
    return turnsInvisible > 0;
}

void Hero::checkVisibleCells() {
    seenMap.forEach([this] (Coord2i pos, bool & see) {
        see = canSee(pos);
    });
}

int Hero::getInventoryItemsWeight() const {
    int totalWeight = 0;
    for (auto const & entry : inventory) {
        totalWeight += entry.second->getTotalWeight();
    }
    return totalWeight;
}

template<class ... FMTStrategies>
void printList(std::string_view title, std::vector<Item const *> const & items, FMTStrategies && ... strategies) {
    g_game.getRenderer()
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
            .put(title);

    int lineNo = 2;
    for (int i = 0; i < items.size(); i++) {
        g_game.getRenderer()
                .setCursorPosition(Coord2i{ LEVEL_COLS + 10, lineNo })
                .put(formatItem(i, *items[i], std::forward<FMTStrategies>(strategies)...));
        lineNo ++;
    }
}

std::pair<Hero::SelectStatus, char> Hero::selectOneFromInventory(std::string_view title, std::function<bool(Item const &)> filter) const {
    std::vector<Item const *> items;
    for (auto const & entry : inventory)
        if (filter(*entry.second))
            items.push_back(entry.second);

    if (items.empty())
        return { NothingToSelect, 0 };

    std::sort(items.begin(), items.end(), [] (Item const * a, Item const * b) {
        return a->inventorySymbol < b->inventorySymbol;
    });

    printList(title, items,
            formatters::LetterNumberingByInventoryID{},
            formatters::DontMark{},
            formatters::WithEquippedStatus{ weapon, armor });

    while (true) {
        char choice = g_game.getReader().readChar();
        if (choice == '\033')
            return { Cancelled, 0 };
        for (Item const * item : items)
            if (item->inventorySymbol == choice)
                return { Success, choice };
    }
}

std::pair<Hero::SelectStatus, std::vector<char>> Hero::selectMultipleFromInventory(
        std::string_view title,
        std::function<bool(Item const &)> filter) const {
    std::vector<Item const *> items;
    for (auto const & entry : inventory)
        if (filter(*entry.second))
            items.push_back(entry.second);

    if (items.empty())
        return { NothingToSelect, {} };

    std::sort(items.begin(), items.end(), [] (Item const * a, Item const * b) {
        return a->inventorySymbol < b->inventorySymbol;
    });

    std::vector<bool> selected(items.size());

    while (true) {
        printList(title, items,
                formatters::LetterNumberingByInventoryID{},
                formatters::MarkSelected{selected},
                formatters::WithEquippedStatus{weapon, armor});

        char choice = g_game.getReader().readChar();
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

std::pair<Hero::SelectStatus, int> Hero::selectOneFromList(std::string_view title, std::vector<Item const *> const & items) const {
    if (items.empty())
        return { NothingToSelect, 0 };

    printList(title, items,
            formatters::LetterNumberingByIndex{},
            formatters::DontMark{},
            formatters::WithoutEquippedStatus{});

    while (true) {
        char choice = g_game.getReader().readChar();
        if (choice == '\033')
            return { Cancelled, 0 };
        if (not std::isalpha(choice))
            continue;
        int index = std::islower(choice) ? choice - 'a' : choice - 'A' + 26;
        if (index < items.size())
            return { Success, index };
    }
}

std::pair<Hero::SelectStatus, std::vector<int>> Hero::selectMultipleFromList(std::string_view title, std::vector<Item const *> const & items) const {
    if (items.empty())
        return { NothingToSelect, {} };

    std::vector<bool> selected(items.size());

    while (true) {
        printList(title, items,
                  formatters::LetterNumberingByIndex{},
                  formatters::MarkSelected{selected},
                  formatters::WithoutEquippedStatus{});

        char choice = g_game.getReader().readChar();
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
    for (auto const & entry : inventory)
        if (entry.second->id == "map")
            return true;
    return false;
}

void Hero::pickUp() {
    auto & itemsMap = g_game.getItemsMap();
    if (itemsMap[pos].empty()) {
        g_game.addMessage("There is nothing here to pick up.");
        g_game.skipUpdate();
        return;
    }

    std::vector<ItemPile::iterator> iters;
    if (itemsMap[pos].size() == 1) {
        iters.push_back(itemsMap[pos].begin());
    } else {
        std::vector<Item const *> list;
        for (auto const & item : itemsMap[pos])
            list.push_back(item.get());

        auto [status, indices] = selectMultipleFromList("What do you want to pick up? ", list);
        if (status == Cancelled or indices.empty()) {
            g_game.skipUpdate();
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
    std::string message;
    for (auto it : iters) {
        auto & itemToPick = *it;
        std::string pickUpString;

        inventory.add(std::move(itemToPick)).doIf<AddStatus::New>(
                [this, it, &pickUpString](AddStatus::New added) {
            g_game.getItemsMap()[pos].erase(it);
            auto & item = inventory[added.at];
            if (item.isStackable and item.count > 1)
                pickUpString = format("{}x {} ({})", item.count, item.getName(), added.at);
            else
                pickUpString = format("{} ({})", item.getName(), added.at);
        }).doIf<AddStatus::Stacked>([this, it, &pickUpString](AddStatus::Stacked stacked) {
            g_game.getItemsMap()[pos].erase(it);
            auto & item = inventory[stacked.at];

            std::string pickedCount;
            if (stacked.pickedCount > 1)
                pickedCount = fmt::format("{}x ", stacked.pickedCount);

            pickUpString = fmt::format("{}{} ({}), now you have {}",
                                 pickedCount, item.getName(), stacked.at, item.count);
        }).doIf<AddStatus::AddError>([&itemToPick, &fullInventory, &message](auto & err) {
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
    message += ".";
    g_game.addMessage(message);

    if (getInventoryItemsWeight() > maxBurden and !isBurdened) {
        g_game.addMessage("You're burdened.");
        isBurdened = true;
    }
}

void Hero::clearRightPane() const {
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            g_game.getRenderer()
                .setCursorPosition(Coord2i{ LEVEL_COLS + j + 10, i })
                .put(' ');
        }
    }
}

void Hero::eat() {
    auto [status, choice] = selectOneFromInventory("What do you want to eat?", [] (Item const & item) {
        return item.getType() == Item::Type::Food;
    });
    switch (status) {
        case NothingToSelect:
            g_game.addMessage("You don't have anything to eat.");
            g_game.skipUpdate();
            return;
        case Cancelled:
            g_game.skipUpdate();
            return;
        default:break;
    }

    auto & item = inventory[choice];
    float rottenProbability = 1.f / luck;
    if (Random::get<bool>(rottenProbability)) {
        hunger += dynamic_cast<Food &>(item).nutritionalValue / 3;
        health --;
        g_game.addMessage("Fuck! This food was rotten!");
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
                g_game.skipUpdate();
            else
                takeArmorOff();
            break;
        case CONTROL_UNEQUIP:
            if (weapon == nullptr)
                g_game.skipUpdate();
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
            char hv = g_game.getReader().readChar();

            if (hv == 'h') {
                if (g_game.getReader().readChar() == 'e') {
                    if (g_game.getReader().readChar() == 'a') {
                        if (g_game.getReader().readChar() == 'l') {
                            hunger = 3000;
                            health = maxHealth * 100;
                        }
                    }
                }
            }

            if (hv == 'w') {
                if (g_game.getReader().readChar() == 'a') {
                    if (g_game.getReader().readChar() == 'l') {
                        if (g_game.getReader().readChar() == 'l') {
                            if (g_game.getReader().readChar() == 's') {
                                canMoveThroughWalls = true;
                            }
                        }
                    }
                }
            } else if (hv == 'd') {
                if (g_game.getReader().readChar() == 's') {
                    if (g_game.getReader().readChar() == 'c') {
                        canMoveThroughWalls = false;
                    }
                } else {
                    //g_game.getItemsMap().at(1, 1).push_back(g_game.getFoodTypes()[0]->clone());
                }
            } else if (hv == 'k') {
                if (g_game.getReader().readChar() == 'i') {
                    if (g_game.getReader().readChar() == 'l') {
                        if (g_game.getReader().readChar() == 'l') {
                            health -= (health * 2) / 3;
                            g_game.addMessage("Ouch!");
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
        g_game.addMessage("You have no ranged weapon in hands.");
        g_game.skipUpdate();
        return;
    }

    clearRightPane();
    g_game.getRenderer()
        .setCursorPosition(Coord2i{ LEVEL_COLS + 10 })
        .put("Now you can load your weapon");

    while (true) {
        clearRightPane();
        g_game.getRenderer()
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 1 })
            .put('[');

        for (int i = 0; i < weapon->cartridge.getCapacity(); i++) {
            TextStyle style{ TerminalColor{} };
            char symbol = 'i';
            if (weapon->cartridge[i]) {
                std::string_view ammoID = weapon->cartridge[i]->id;
                if (ammoID == "steel_bullets") {
                    style = TextStyle{TextStyle::Bold, Color::Black};
                } else if (ammoID == "shotgun_bullets") {
                    style = TextStyle{TextStyle::Bold, Color::Red};
                } else {
                    symbol = '?';
                }
            } else {
                symbol = '_';
            }
            g_game.getRenderer().put(symbol, style);
        }
        g_game.getRenderer().put(']');

        int lineY = 2;
        for (auto entry : inventory) {
            if (entry.second->getType() != Item::Type::Ammo)
                continue;

            std::string line = format("{} - {} (x{})",
                entry.first,
                entry.second->getName(),
                entry.second->count);

            g_game.getRenderer()
                .setCursorPosition(Coord2i{ LEVEL_COLS + 10, lineY })
                .put(line);

            ++lineY;
        }

        ++lineY;
        g_game.getRenderer()
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10, lineY })
            .put("Press '-' to unload one.");

        char chToLoad;
        while (true) {
            chToLoad = g_game.getReader().readChar();
            if (chToLoad == '\033')
                return;
            if (chToLoad == '-' or inventory.hasID(chToLoad))
                break;
        }

        if (chToLoad == '-') {
            auto bullet = weapon->cartridge.unloadOne();
            if (bullet) {
                inventory.add(std::move(bullet)).doIf<AddStatus::AddError>([&] (auto & err) {
                    g_game.drop(std::move(bullet), pos);
                });
            }
        } else {
            auto & item = inventory[chToLoad];
            if (item.getType() == Item::Type::Ammo) {
                if (weapon->cartridge.isFull()) {
                    g_game.addMessage("Weapon is loaded");
                    return;
                }

                Ptr<Ammo> bullet;
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
    g_game.skipUpdate();
    if (inventory.isEmpty()) {
        g_game.addMessage("Your inventory is empty");
        return;
    }
    std::vector<Item const *> list;
    for (auto const & entry : inventory)
        list.push_back(entry.second);

    std::sort(list.begin(), list.end(), [] (Item const * a, Item const * b) {
        return a->inventorySymbol < b->inventorySymbol;
    });

    printList("Here is your inventory.", list,
            formatters::LetterNumberingByInventoryID{},
            formatters::DontMark{},
            formatters::WithEquippedStatus{ weapon, armor });
    g_game.getReader().readChar();
}

void Hero::wearArmor() {
    auto [status, choice] = selectOneFromInventory("What do you want to wear?");
    switch (status) {
        case NothingToSelect:
            g_game.addMessage("You don't have anything to wear.");
            g_game.skipUpdate();
            return;
        case Cancelled:
            g_game.skipUpdate();
            return;
        default:break;
    }
    auto & item = inventory[choice];
    g_game.addMessage(format("Now you wearing {}.", item.getName()));
    armor = dynamic_cast<Armor *>(&item);
}

void Hero::dropItems() {
    auto [status, choice] = selectOneFromInventory("What do you want to drop?");
    switch (status) {
        case NothingToSelect:
            g_game.addMessage("You don't have anything to drop.");
            g_game.skipUpdate();
            return;
        case Cancelled:
            g_game.skipUpdate();
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
        g_game.getRenderer()
            .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
            .put(format("How much items do you want to drop? [1-{}]", maxCount))
            .display();

        dropCount = clamp(1, g_game.getReader().readChar() - '0', item.count);
    }
    g_game.drop(item.splitStack(dropCount), pos);
    if (item.count == 0)
        inventory.remove(choice);

    if (getInventoryItemsWeight() <= maxBurden and isBurdened) {
        g_game.addMessage("You are burdened no more.");
        isBurdened = false;
    }
}

void Hero::wieldWeapon() {
    auto [status, itemID] = selectOneFromInventory("What do you want to wield?", [] (Item const & item) {
        return item.getType() == Item::Type::Weapon;
    });
    switch (status) {
        case NothingToSelect:
            g_game.addMessage("You don't have anything to wield.");
            g_game.skipUpdate();
            return;
        case Success: {
            auto & item = inventory[itemID];
            g_game.addMessage(format("You wield {}.", item.getName()));
            weapon = dynamic_cast<Weapon *>(&item);
            break;
        }
        case Cancelled:
            g_game.skipUpdate();
            break;
        default:break;
    }
}

void Hero::throwItem() {
    auto [status, itemID] = selectOneFromInventory("What do you want to throw?");
    switch (status) {
        case NothingToSelect:
            g_game.addMessage("You don't have anything to throw.");
            g_game.skipUpdate();
            return;
        case Cancelled:
            g_game.skipUpdate();
            return;
        default:
            break;
    }
    auto & item = inventory[itemID];
    int throwCount = 1;
    if (item.count > 1) {
        clearRightPane();
        int maxCount = std::min(item.count, 9);
        g_game.getRenderer()
                .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
                .put(format("How many items do you want to throw? [1-{}]", maxCount));

        while (true) {
            char countChoice = g_game.getReader().readChar();
            if (countChoice == '\033')
                return;

            throwCount = countChoice - '0';
            if (throwCount > 0 and throwCount <= maxCount)
                break;
        }
    }

    clearRightPane();
    g_game.getRenderer()
        .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
        .put("In what direction?");

    Direction throwDir;
    while (true) {
        char dirChoice = g_game.getReader().readChar();
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
    auto [status, itemID] = selectOneFromInventory("What do you want to drink?", [] (Item const & item) {
        return item.getType() == Item::Type::Potion;
    });
    switch (status) {
        case NothingToSelect:
            g_game.addMessage("You don't have anything to drink.");
            g_game.skipUpdate();
            return;
        case Cancelled:
            g_game.skipUpdate();
            return;
        default:break;
    }

    auto & item = inventory[itemID];
    auto & potion = dynamic_cast<Potion &>(item);
    switch (potion.effect) {
        case Potion::Heal:
            heal(3);
            g_game.addMessage("Now you feeling better.");
            break;
        case Potion::Invisibility:
            turnsInvisible = 150;
            g_game.addMessage("Am I invisible? Oh, lol!");
            break;
        case Potion::Teleport:
            while (true) {
                Coord2i pos = { Random::get(0, LEVEL_COLS - 1), Random::get(0, LEVEL_ROWS - 1) };
                if (g_game.level()[pos] != 2 and not g_game.getUnitsMap()[pos]) {
                    setTo(pos);
                    break;
                }
            }
            g_game.addMessage("Teleportation is so straaange thing!");
            break;
        case Potion::None:
            g_game.addMessage("Well.. You didn't die. Nice.");
            break;
        case Potion::Blindness:
            vision = 1;
            turnsBlind = 50;
            g_game.addMessage("My eyes!!");
            break;
        default:
            throw std::logic_error("Unknown potion id");
    }
    g_game.markPotionAsKnown(potion.id);

    if (item.count == 1) {
        inventory.remove(itemID);
    } else {
        --item.count;
    }
}

void Hero::readScroll() {
    auto [status, itemID] = selectOneFromInventory("What do you want to read?", [] (Item const & item) {
        return item.getType() == Item::Type::Scroll;
    });
    switch (status) {
        case NothingToSelect:
            g_game.addMessage("You don't have anything to read.");
            g_game.skipUpdate();
            return;
        case Cancelled:
            g_game.skipUpdate();
            return;
        default:
            break;
    }

    auto & item = inventory[itemID];
    switch (dynamic_cast<Scroll &>(item).effect) {
        case Scroll::Map:
            g_game.addMessage("You wrote this map. Why you read it, I don't know.");
            break;
        case Scroll::Identify: {
            auto [status, chToApply] = selectOneFromInventory("What do you want to identify?", [] (Item const & item) {
                if (item.getType() == Item::Type::Potion) {
                    if (not g_game.isPotionKnown(item.id))
                        return true;
                } else if (not item.showMdf){
                    return true;
                }
                return false;
            });
            switch (status) {
                case NothingToSelect:
                    g_game.addMessage("You have nothing to identify.");
                    g_game.skipUpdate();
                    return;
                case Cancelled:
                    g_game.skipUpdate();
                    return;
                default:
                    break;
            }

            auto & item2 = inventory[chToApply];
            if (item2.getType() == Item::Type::Potion) {
                g_game.markPotionAsKnown(item2.id);
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
    auto & unitsMap = g_game.getUnitsMap();
    auto & enemy = dynamic_cast<Enemy &>(*unitsMap[cell]);
    if (weapon) {
        enemy.dealDamage(weapon->damage);
    }
    if (enemy.health <= 0) {
        enemy.dropInventory();
        xp += enemy.xpCost;
        unitsMap[cell].reset();
    }
}

void Hero::throwAnimated(Ptr<Item> item, Direction direction) {
    int throwDist = 0;
    auto offset = toVec2i(direction);
    char sym = toChar(direction);
    for (int i = 0; i < 12 - item->getTotalWeight() / 3; i++) {                        // 12 is "strength"
        auto cell = pos + offset * (i + 1);

        if (g_game.level()[cell] == 2)
            break;

        auto & unitsMap = g_game.getUnitsMap();
        if (unitsMap[cell]) {
            unitsMap[cell]->dealDamage(item->getTotalWeight() / 2);
            if (unitsMap[cell]->health <= 0) {
                auto & enemy = dynamic_cast<Enemy &>(*unitsMap[cell]);
                enemy.dropInventory();
                xp += enemy.xpCost;
                unitsMap[cell].reset();
            }
            break;
        }
        g_game.getRenderer()
            .setCursorPosition(cell)
            .put(sym)
            .display();
        throwDist++;
        sleep(DELAY);
    }
    g_game.drop(std::move(item), pos + offset * throwDist);
}

void Hero::shoot() {
    if (weapon == nullptr or not weapon->isRanged) {
        g_game.addMessage("You have no ranged weapon in hands.");
        return;
    }
    if (weapon->cartridge.isEmpty()) {
        g_game.addMessage("You have no bullets.");
        g_game.skipUpdate();
        return;
    }
    g_game.getRenderer()
        .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
        .put("In what direction? ");

    char choice = g_game.getReader().readChar();
    auto optdir = getDirectionByControl(choice);
    if (not optdir) {
        g_game.skipUpdate();
        return;
    }
    auto direction = *optdir;
    auto offset = toVec2i(direction);
    char sym = toChar(direction);
    int bulletPower = weapon->cartridge.next().damage + weapon->damageBonus;

    for (int i = 1; i < weapon->range + weapon->cartridge.next().range; i++) {
        auto cell = pos + offset * i;

        if (g_game.level()[cell] == 2)
            break;

        auto & unitsMap = g_game.getUnitsMap();
        if (unitsMap[cell]) {
            unitsMap[cell]->dealDamage(bulletPower - i / 3);
            if (unitsMap[cell]->health <= 0) {
                auto & enemy = dynamic_cast<Enemy &>(*unitsMap[cell]);
                enemy.dropInventory();
                xp += enemy.xpCost;
                unitsMap[cell].reset();
            }
        }
        g_game.getRenderer()
            .setCursorPosition(cell)
            .put(sym)
            .display();
        sleep(DELAY / 3);
    }
    weapon->cartridge.unloadOne();
}

void Hero::moveTo(Coord2i cell) {
    auto const & level = g_game.level();
    if (not level.isIndex(cell))
        return;
    if (level[cell] != 2 or canMoveThroughWalls) {
        auto const & unitsMap = g_game.getUnitsMap();
        if (unitsMap[cell] and unitsMap[cell]->getType() == Unit::Type::Enemy) {
            attackEnemy(cell);
        } else if (not unitsMap[cell]) {
            setTo(cell);
        }
    } else if (level[cell] == 2) {
        if (weapon != nullptr and weapon->canDig) {
            g_game.getRenderer()
                .setCursorPosition(Coord2i{ LEVEL_COLS + 10, 0 })
                .put("Do you want to dig this wall? [yn]");

            char inpChar = g_game.getReader().readChar();
            if (inpChar == 'y' or inpChar == 'Y') {
                g_game.level()[cell] = 1;
                float breakProbability = (Hero::MAX_LUCK - luck) / 100.f;
                if (Random::get<bool>(breakProbability)) {
                    g_game.addMessage(format("You've broken your {}.", weapon->getName()));
                    char weaponID = weapon->inventorySymbol;
                    unequipWeapon();
                    inventory.remove(weaponID);
                }
                return;
            }
        }
        g_game.addMessage("The wall is the way.");
        g_game.skipUpdate();
    }
}

