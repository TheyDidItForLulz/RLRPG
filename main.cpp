//////////////////////////////////////////////////// Symbols ///////////////////////////////////////////////////////////////
/*
                                                '_'(Floor) == 1
                                                '#'(Wall) == 2
                                                '^'(Pile) == 3
                                                '%'(Egg) == 100
                                                '%'(Apple) == 101
                                                '@'(Hero) == 200
                                                '@'(Barbarian) == 201
                                                '@'(Zombie) == 202
                                                '@'(Guardian) == 203
                                                '&'(Leather armor) == 301
                                                '&'(Chain armor) == 300
                                                '/'(Copper shortsword) == 400
                                                '/'(Bronze spear) == 401
                                                '/'(Musket) == 402
                                                '/'(Stick) == 403
                                                '/'(Shotgun) == 404
                                                '/'(Pistol) == 405
                                                ','(Steel bullets) == 450
                                                ','(Shotgun shells) == 451
                                                '~'(Map) == 500
                                                '~'(Identify scroll) == 501
                                                '!'(Blue potion) == 600
                                                '!'(Green potion) == 601
                                                '!'(Dark potion) == 602
                                                '!'(Magenta potion) == 603
                                                '!'(Yellow potion) == 604
                                                '\'(Pickaxe) == 700
*/
////////////////////////////////////////////////// Modificators ///////////////////////////////////////////////////////////
/*
                                                1 - Nothing
                                                2 - Thorns (chance to turn damage back)
*/
/////////////////////////////////////////////////// Attributes ////////////////////////////////////////////////////////////
/*
                                                100 - Nothing
                                                101..199 - Food attributes
                                                200..299 - Armor attributes
                                                300..399 - Weapon attributes
                                                201 - Worn
                                                301 - Wielded
*/
//////////////////////////////////////////////////// Effects //////////////////////////////////////////////////////////////
/*
                                                1 - Map recording in Hard-mode                
                                                2 - Identify
*/
///////////////////////////////////////////////// Potion Effects //////////////////////////////////////////////////////////
/*
                                                1 - Healing 3 hp
                                                2 - Invisibility
                                                3 - Random location teleport
                                                4 - Nothing
                                                5 - Blindness
*/
/////////////////////////////////////////////// Tool possibilities ////////////////////////////////////////////////////////
/*
                                                1 - Digging through walls
*/
//////////////////////////////////////////// Types of weapon construction /////////////////////////////////////////////////
/*
                                        1 - One direction(Musket), scheme:

                                                @--->
                                        
                                        2 - Triple direction(Shotgun), scheme:
                                
                                              /
                                             /
                                            @--->
                                             \
                                              \

                                          !COMMENT! This isn't implemented    (lol) хуй
*/
////////////////////////////////////////////////// Tree of skills /////////////////////////////////////////////////////////
/*
                                                 Detect, is food rotten
                                                   /                \
                                        Shoot through             Chance to confuse monster
                                              |                               |
                                        Combine things             Items identify by hands(?)
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//!COMMENT! // Also it isn't needed to show to the player his satiation. And luck too. And other stuff.

#include<iostream>
#include<fstream>
#include<cstdlib>
#include<ctime>
#include<vector>
#include<queue>
#include<assert.h>
#include<termlib/termlib.hpp>

#include<fmt/core.h>
#include<fmt/printf.h>

using namespace fmt::literals;

#include"include/colors.hpp"
#include"include/controls.hpp"
#include"include/level.hpp"
#include"include/item.hpp"
#include"include/gen_map.hpp"
#include"include/unit.hpp"
#include"include/utils.hpp"
#include"include/log.hpp"

// !COMMENT! random potion effect, random events, modificators

#define EMPTY_SLOT 52
// !COMMENT! // Level-up and items stacking
// !COMMENT! // Enemies must move at first turn
int g_mode = 1;    
bool g_exit = false;
bool g_stop = false;
bool g_generateMap = true;
                                                            
int map[FIELD_ROWS][FIELD_COLS];                                            
bool seenUpdated[FIELD_ROWS][FIELD_COLS];                                        // <- visible array
int g_turns = 0; /*-1*/

TerminalRenderer termRend;
TerminalReader termRead;
                                                            
void initField() {                                                            
    for (int i = 0; i < FIELD_ROWS; ++i) {                                                        
        for (int j = 0; j < FIELD_COLS; ++j) {                                                    
            map[i][j] = 1;                                        
        }
    }
}

void initialize() {
    std::srand(std::time(0));
    initField();
    initLog();
}

string message = "";
string bar = "";
string weaponBar = "";

std::optional<int> findItemAtCell(int h, int l, int sym) {
    for (int i = 0; i < FIELD_DEPTH; i++) {
        if (itemsMap[h][l][i].type != ItemEmpty && itemsMap[h][l][i].getItem().symbol == sym)
            return i;
    }
    return {};
}

Hero hero;

PossibleUnit unitMap[FIELD_ROWS][FIELD_COLS];

Enemy differentEnemies[TYPES_OF_ENEMIES];

void Hero::showInventory(char inp) {    
    PossibleItem list[MAX_USABLE_INV_SIZE];
    int len = 0;
    switch (inp) {    
        case CONTROL_SHOWINVENTORY: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
                if (inventory[i].type != ItemEmpty) {
                    list[len] = inventory[i];
                    len++;
                }
            }
            
            printList(list, len, "Here is your inventory.", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            len = 0;
            break;
        }
        case CONTROL_EAT: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
                if (inventory[i].type == ItemFood) {
                    list[len] = inventory[i];
                    len++;
                }
            }
            printList(list, len, "What do you want to eat?", 1);
            len = 0;
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            if (inventory[intch].type == ItemFood) {
                int prob = rand() % hero.luck;
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
            for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
                if (inventory[i].type == ItemArmor) {
                    list[len] = inventory[i];
                    len++;
                }
            }
            printList(list, len, "What do you want to wear?", 1);
            len = 0;
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
            for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
                if (inventory[i].type != ItemEmpty) {
                    list[len] = inventory[i];
                    len++;
                }
            }

            printList(list, len, "What do you want to drop?", 1);
            len = 0;
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            int num = findEmptyItemUnderThisCell(posH, posL);
            if (num == 101010) {
                message += "There is too much items";
                return;
            }
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

                auto optdepth = findItemAtCell(posH, posL, inventory[intch].getItem().symbol);
                if (optdepth) {        
                    itemsMap[posH][posL][*optdepth].getItem().count += dropCount;
                } else {            
                    itemsMap[posH][posL][num] = inventory[intch];
                    itemsMap[posH][posL][num].getItem().count = dropCount;
                }
                inventory[intch].getItem().count -= dropCount;
                if (inventory[intch].getItem().count == 0) {
                    inventory[intch].type = ItemEmpty;
                }
            } else if (inventory[intch].getItem().isStackable && inventory[intch].getItem().count == 1) {
                auto optdepth = findItemAtCell(posH, posL, inventory[intch].getItem().symbol);
                if (optdepth) {        
                    itemsMap[posH][posL][*optdepth].getItem().count++;
                    inventory[intch].getItem().count--;
                } else {            
                    itemsMap[posH][posL][num] = inventory[intch];
                }
                inventory[intch].type = ItemEmpty;
            } else {
                itemsMap[posH][posL][num] = inventory[intch];
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
            heroArmor = &inventory[EMPTY_SLOT];
            break;
        
        }
        case CONTROL_WIELD: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
                if (inventory[i].type == ItemWeapon || inventory[i].type == ItemTools) {
                    list[len] = inventory[i];
                    len++;
                }
            }

            printList(list, len, "What do you want to wield?", 1);
            len = 0;
            
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
            heroWeapon = &inventory[EMPTY_SLOT];
            break;
        }
        case CONTROL_THROW: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
                if (inventory[i].type != ItemEmpty) {
                    list[len] = inventory[i];
                    len++;
                }
            }

            printList(list, len, "What do you want to throw?", 1);
            len = 0;

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
            for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
                if (inventory[i].type == ItemPotion) {
                    list[len] = inventory[i];
                    len++;
                }
            }

            printList(list, len, "What do you want to drink?", 1);
            len = 0;

            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';

            if (inventory[intch].type == ItemPotion) {
                switch (inventory[intch].item.invPotion.effect) {
                    case 1: {
                        health = std::min(health + 3, DEFAULT_HERO_HEALTH);
                        message += "Now you feeling better. ";
                        break;
                    }
                    case 2: {
                        hero.turnsInvisible = 150;
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
                        hero.turnsBlind = 50;
                        message += "My eyes!! ";
                        break;
                    }
                }
                discoveredPotions[inventory[intch].item.invPotion.symbol - 600] = true;
                if (inventory[intch].getItem().count == 1) {
                    inventory[intch].type = ItemEmpty;
                } else {
                    --inventory[intch].getItem().count;
                }
            }
            break;
        }
        case CONTROL_READ: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
                if (inventory[i].type == ItemScroll) {
                    list[len] = inventory[i];
                    len++;
                }
            }

            printList(list, len, "What do you want to read?", 1);
            len = 0;

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
                                discoveredPotions[inventory[intin].getItem().symbol - 600] = true;
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
                
                string loadString = "";
                
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
                        if (!found) {
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
                        if (!found) {
                            bool can_stack = false;
                            for (int j = 0; j < FIELD_DEPTH; j++) {
                                if (itemsMap[posH][posL][j].type == 
                                        ItemAmmo && 
                                        itemsMap[posH][posL][j].getItem().symbol == 
                                        heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].symbol) {
                                    itemsMap[posH][posL][j].item.invAmmo.count++;
                                    heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
                                    heroWeapon->item.invWeapon.currentCS--;
                                    can_stack = true;
                                    found = true;
                                }
                            }
                            if (!can_stack) {
                                int empty = findEmptyItemUnderThisCell(posH, posL);
                                if (empty != 101010) {
                                    itemsMap[posH][posL][empty].item.invAmmo = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1];
                                    itemsMap[posH][posL][empty].type = ItemAmmo;
                                    heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
                                    heroWeapon->item.invWeapon.currentCS--;
                                    found = true;
                                }
                            }
                        }
                        if (!found) {
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

void dropInventory(PossibleUnit& unit) {
    int h = unit.getUnit().posH;
    int l = unit.getUnit().posL;
    for (int i = 0; i < UNITINVENTORY; i++) {                                        /* Here are some changes, that we need to test */
        if (unit.getUnit().unitInventory[i].type != ItemEmpty) {
            if (unit.getUnit().unitInventory[i].getItem().isStackable) {
                auto optdepth = findItemAtCell(unit.getUnit().posH, unit.getUnit().posL, unit.getUnit().unitInventory[i].getItem().symbol);
                if (optdepth) {
                    itemsMap[unit.getUnit().posH][unit.getUnit().posL][*optdepth].getItem().count += unit.getUnit().unitInventory[i].getItem().count;
                } else {
                    int empty = hero.findEmptyItemUnderThisCell(h, l);
                    if (empty != 101010) {
                        itemsMap[h][l][empty] = unit.getUnit().unitInventory[i];
                    }
                }
            } else {
                int empty = hero.findEmptyItemUnderThisCell(h, l);
                if (empty != 101010) {
                    itemsMap[h][l][empty] = unit.getUnit().unitInventory[i];
                }
            }
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
        dropInventory(unitMap[posH + a1][posL + a2]);
        unitMap[posH + a1][posL + a2].type = UnitEmpty;
        xp += unitMap[posH + a1][posL + a2].unit.uEnemy.xpIncreasing;
    }
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

void Hero::throwAnimated(PossibleItem& item, Direction direction) {
    int ThrowFIELD_COLS = 0;
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
                dropInventory(unitMap[row][col]);
                unitMap[row][col].type = UnitEmpty;
                xp += unitMap[row][col].unit.uEnemy.xpIncreasing;
            }
            break;
        }
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
        ThrowFIELD_COLS++;
        sleep(DELAY);
    }
    int empty = findEmptyItemUnderThisCell(posH + dy * ThrowFIELD_COLS, posL + dx * ThrowFIELD_COLS);
    if (empty == 101010) {
        int empty2 = findEmptyItemUnderThisCell(posH + dy * (ThrowFIELD_COLS - 1), posL + dx * (ThrowFIELD_COLS - 1));
        itemsMap[posH + dy * (ThrowFIELD_COLS - 1)][posL + dx * (ThrowFIELD_COLS - 1)][empty2] = item;
        item.type = ItemEmpty;
    } else {
        itemsMap[posH + dy * ThrowFIELD_COLS][posL + dx * ThrowFIELD_COLS][empty] = item;
        item.type = ItemEmpty;
    }
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
    int bulletPower = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].damage + hero.heroWeapon->item.invWeapon.damageBonus;

    for (int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++) {
        int row = posH + dy * i;
        int col = posL + dx * i; 

        if (map[row][col] == 2)
            break;

        if (unitMap[row][col].type != UnitEmpty) {
            unitMap[row][col].getUnit().health -= bulletPower - i / 3;
            if (unitMap[row][col].getUnit().health <= 0) {
                dropInventory(unitMap[row][col]);
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

void Enemy::shoot() {
    if (posH == hero.posH && posL < hero.posL)
        dir = Direction::Right;
    else if (posH == hero.posH && posL > hero.posL)
        dir = Direction::Left;
    else if (posL == hero.posL && posH > hero.posH)
        dir = Direction::Up;
    else if (posL == hero.posL && posH < hero.posH)
        dir = Direction::Down;
    else if (posL > hero.posL && posH > hero.posH)
        dir = Direction::UpLeft;
    else if (posL > hero.posL && posH < hero.posH)
        dir = Direction::DownLeft;
    else if (posL < hero.posL && posH < hero.posH)
        dir = Direction::DownRight;
    else if (posL < hero.posL && posH > hero.posH)
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
            hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
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

int bfs(int targetH, int targetL, int h, int l, int &posH, int &posL) {
    int depth = 2 + std::abs(targetH - h) + std::abs(targetL - l);                        // <- smth a little bit strange
    queue<int> x, y;
    x.push(l);
    y.push(h);
    int used[FIELD_ROWS][FIELD_COLS] = {};
    used[h][l] = true;
    while (!x.empty() && !y.empty()) {
        int v_x = x.front();
        int v_y = y.front();
        if (v_y == targetH && v_x == targetL)
            break;
        if (used[v_y][v_x] > depth) {
            return -1;
        }
        x.pop();
        y.pop();
    
        if (v_y < FIELD_ROWS - 1 && !used[v_y + 1][v_x] && (unitMap[v_y + 1][v_x].type == UnitEmpty 
            || unitMap[v_y + 1][v_x].type == UnitHero) && map[v_y + 1][v_x] != 2) {
            y.push(v_y + 1);
            x.push(v_x);
            used[v_y + 1][v_x] = 1 + used[v_y][v_x];
        }
        if (v_y > 0 && !used[v_y - 1][v_x] && (unitMap[v_y - 1][v_x].type == UnitEmpty 
            || unitMap[v_y - 1][v_x].type == UnitHero) && map[v_y - 1][v_x] != 2) {
            y.push(v_y - 1);
            x.push(v_x);    
            used[v_y - 1][v_x] = 1 + used[v_y][v_x];
        }
        if (v_x < FIELD_COLS - 1 && !used[v_y][v_x + 1] && (unitMap[v_y][v_x + 1].type == UnitEmpty 
            || unitMap[v_y][v_x + 1].type == UnitHero) && map[v_y][v_x + 1] != 2) {
            y.push(v_y);
            x.push(v_x + 1);
            used[v_y][v_x + 1] = 1 + used[v_y][v_x];
        }
        if (v_x > 0 && !used[v_y][v_x - 1] && (unitMap[v_y][v_x - 1].type == UnitEmpty 
            || unitMap[v_y][v_x - 1].type == UnitHero) && map[v_y][v_x - 1] != 2) {
            y.push(v_y);
            x.push(v_x - 1);
            used[v_y][v_x - 1] = 1 + used[v_y][v_x];    
        }
        if (g_mode == 2) {
            if (v_y < FIELD_ROWS - 1)
            {
                if (v_x > 0 && !used[v_y + 1][v_x - 1] && (unitMap[v_y + 1][v_x - 1].type == UnitEmpty 
                    || unitMap[v_y + 1][v_x - 1].type == UnitHero) && map[v_y + 1][v_x - 1] != 2) {
                    y.push(v_y + 1);
                    x.push(v_x - 1);
                    used[v_y + 1][v_x - 1] = 1 + used[v_y][v_x];
                }
                if (v_x < FIELD_COLS - 1 && !used[v_y + 1][v_x + 1] && (unitMap[v_y + 1][v_x + 1].type == UnitEmpty 
                    || unitMap[v_y + 1][v_x + 1].type == UnitHero) && map[v_y + 1][v_x + 1] != 2) { 
                    y.push(v_y + 1);
                    x.push(v_x + 1);
                    used[v_y + 1][v_x + 1] = 1 + used[v_y][v_x];
                }
            }
            if (v_y > 0) {
                if (v_x > 0 && !used[v_y - 1][v_x - 1] && (unitMap[v_y - 1][v_x - 1].type == UnitEmpty 
                    || unitMap[v_y - 1][v_x - 1].type == UnitHero) && map[v_y - 1][v_x - 1] != 2) {
                    y.push(v_y - 1);
                    x.push(v_x - 1);
                    used[v_y - 1][v_x - 1] = 1 + used[v_y][v_x];
                }
                if (v_x < FIELD_COLS - 1 && !used[v_y - 1][v_x + 1] && (unitMap[v_y - 1][v_x + 1].type == UnitEmpty 
                    || unitMap[v_y - 1][v_x + 1].type == UnitHero) && map[v_y - 1][v_x + 1] != 2) {
                    y.push(v_y - 1);
                    x.push(v_x + 1);
                    used[v_y - 1][v_x + 1] = 1 + used[v_y][v_x];
                }
            }
        }
    }

    if (!used[targetH][targetL]) {
        return -1;
    }
    int v_y = targetH, v_x = targetL;
    while (used[v_y][v_x] != 2) {
        if (g_mode == 2) {
            if (v_y && v_x && used[v_y - 1][v_x - 1] + 1 == used[v_y][v_x]) {
                --v_y;
                --v_x;
                continue;
            }
            if (v_y && v_x < FIELD_COLS - 1 && used[v_y - 1][v_x + 1] + 1 == used[v_y][v_x]) {
                --v_y;
                ++v_x;
                continue;
            }
            if (v_y < FIELD_ROWS - 1 && v_x && used[v_y + 1][v_x - 1] + 1 == used[v_y][v_x]) {
                ++v_y;
                --v_x;
                continue;
            }
            if (v_y < FIELD_ROWS - 1 && v_x < FIELD_COLS - 1 && used[v_y + 1][v_x + 1] + 1 == used[v_y][v_x]) {
                ++v_y;
                ++v_x;
                continue;
            }
        }
        if (v_y && used[v_y - 1][v_x] + 1 == used[v_y][v_x]) {
            --v_y;
            continue;
        }
        if (v_x && used[v_y][v_x - 1] + 1 == used[v_y][v_x]) {
            --v_x;
            continue;
        }
        if (v_y < FIELD_ROWS - 1 && used[v_y + 1][v_x] + 1 == used[v_y][v_x]) {
            ++v_y;
            continue;
        }
        if (v_x < FIELD_COLS - 1 && used[v_y][v_x + 1] + 1 == used[v_y][v_x]) {
            ++v_x;
            continue;
        }
    }

    posH = v_y;
    posL = v_x;
}

void updatePosition(PossibleUnit& unit) {
    bool HeroVisible = false;

    if (hero.turnsInvisible > 0) {
        HeroVisible = false;
    } else if ((sqr(unit.getUnit().posH - hero.posH) + sqr(unit.getUnit().posL - hero.posL) < sqr(unit.getUnit().vision)) 
        && unit.getUnit().canSeeCell(hero.posH, hero.posL)) {
        HeroVisible = true;
    }
    
    int pH = 1, pL = 1;

    if (HeroVisible) {
        if ((unit.getUnit().posH == hero.posH || 
                unit.getUnit().posL == hero.posL || 
                std::abs(hero.posH - unit.getUnit().posH) == std::abs(hero.posL - unit.getUnit().posL)) && 
                unit.getUnit().unitWeapon->item.invWeapon.Ranged == true &&
                unit.getUnit().unitWeapon->item.invWeapon.range + unit.getUnit().unitAmmo->item.invAmmo.range >= 
                std::abs(hero.posH - unit.getUnit().posH) + std::abs(hero.posL - unit.getUnit().posL)) {
            unit.unit.uEnemy.shoot();
        } else {
            unit.unit.uEnemy.targetH = hero.posH;
            unit.unit.uEnemy.targetL = hero.posL;

            if (bfs(hero.posH, hero.posL, unit.getUnit().posH, unit.getUnit().posL, pH, pL) == -1) {
                HeroVisible = false;
            } else {
                if (unitMap[pH][pL].type == UnitEnemy) {
                    return;
                } else if (unitMap[pH][pL].type == UnitHero) {
                    if (unit.getUnit().unitWeapon->type == ItemWeapon) {
                        if (hero.heroArmor->item.invArmor.mdf != 2) {
                            hero.health -= unit.getUnit().unitWeapon->item.invWeapon.damage * ((100 - hero.heroArmor->item.invArmor.defence) / 100.0);
                        } else {
                            unit.getUnit().health -= unit.getUnit().unitWeapon->item.invWeapon.damage;
                        }
                    } else if (unit.getUnit().unitWeapon->type == ItemTools) {
                        if (hero.heroArmor->item.invArmor.mdf != 2) {
                            hero.health -= unit.getUnit().unitWeapon->item.invTools.damage * ((100 - hero.heroArmor->item.invArmor.defence) / 100.0);
                        } else {
                            unit.getUnit().health -= unit.getUnit().unitWeapon->item.invTools.damage;
                        }
                    }
                    if (unit.getUnit().health <= 0) {
                        unit.type = UnitEmpty;
                    }
                } else {
                    unit.getUnit().posH = pH;
                    unit.getUnit().posL = pL;
                    unitMap[pH][pL] = unit;
                    unit.type = UnitEmpty;
                }
            }
        }
    }
    if (!HeroVisible) {
        bool needRandDir = 0;
        if (unit.unit.uEnemy.targetH != -1 && (unit.unit.uEnemy.targetH != unit.getUnit().posH || unit.unit.uEnemy.targetL != unit.getUnit().posL)) {

            if (bfs(unit.unit.uEnemy.targetH, unit.unit.uEnemy.targetL, unit.unit.uEnemy.posH, unit.unit.uEnemy.posL, pH, pL) == -1) {
                needRandDir = 1;
            } else {
                if (pH < FIELD_ROWS && pH > 0 && pL < FIELD_COLS && pL > 0) {
                    if (unitMap[pH][pL].type == UnitHero) {
                        if (unit.getUnit().unitWeapon->type == ItemWeapon) {
                            if (hero.heroArmor->item.invArmor.mdf != 2) {
                                hero.health -= unit.getUnit().unitWeapon->item.invWeapon.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
                            } else {
                                unit.getUnit().health -= unit.getUnit().unitWeapon->item.invWeapon.damage;
                            }
                        } else if (unit.getUnit().unitWeapon->type == ItemTools) {
                            if (hero.heroArmor->item.invArmor.mdf != 2) {
                                hero.health -= unit.getUnit().unitWeapon->item.invTools.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
                            } else {
                                unit.getUnit().health -= unit.getUnit().unitWeapon->item.invTools.damage;
                            }
                        }
                        if (unit.getUnit().health <= 0) {
                            unit.type = UnitEmpty;
                        }
                    } else {
                        unit.getUnit().posH = pH;
                        unit.getUnit().posL = pL;
                        unitMap[pH][pL] = unit;
                        unit.type = UnitEmpty;
                    }
                }
            }
        } else {
            needRandDir = 1;
        }
        if (needRandDir) {
            vector<int> visionArrayH;
            vector<int> visionArrayL;

            int psH = unit.getUnit().posH, psL = unit.getUnit().posL, vis = unit.getUnit().vision;

            for (int i = std::max(psH - vis, 0); i < std::min(FIELD_ROWS, psH + vis); i++) {
                for (int j = std::max(psL - vis, 0); j < std::min(psL + vis, FIELD_COLS); j++) {
                    if ((i != psH || j != psL)
                    && sqr(psH - i) + sqr(psL - j) < sqr(vis) && map[i][j] != 2 
                    && unitMap[i][j].type == UnitEmpty && unit.getUnit().canSeeCell(i, j)) {
                        visionArrayH.push_back(i);
                        visionArrayL.push_back(j);
                    }
                }    
            }
            while (true) {
                int r; 
                int rposH = visionArrayH[r = (rand() % visionArrayH.size())];
                int rposL = visionArrayL[r];
                
                unit.unit.uEnemy.targetH = rposH;
                unit.unit.uEnemy.targetL = rposL;

                if (bfs(unit.unit.uEnemy.targetH, unit.unit.uEnemy.targetL, unit.unit.uEnemy.posH, unit.unit.uEnemy.posL, pH, pL) == -1) {
                    continue;
                }
                if (pH < FIELD_ROWS && pH > 0 && pL < FIELD_COLS && pL > 0) {
                    if (unitMap[pH][pL].type == UnitHero) {
                        if (unit.getUnit().unitWeapon->type == ItemWeapon) {
                            if (hero.heroArmor->item.invArmor.mdf != 2) {
                                hero.health -= unit.getUnit().unitWeapon->item.invWeapon.damage * ((100 - hero.heroArmor->item.invArmor.defence) / 100.0);
                            } else {
                                unit.getUnit().health -= unit.getUnit().unitWeapon->item.invWeapon.damage;
                            }
                        } else if (unit.getUnit().unitWeapon->type == ItemTools) {
                            if (hero.heroArmor->item.invArmor.mdf != 2) {
                                hero.health -= unit.getUnit().unitWeapon->item.invTools.damage * ((100 - hero.heroArmor->item.invArmor.defence) / 100.0);
                            } else {
                                unit.getUnit().health -= unit.getUnit().unitWeapon->item.invTools.damage;
                            }
                        }
                        if (unit.getUnit().health <= 0) {
                            unit.type = UnitEmpty;
                        }
                    } else {
                        unit.getUnit().posH = pH;
                        unit.getUnit().posL = pL;
                        unitMap[pH][pL] = unit;
                        unit.type = UnitEmpty;
                    }
                }
                break;
                /* Here ^ is random moving */
            }
        }
    }
}

void updateAI() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (unitMap[i][j].type == UnitEnemy && unitMap[i][j].unit.uEnemy.movedOnTurn != g_turns) {
#                ifdef DEBUG
                message += "{{{}|{}|{}|{}}}"_format(i, j, unitMap[i][j].unit.uEnemy.symbol, unitMap[i][j].getUnit().health);
#                endif
                if (g_mode == 2 && g_turns % 200 == 0) {
                    unitMap[i][j].getUnit().health++;
                }
                unitMap[i][j].unit.uEnemy.movedOnTurn = g_turns;
                updatePosition(unitMap[i][j]);
            }
        }
    }
}

void setItems() {
    for (int i = 0; i < FOODCOUNT; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;

        if (map[h][l] == 1) {
            int p = rand() % TYPES_OF_FOOD;
            int d = rand() % FIELD_DEPTH;
            itemsMap[h][l][d] = differentFood[p];
            itemsMap[h][l][d].getItem().posH = h;
            itemsMap[h][l][d].getItem().posL = l;
        } else {
            i--;
        }
    }

    for (int i = 0; i < ARMORCOUNT; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;

        if (map[h][l] == 1) {
            int p = rand() % TYPES_OF_ARMOR;
            int d = rand() % FIELD_DEPTH;
            itemsMap[h][l][d] = differentArmor[p];
            itemsMap[h][l][d].getItem().posH = h;
            itemsMap[h][l][d].getItem().posL = l;
            if (rand() % 500 / hero.luck == 0) {
                itemsMap[h][l][d].item.invArmor.mdf = 2;
            }
        } else {
            i--;
        }
    }
    
    for (int i = 0; i < WEAPONCOUNT; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;
        
        if (map[h][l] == 1) {
            int p = rand() % TYPES_OF_WEAPONS;
            int d = rand() % FIELD_DEPTH;
            itemsMap[h][l][d] = differentWeapon[p];
            itemsMap[h][l][d].getItem().posH = h;
            itemsMap[h][l][d].getItem().posL = l;
        } else {
            i--;
        }
    }

    for (int i = 0; i < AMMOCOUNT; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;
        
        if (map[h][l] == 1) {
            int p = rand() % TYPES_OF_AMMO;
            int d = rand() % FIELD_DEPTH;
            itemsMap[h][l][d] = differentAmmo[p];
            itemsMap[h][l][d].getItem().posH = h;
            itemsMap[h][l][d].getItem().posL = l;
            itemsMap[h][l][d].item.invAmmo.count = (rand() % hero.luck) / 2 + 1;
        } else {
            i--;
        }
    }

    for (int i = 0; i < SCROLLCOUNT; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;

        if (map[h][l] == 1) {
            int p = rand() % TYPES_OF_SCROLLS;
            int d = rand() % FIELD_DEPTH;
            itemsMap[h][l][d] = differentScroll[p];
            itemsMap[h][l][d].getItem().posH = h;
            itemsMap[h][l][d].getItem().posL = l;
        }
    }
    for (int i = 0; i < POTIONCOUNT; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;

        if (map[h][l] == 1) {
            int p = rand() % TYPES_OF_POTIONS;
            int d = rand() % FIELD_DEPTH;
            itemsMap[h][l][d] = differentPotion[p];
            itemsMap[h][l][d].getItem().posH = h;
            itemsMap[h][l][d].getItem().posL = l;
        }
    }
    for (int i = 0; i < TOOLSCOUNT; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;

        if (map[h][l] == 1) {
            int p = rand() % TYPES_OF_TOOLS;
            int d = rand() % FIELD_DEPTH;
            itemsMap[h][l][d] = differentTools[p];
            itemsMap[h][l][d].getItem().posH = h;
            itemsMap[h][l][d].getItem().posL = l;
        }
    }
}

void spawnUnits() {
    for (int i = 0; i < 1; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;
        if (map[h][l] == 1 && unitMap[h][l].type == UnitEmpty) {
            unitMap[h][l] = hero;
            hero.posH = h;
            hero.posL = l;
            break;
        } else {
            i--;
        }
    }
    for (int i = 0; i < ENEMIESCOUNT; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;
        if (map[h][l] == 1 && unitMap[h][l].type == UnitEmpty) {
            int p = rand() % TYPES_OF_ENEMIES;
            unitMap[h][l] = differentEnemies[p];
            unitMap[h][l].getUnit().posH = h;
            unitMap[h][l].getUnit().posL = l;
        } else {
            i--;
        }
    }
}

#ifdef DEBUG

void draw() {
    termRend.setCursorPosition(Vec2i{});

    static int mapSaved[FIELD_ROWS][FIELD_COLS] = {};

    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            mapSaved[i][j] = map[i][j];
        }
    }
    
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            char symbol = ' ';
            TextStyle style{ TerminalColor{} };
            if (mapSaved[i][j] != 0) {
                bool near = abs(i - hero.posH) <= 1 && abs(j - hero.posL) <= 1;
                if (hero.findItemsCountUnderThisCell(i, j) == 0 && unitMap[i][j].type == UnitEmpty) {
                    switch (mapSaved[i][j]) {
                        case 1:
                            if (seenUpdated[i][j]) {
                                symbol = '.';
                            }
                            break;
                        case 2:
                            if (seenUpdated[i][j])
                                style += TextStyle::Bold;
                            symbol = '#';
                            break;
                    }
                } else if (hero.findItemsCountUnderThisCell(i, j) == 1 && unitMap[i][j].type == UnitEmpty) { /* HERE */
                    int MeetedItem = hero.findNotEmptyItemUnderThisCell(i, j);
                    switch (itemsMap[i][j][MeetedItem].getItem().symbol) {
                        case 100:
                            symbol = '%';
                            break;
                        case 101:
                            symbol = '%';
                            style += Color::Red;
                            break;
                        case 300:
                            symbol = '&';
                            style += Color::Black;
                            style += TextStyle::Bold;
                            break;
                        case 301:
                            symbol = '&';
                            style += Color::Yellow;
                            break;
                        case 400:
                            symbol = '/';
                            style += Color::Red;
                            style += TextStyle::Bold;
                            break;
                        case 401:
                            symbol = '/';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
                            break;
                        case 402:
                            symbol = '/';
                            style += TextStyle::Bold;
                            break;
                        case 403:
                            symbol = '/';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
                            break;
                        case 404:
                            symbol = '/';
                            style += Color::Black;
                            style += TextStyle::Bold;
                            break;
                        case 405:
                            symbol = '/';
                            break;
                        case 450:
                            symbol = ',';
                            style += Color::Black;
                            style += TextStyle::Bold;
                            break;
                        case 451:
                            symbol = ',';
                            style += Color::Red;
                            style += TextStyle::Bold;
                            break;
                        case 500:
                            symbol = '~';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
                            break;
                        case 501:
                            symbol = '~';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
                            break;
                        case 600:
                            symbol = '!';
                            style += Color::Blue;
                            style += TextStyle::Bold;
                            break;
                        case 601:
                            symbol = '!';
                            style += Color::Green;
                            break;
                        case 602:
                            symbol = '!';
                            style += Color::Black;
                            style += TextStyle::Bold;
                            break;
                        case 603:
                            symbol = '!';
                            style += Color::Magenta;
                            style += TextStyle::Bold;
                            break;
                        case 604:
                            symbol = '!';
                            style += Color::Yellow;
                            break;
                        case 700:
                            symbol = '\\';
                            style += Color::Yellow;
                            break;
                    }
                } else if (hero.findItemsCountUnderThisCell(i, j) > 1 && unitMap[i][j].type == UnitEmpty) { /* Here */
                    symbol = '^';
                    style = TextStyle{ TextStyle::Bold, TerminalColor{ Color::Black, Color::White } };
                }
                if (unitMap[i][j].type == UnitHero) { /* Here */
                    symbol = '@';
                    style += Color::Green;
                } else if (unitMap[i][j].type == UnitEnemy) { /* Here */
                    symbol = '@';
                    switch (unitMap[i][j].getUnit().symbol) { /* Here */
                        case 201:
                            style += Color::Yellow;
                            break;
                        case 202:
                            style = TextStyle{ TextStyle::Bold, Color::Green };
                            break;
                        case 203:
                            style = TextStyle{ TextStyle::Bold, Color::Black };
                            break;
                    }
                }
            } else {
                symbol = ' ';
            }
            termRend.put(symbol, style);
        }
        
        termRend.put('\n');
        
    }

}

#else

void draw(){
    
    //move(0, 0);
    termRend.setCursorPosition(Vec2i{});

    static int mapSaved[FIELD_ROWS][FIELD_COLS] = {};

    if (g_mode == 2 && !hero.isMapInInventory()) {
        for (int i = 0; i < FIELD_ROWS; i++) {
            for (int j = 0; j < FIELD_COLS; j++) {
                mapSaved[i][j] = 0;
            }
        }
    }

    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (seenUpdated[i][j]) {
                int itemsOnCell = hero.findItemsCountUnderThisCell(i, j);
                if (itemsOnCell == 0) {
                    mapSaved[i][j] = map[i][j];
                } else if (itemsOnCell == 1) {
                    mapSaved[i][j] = itemsMap[i][j][hero.findNotEmptyItemUnderThisCell(i, j)].getItem().symbol;
                } else {
                    mapSaved[i][j] = 100500; // Magic constant that means 'pile'
                }
            }
        }
    }
    
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            char symbol = ' ';
            TextStyle style{ TerminalColor{} };
            if (mapSaved[i][j] != 0) {
                bool near = abs(i - hero.posH) <= 1 && abs(j - hero.posL) <= 1;
                if (seenUpdated[i][j]) {
                    if (unitMap[i][j].type == UnitEmpty) {
                        switch (mapSaved[i][j]) {
                            case 1:
                                symbol = '.';
                                break;
                            case 2:
                                style += TextStyle::Bold;
                                symbol = '#';
                                break;
                            case 100:
                                symbol = '%';
                                break;
                            case 101:
                                symbol = '%';
                                style += Color::Red;
                                break;
                            case 300:
                                symbol = '&';
                                style += Color::Black;
                                style += TextStyle::Bold;
                                break;
                            case 301:
                                symbol = '&';
                                style += Color::Yellow;
                                break;
                            case 400:
                                symbol = '/';
                                style += Color::Red;
                                style += TextStyle::Bold;
                                break;
                            case 401:
                                symbol = '/';
                                style += Color::Yellow;
                                style += TextStyle::Bold;
                                break;
                            case 402:
                                symbol = '/';
                                style += TextStyle::Bold;
                                break;
                            case 403:
                                symbol = '/';
                                style += Color::Yellow;
                                style += TextStyle::Bold;
                                break;
                            case 404:
                                symbol = '/';
                                style += Color::Black;
                                style += TextStyle::Bold;
                                break;
                            case 405:
                                symbol = '/';
                                break;
                            case 450:
                                symbol = ',';
                                style += Color::Black;
                                style += TextStyle::Bold;
                                break;
                            case 451:
                                symbol = ',';
                                style += Color::Red;
                                style += TextStyle::Bold;
                                break;
                            case 500:
                                symbol = '~';
                                style += Color::Yellow;
                                style += TextStyle::Bold;
                                break;
                            case 501:
                                symbol = '~';
                                style += Color::Yellow;
                                style += TextStyle::Bold;
                                break;
                            case 600:
                                symbol = '!';
                                style += Color::Blue;
                                style += TextStyle::Bold;
                                break;
                            case 601:
                                symbol = '!';
                                style += Color::Green;
                                break;
                            case 602:
                                symbol = '!';
                                style += Color::Black;
                                style += TextStyle::Bold;
                                break;
                            case 603:
                                symbol = '!';
                                style += Color::Magenta;
                                style += TextStyle::Bold;
                                break;
                            case 604:
                                symbol = '!';
                                style += Color::Yellow;
                                break;
                            case 700:
                                symbol = '\\';
                                style += Color::Yellow;
                                break;
                        }
                    } else {
                        if (unitMap[i][j].type == UnitHero) {
                            symbol = '@';
                            style += Color::Green;
                        } else if (unitMap[i][j].type == UnitEnemy && seenUpdated[i][j]) {
                            symbol = '@';
                            switch (unitMap[i][j].getUnit().symbol) {
                                case 201:
                                    style += Color::Yellow;
                                    break;
                                case 202:
                                    style = TextStyle{ TextStyle::Bold, Color::Green };
                                    break;
                                case 203:
                                    style = TextStyle{ TextStyle::Bold, Color::Black };
                                    break;                            
                            }
                        }
                    }
                } else {
                    switch (mapSaved[i][j]) {
                        case 1:
                            symbol = ' ';
                            break;
                        case 2:
                            symbol = '#';
                            break;
                        case 100:
                            symbol = '%';
                            break;
                        case 101:
                            symbol = '%';
                            style += Color::Red;
                            break;
                        case 300:
                            symbol = '&';
                            style += Color::Black;
                            style += TextStyle::Bold;
                            break;
                        case 301:
                            symbol = '&';
                            style += Color::Yellow;
                            break;
                        case 400:
                            symbol = '/';
                            style += Color::Red;
                            style += TextStyle::Bold;
                            break;
                        case 401:
                            symbol = '/';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
                            break;
                        case 402:
                            symbol = '/';
                            style += TextStyle::Bold;
                            break;
                        case 403:
                            symbol = '/';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
                            break;
                        case 404:
                            symbol = '/';
                            style += Color::Black;
                            style += TextStyle::Bold;
                            break;
                        case 405:
                            symbol = '/';
                            break;
                        case 450:
                            symbol = ',';
                            style += Color::Black;
                            style += TextStyle::Bold;
                            break;
                        case 451:
                            symbol = ',';
                            style += Color::Red;
                            style += TextStyle::Bold;
                            break;
                        case 500:
                            symbol = '~';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
                            break;
                        case 501:
                            symbol = '~';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
                            break;
                        case 600:
                            symbol = '!';
                            style += Color::Blue;
                            style += TextStyle::Bold;
                            break;
                        case 601:
                            symbol = '!';
                            style += Color::Green;
                            break;
                        case 602:
                            symbol = '!';
                            style += Color::Black;
                            style += TextStyle::Bold;
                            break;
                        case 603:
                            symbol = '!';
                            style += Color::Magenta;
                            style += TextStyle::Bold;
                            break;
                        case 604:
                            symbol = '!';
                            style += Color::Yellow;
                            break;
                        case 700:
                            symbol = '\\';
                            style += Color::Yellow;
                            break;
                    }
                }
            } else {
                symbol = ' ';
            }
            termRend.put(symbol, style);
        }
        termRend.put('\n');
    }
}

#endif

void printMenu(const std::vector<std::string_view> & items, int active) {
    TextStyle activeItemStyle{ TextStyle::Bold, Color::Red };
    std::vector<TextStyle> itemStyles(items.size());
    itemStyles[active - 1] = activeItemStyle;
    for (int i = 1; i <= items.size(); ++i) {
        termRend
            .setCursorPosition(Vec2i{ 0, i })
            .put("{} {}"_format(i, items[i - 1]), itemStyles[i - 1]);
    }
}

std::optional<std::string> processMenu(std::string_view title, const std::vector<std::string_view> & items, bool canExit = true) {
    int selected = 1;
    int itemsCount = items.size();
    while (true) {
        termRend
            .clear()
            .setCursorPosition(Vec2i{})
            .put(title);

        printMenu(items, selected);

        char input = termRead.readChar();
        switch (input) {
            case CONTROL_DOWN:
                selected = std::min(selected + 1, itemsCount);
                break;
            case CONTROL_UP:
                selected = std::max(selected - 1, 1);
                break;
            case '\033':
                if (canExit) {
                    termRend.clear();
                    return {};
                }
                break;
            case CONTROL_CONFIRM:
                return std::string{ items[selected - 1] };
        }
    }
}

void mSettingsMode() {
    auto result = processMenu("Choose mode", {
            "Normal",
            "Hard"});

    if (result == "Normal") {
        g_mode = 1;
    } else if (result == "Hard") {
        g_mode = 2;
    }
}

void mSettingsMap() {
    termRend
        .clear()
        .setCursorPosition(Vec2i{})
        .put("Do you want to load map from file?");
    char inpChar = termRead.readChar();
    if (inpChar == 'y' || inpChar == 'Y') {
        g_generateMap = false;
    }
    termRend.clear();
}

void mSettings() {
    while (true) {
        auto result = processMenu("Settings", {
                "Mode",
                "Maps"});

        if (result == "Mode") {
            mSettingsMode();
        } else if (result == "Maps") {
            mSettingsMap();
        } else {
            return;
        }
    }
}

std::vector<std::string> readTips() {
    std::ifstream file{ "tips.txt" };
    std::vector<std::string> tips;

    bool done = false;
    while (not done) {
        std::string line;
        std::getline(file, line);

        if (not file) {
            done = true;
        } else {
            tips.emplace_back(std::move(line));
        }
    }

    return tips;
}

void mainMenu() {
    auto tips = readTips();

    while (true) {
        auto tip = tips[rand() % tips.size()];
        auto result = processMenu("Welcome to RLRPG /* Tip of the day: {} */"_format(tip), {
            "Start game",
            "Settings",
            "Exit"
        }, false);

        if (result == "Start game") {
            return;
        } else if (result == "Settings") {
            mSettings();
        } else if (result == "Exit") {
            g_exit = true;
            return;
        }
    }
}

void readMap() {
    std::ifstream file{ "map.me" };
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            file >> map[i][j];
        }
    }
}

int getLevelUpXP(int level) {
    return level * level + 4;
}

int g_levelUpXP = getLevelUpXP(hero.level);

void getXP() {
    if (hero.xp > g_levelUpXP) {
        hero.level++;
        message += fmt::format("Now you are level {}. ", hero.level);
        g_maxBurden += g_maxBurden / 4;
        DEFAULT_HERO_HEALTH += DEFAULT_HERO_HEALTH / 4;
        hero.health = DEFAULT_HERO_HEALTH;
        g_levelUpXP = getLevelUpXP(hero.level * hero.level + 4);
    }
}

void setRandomPotionEffects() {
    for (int i = 0; i < TYPES_OF_POTIONS; i++) {
        int rv = rand() % TYPES_OF_POTIONS;
        if (differentPotion[rv].effect == 0) {
            differentPotion[rv].effect = i + 1;
        } else {
            i--;
        }
    }
}

int main() {
    termRead.setEchoing(false);

    initialize();
    
    mainMenu();
    if (g_exit) {     
        return 0;
    }

    if (g_generateMap) {
        generate_maze();
    } else {
        readMap();
    }

    hero.health = DEFAULT_HERO_HEALTH;
    hero.symbol = 200;
    hero.hunger = 900;

    Food Egg(0);
    Food Apple(1);
    differentFood[0] = Egg;
    differentFood[1] = Apple;
    
    Armor ChainChestplate(0);
    Armor LeatherChestplate(1);
    differentArmor[0] = ChainChestplate;
    differentArmor[1] = LeatherChestplate;

    inventory[0] = LeatherChestplate;
    inventory[0].getItem().inventorySymbol = 'a';

    hero.heroArmor = &inventory[0];
    hero.heroArmor->getItem().attribute = 201;
    if (rand() % (500 / hero.luck) == 0)
        hero.heroArmor->getItem().mdf = 2;
    
    Weapon CopperShortsword(0);
    Weapon BronzeSpear(1);
    Weapon Musket(2);
    Weapon Stick(3);
    Weapon Shotgun(4);
    Weapon Pistol(5);
    differentWeapon[0] = CopperShortsword;
    differentWeapon[1] = BronzeSpear;
    differentWeapon[2] = Musket;
    differentWeapon[3] = Stick;
    differentWeapon[4] = Shotgun;
    differentWeapon[5] = Pistol;
    
    Tools Pickaxe(0);
    differentTools[0] = Pickaxe;

    Ammo SteelBullets(0);
    Ammo ShotgunShells(1);
    differentAmmo[0] = SteelBullets;
    differentAmmo[1] = ShotgunShells;
    
    Scroll MapScroll(0);
    Scroll IdentifyScrollBitch(1);
    differentScroll[0] = MapScroll;
    differentScroll[1] = IdentifyScrollBitch;

    Potion BluePotion(0);
    Potion GreenPotion(1);
    Potion DarkPotion(2);
    Potion lol(3);
    Potion kek(4);
    differentPotion[0] = BluePotion;
    differentPotion[1] = GreenPotion;
    differentPotion[2] = DarkPotion;
    differentPotion[3] = lol;
    differentPotion[4] = kek;
    
    setRandomPotionEffects();

    Enemy Barbarian(0);
    Enemy Zombie(1);
    Enemy Guardian(2);
    differentEnemies[0] = Barbarian;
    differentEnemies[1] = Zombie;
    differentEnemies[2] = Guardian;

    hero.heroWeapon = &inventory[EMPTY_SLOT];

    setItems();

    spawnUnits();

    hero.checkVisibleCells();

    int TurnsCounter = 0;
    
    draw();
            
    termRend.setCursorPosition(Vec2i{ 0, FIELD_ROWS} );
    bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
            hero.health,
            hero.hunger,
            hero.heroArmor->item.invArmor.defence,
            hero.heroWeapon->item.invWeapon.damage,
            hero.level, hero.xp,
            hero.luck);

    bar += "Bul: |";
    for (int i = 0; i < BANDOLIER; i++) {
        if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
            bar += fmt::format("{}|", inventory[AMMO_SLOT + i].item.invAmmo.count);
        } else {
            bar += "0|";
        }
    }
    bar += " ";
    if (hero.isBurdened)
        bar += "Burdened. ";
    termRend.put(fmt::sprintf("%- 190s", bar));
    
    if (hero.heroWeapon->type != ItemEmpty) {
        weaponBar = "";
        weaponBar += hero.heroWeapon->getItem().getName();
        if (hero.heroWeapon->item.invWeapon.Ranged) {
            weaponBar += "[";
            for (int i = 0; i < hero.heroWeapon->item.invWeapon.cartridgeSize; i++) {
                if (i < hero.heroWeapon->item.invWeapon.currentCS && (hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 450 ||
                    hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 451)) {
                    weaponBar += "i";
                } else {
                    weaponBar += "_";
                }
            }
            weaponBar += "]";
        }        
        termRend
            .setCursorPosition(Vec2i{ 0, FIELD_ROWS + 1 })
            .put(fmt::sprintf("%- 190s", weaponBar));
    }

    termRend.setCursorPosition(Vec2i{ hero.posL, hero.posH });
    
    while (true) {
        if (g_exit) {     
            termRend.display();
            return 0;
        }

        message = "";
        bar = "";
    
        bool died = false;

        if (hero.hunger < 1) {
            message += "You died from starvation. Press any key to exit. ";
            died = true;
        }

        if (hero.health < 1) {
            message += "You died. Press any key to exit. ";
            died = true;
        }

        if (died) {
            hero.health = 0;
            termRend
                .setCursorPosition(Vec2i{ 0, FIELD_ROWS + 2 })
                .put(fmt::sprintf("%- 190s", message))
                .display();
            termRead.readChar();
            return 0;
        }

        termRend.setCursorPosition(Vec2i{ hero.posL, hero.posH });

        char inp = termRead.readChar();
    
        hero.moveHero(inp);

        if (!g_stop) {
            TurnsCounter++;

            if (TurnsCounter % 25 == 0 && TurnsCounter != 0 && g_mode == 1) {
                if (hero.health < DEFAULT_HERO_HEALTH) {
                    hero.health ++;
                }
            }

            hero.hunger--;
            
            if (hero.turnsInvisible > 0)
                hero.turnsInvisible--;

            if (hero.turnsBlind > 1) {
                if (hero.turnsBlind == 1) {
                    g_vision = DEFAULT_VISION;
                }
                hero.turnsBlind --;
            }
        
            if (hero.isBurdened)
                hero.hunger--;

            updateAI();
            
            ++g_turns;

            draw();

            bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
                    hero.health,
                    hero.hunger,
                    hero.heroArmor->item.invArmor.defence,
                    hero.heroWeapon->item.invWeapon.damage,
                    hero.level, hero.xp,
                    hero.luck);
            
            bar += "Bul: |";
            for (int i = 0; i < BANDOLIER; i++) {
                if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
                    bar += fmt::format("{}|", inventory[AMMO_SLOT + i].item.invAmmo.count);
                } else {
                    bar += "0|";
                }
            }
            bar += " ";
            if (hero.isBurdened)
                bar += "Burdened. ";
    
            if (hero.hunger < 75) {    
                bar += "Hungry. ";
            }

            termRend
                .setCursorPosition(Vec2i{ 0, FIELD_ROWS })
                .put(fmt::sprintf("%- 190s", bar));
        
            if (hero.heroWeapon->type != ItemEmpty) {
                weaponBar = "";
                weaponBar += hero.heroWeapon->getItem().getName();
                if (hero.heroWeapon->item.invWeapon.Ranged) {
                    weaponBar += "[";
                    for (int i = 0; i < hero.heroWeapon->item.invWeapon.cartridgeSize; i++) {
                        if (i < hero.heroWeapon->item.invWeapon.currentCS && (hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 450 ||
                            hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 451)) {
                            weaponBar += "i";
                        } else {
                            weaponBar += "_";
                        }
                    }
                    weaponBar += "]";
                }
                termRend
                    .setCursorPosition(Vec2i{ 0, FIELD_ROWS + 1 })
                    .put(fmt::sprintf("%- 190s", weaponBar));
            }

            termRend
                .setCursorPosition(Vec2i{ 0, FIELD_ROWS + 1 })
                .put(fmt::sprintf("%- 190s", message));
            
            if (inp == '\033') {    
                termRend
                    .setCursorPosition(Vec2i{ 0, FIELD_ROWS })
                    .put("Are you sure you want to exit?\n")
                    .display();
                char inp = termRead.readChar();
                if (inp == 'y' || inp == 'Y' || inp == CONTROL_CONFIRM) {
                    return 0;
                }
                g_stop = true;
            }    
    
            getXP();

            termRend.setCursorPosition(Vec2i{ hero.posL, hero.posH });
        } else {
            draw();

            termRend.setCursorPosition(Vec2i{ 0, FIELD_ROWS} );
            bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
                    hero.health,
                    hero.hunger,
                    hero.heroArmor->item.invArmor.defence,
                    hero.heroWeapon->item.invWeapon.damage,
                    hero.level, hero.xp,
                    hero.luck);

            bar += "Bul: |";
            for (int i = 0; i < BANDOLIER; i++) {
                if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
                    bar += fmt::format("{}|", inventory[AMMO_SLOT + i].item.invAmmo.count);
                } else {
                    bar += "0|";
                }
            }
            bar += " ";
            if (hero.isBurdened) {
                bar += "Burdened. ";
            }
        
            if (hero.hunger < 75) {
                bar += "Hungry. ";
            }
            termRend
                .put(fmt::sprintf("%- 190s", bar))
                .setCursorPosition(Vec2i{ 0, FIELD_ROWS + 2 })
                .put(fmt::sprintf("%- 190s", message));

            g_stop = false;
        }
    }
        
    termRend.display();
    stopLog();

    return 0;
}
