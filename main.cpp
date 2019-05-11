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
/* 1 - Nothing
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

// !COMMENT! // Enemies must move at first turn
int g_mode = 1;    
bool g_exit = false;
bool g_stop = false;
bool g_generateMap = true;
int g_turns = 0; /*-1*/

TerminalRenderer termRend;
TerminalReader termRead;
                                                            
void initialize() {
    std::srand(std::time(0));
    initField();
}

std::string message = "";
std::string bar = "";
std::string weaponBar = "";

Hero g_hero;

void updateAI() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (unitMap[i][j].type == UnitEnemy && unitMap[i][j].unit.uEnemy.movedOnTurn != g_turns) {
#                ifdef DEBUG
                message += "{{{}|{}|{}|{}}}"_format(i, j, unitMap[i][j].unit.uEnemy.symbol, unitMap[i][j].getUnit().health);
#                endif
                if (g_mode == 2 && g_turns % 200 == 0) {
                    unitMap[i][j].getUnit().heal(1);
                }
                unitMap[i][j].unit.uEnemy.updatePosition();
            }
        }
    }
}

void setItems() {
    randomlySelectAndSetOnMap(foodTypes, FOOD_COUNT);
    randomlySelectAndSetOnMap(armorTypes, ARMOR_COUNT, [&] (const auto & types) {
        Armor item = types[std::rand() % types.size()];
        if (std::rand() % 500 < g_hero.luck) {
            item.mdf = 2;
        }
        return item;
    });
    randomlySelectAndSetOnMap(weaponTypes, WEAPON_COUNT);
    randomlySelectAndSetOnMap(ammoTypes, AMMO_COUNT, [&] (const auto & types) {
        Ammo ammo = types[std::rand() % types.size()];
        ammo.count = std::rand() % g_hero.luck + 1;
        return ammo;
    });
    randomlySelectAndSetOnMap(scrollTypes, SCROLL_COUNT);
    randomlySelectAndSetOnMap(potionTypes, POTION_COUNT);
    randomlySelectAndSetOnMap(toolTypes, TOOL_COUNT);
}

void spawnUnits() {
    for (int i = 0; i < 1; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;
        if (map[h][l] == 1 && unitMap[h][l].type == UnitEmpty) {
            unitMap[h][l] = g_hero;
            g_hero.posH = h;
            g_hero.posL = l;
            break;
        } else {
            i--;
        }
    }
    for (int i = 0; i < ENEMIESCOUNT; i++) {
        int h = rand() % FIELD_ROWS;
        int l = rand() % FIELD_COLS;
        if (map[h][l] == 1 && unitMap[h][l].type == UnitEmpty) {
            int p = rand() % Enemy::TYPES_COUNT;
            unitMap[h][l] = differentEnemies[p];
            unitMap[h][l].getUnit().posH = h;
            unitMap[h][l].getUnit().posL = l;
        } else {
            i--;
        }
    }
}

struct SymbolRenderData {
    TextStyle style;
    char symbol;
    

    SymbolRenderData(char symbol, TextStyle style = {})
        : symbol(symbol)
        , style(style) {}
};

struct CellRenderData {
    std::optional<SymbolRenderData> level;
    std::optional<SymbolRenderData> item;
    std::optional<SymbolRenderData> unit;

    std::optional<SymbolRenderData> get() const {
        if (unit.has_value())
            return unit;
        if (item.has_value())
            return item;
        return level;
    }

    CellRenderData forCache() const {
        CellRenderData cached;
        cached.item = item;
        if (level.has_value() and level->symbol != '.') {
            cached.level = level;
        }
        return cached;
    }
};

SymbolRenderData getRenderData(const Item::Ptr & item) {
    switch (item->symbol) {
        case 100: return '%';
        case 101: return { '%', { Color::Red } };
        case 300: return { '&', { TextStyle::Bold, Color::Black } };
        case 301: return { '&', { Color::Yellow } };
        case 400: return { '/', { TextStyle::Bold, Color::Red } };
        case 401: return { '/', { TextStyle::Bold, Color::Yellow } };
        case 402: return { '/', { TextStyle::Bold } };
        case 403: return { '/', { TextStyle::Bold, Color::Yellow } };
        case 404: return { '/', { TextStyle::Bold, Color::Black } };
        case 405: return '/';
        case 450: return { ',', { TextStyle::Bold, Color::Black } };
        case 451: return { ',', { TextStyle::Bold, Color::Red } };
        case 500: return { '~', { TextStyle::Bold, Color::Yellow } };
        case 501: return { '~', { TextStyle::Bold, Color::Yellow } };
        case 600: return { '!', { TextStyle::Bold, Color::Blue } };
        case 601: return { '!', { Color::Green } };
        case 602: return { '!', { TextStyle::Bold, Color::Black } };
        case 603: return { '!', { TextStyle::Bold, Color::Magenta } };
        case 604: return { '!', { Color::Yellow } };
        case 700: return { '\\', { Color::Yellow } };
        default: return { '?', { TextStyle::Bold, TerminalColor{ Color::Green, Color::Magenta } } };
    }
}

SymbolRenderData getRenderData(const PossibleUnit & unit) {
    switch (unit.type) {
        case UnitHero: return { '@', { Color::Green } };
        case UnitEnemy:
            switch (unit.getUnit().symbol) {
                case 201: return { '@', { Color::Yellow } };
                case 202: return { '@', { TextStyle::Bold, Color::Green } };
                case 203: return { '@', { TextStyle::Bold, Color::Black } };
            }
    }
    return { '?', { TextStyle::Bold, TerminalColor{ Color::Yellow, Color::Blue } } }; 
}

std::optional<CellRenderData> cachedMap[FIELD_ROWS][FIELD_COLS];

std::optional<CellRenderData> getRenderData(int row, int col) {
#ifndef DEBUG
    if (not seenUpdated[row][col]) {
        return {};
    }
#endif
    CellRenderData renderData;
    if (unitMap[row][col].type != UnitEmpty) {
        renderData.unit = getRenderData(unitMap[row][col]);
    }
    if (itemsMap[row][col].size() == 1) {
        renderData.item = getRenderData(itemsMap[row][col].front());
    } else if (itemsMap[row][col].size() > 1) {
        renderData.item = SymbolRenderData{ '^', { TextStyle::Bold, TerminalColor{ Color::Black, Color::White } } };
    }
    switch (map[row][col]) {
        case 1:
            if (seenUpdated[row][col]) {
                renderData.level = '.';
            } else {
                renderData.level = ' ';
            }
            break;
        case 2:
            if (seenUpdated[row][col]) {
                renderData.level = SymbolRenderData{ '#', { TextStyle::Bold } };
            } else {
                renderData.level = '#';
            }
    }
    return renderData;
}

void clearCachedMap() {
    for (int r = 0; r < FIELD_ROWS; ++r) {
        for (int c = 0; c < FIELD_COLS; ++c) {
            cachedMap[r][c] = {};
        }
    }
}

void cache(int row, int col, const CellRenderData & renderData) {
    cachedMap[row][col] = renderData.forCache();
}

void drawMap(){
    termRend.setCursorPosition(Vec2i{});

    if (g_mode == 2 and not g_hero.isMapInInventory())
        clearCachedMap();

    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            auto cell = getRenderData(i, j);

            if (cell)
                cache(i, j, *cell);
            else if (cachedMap[i][j])
                cell = cachedMap[i][j];

            auto symbol = cell->get().value_or(' ');

            termRend.put(symbol.symbol, symbol.style);
        }
        termRend.put('\n');
    }
}

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

int getLevelUpXP(int level) {
    return level * level + 4;
}

int g_levelUpXP = getLevelUpXP(g_hero.level);

void getXP() {
    if (g_hero.xp > g_levelUpXP) {
        g_hero.level++;
        message += fmt::format("Now you are level {}. ", g_hero.level);
        g_maxBurden += g_maxBurden / 4;
        g_hero.maxHealth += g_hero.maxHealth / 4;
        g_hero.health = g_hero.maxHealth;
        g_levelUpXP = getLevelUpXP(g_hero.level * g_hero.level + 4);
    }
}

void setRandomPotionEffects() {
    for (int i = 0; i < TYPES_OF_POTIONS; i++) {
        int rv = rand() % TYPES_OF_POTIONS;
        if (potionTypes[rv].effect == 0) {
            potionTypes[rv].effect = i + 1;
        } else {
            i--;
        }
    }
}

void draw() {
    drawMap();

    int defence = g_hero.heroArmor ? g_hero.heroArmor->defence : 0;
    int damage = g_hero.heroWeapon ? g_hero.heroWeapon->damage : 0;
    bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
            g_hero.health,
            g_hero.hunger,
            defence,
            damage,
            g_hero.level, g_hero.xp,
            g_hero.luck);

    bar += "Bul: |";
    for (int i = 0; i < BANDOLIER; i++) {
        if (inventory[AMMO_SLOT + i]) {
            bar += fmt::format("{}|", dynamic_cast<Ammo &>(*inventory[AMMO_SLOT + i]).count);
        } else {
            bar += "0|";
        }
    }
    bar += " ";
    if (g_hero.isBurdened)
        bar += "Burdened. ";

    if (g_hero.hunger < 75) {    
        bar += "Hungry. ";
    }

    if (g_hero.heroWeapon != nullptr) {
        weaponBar = "";
        weaponBar += g_hero.heroWeapon->getName();
        if (g_hero.heroWeapon->Ranged) {
            weaponBar += "[";
            for (int i = 0; i < g_hero.heroWeapon->cartridgeSize; i++) {
                if (i < g_hero.heroWeapon->currentCS && (g_hero.heroWeapon->cartridge[i]->symbol == 450 ||
                    g_hero.heroWeapon->cartridge[i]->symbol == 451)) {
                    weaponBar += "i";
                } else {
                    weaponBar += "_";
                }
            }
            weaponBar += "]";
        }
    }
    termRend
        .setCursorPosition(Vec2i{ 0, FIELD_ROWS })
        .put(fmt::sprintf("%- 190s", bar))
        .setCursorPosition(Vec2i{ 0, FIELD_ROWS + 1 })
        .put(fmt::sprintf("%- 190s", weaponBar))
        .setCursorPosition(Vec2i{ 0, FIELD_ROWS + 2 })
        .put(fmt::sprintf("%- 190s", message))
        .setCursorPosition(Vec2i{ g_hero.posL, g_hero.posH });
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

    Food Egg(0);
    Food Apple(1);
    foodTypes = { Egg, Apple };
    
    Armor ChainChestplate(0);
    Armor LeatherChestplate(1);
    armorTypes = { ChainChestplate, LeatherChestplate };

    inventory[0] = std::make_unique<Armor>(LeatherChestplate);
    inventory[0]->inventorySymbol = 'a';

    g_hero.heroArmor = dynamic_cast<Armor *>(inventory[0].get());
    g_hero.heroArmor->attribute = 201;
    if (rand() % (500 / g_hero.luck) == 0)
        g_hero.heroArmor->mdf = 2;
    
    Weapon CopperShortsword(0);
    Weapon BronzeSpear(1);
    Weapon Musket(2);
    Weapon Stick(3);
    Weapon Shotgun(4);
    Weapon Pistol(5);
    weaponTypes = {
        CopperShortsword,
        BronzeSpear,
        Musket,
        Stick,
        Shotgun,
        Pistol
    };
    
    Tools Pickaxe(0);
    toolTypes = { Pickaxe };

    Ammo SteelBullets(0);
    Ammo ShotgunShells(1);
    ammoTypes = { SteelBullets, ShotgunShells };
    
    Scroll MapScroll(0);
    Scroll IdentifyScrollBitch(1);
    scrollTypes = { MapScroll, IdentifyScrollBitch };

    Potion BluePotion(0);
    Potion GreenPotion(1);
    Potion DarkPotion(2);
    Potion lol(3);
    Potion kek(4);
    potionTypes = {
        BluePotion,
        GreenPotion,
        DarkPotion,
        lol,
        kek
    };
    potionTypeKnown.resize(potionTypes.size());
    
    setRandomPotionEffects();

    Enemy Barbarian(0);
    Enemy Zombie(1);
    Enemy Guardian(2);
    differentEnemies[0] = Barbarian;
    differentEnemies[1] = Zombie;
    differentEnemies[2] = Guardian;

    setItems();

    spawnUnits();

    g_hero.checkVisibleCells();

    int TurnsCounter = 0;

    draw();
    
    while (true) {
        if (g_exit) {     
            return 0;
        }

        message = "";
        bar = "";
        weaponBar = "";
    
        bool died = false;

        if (g_hero.hunger < 1) {
            message += "You died from starvation. Press any key to exit. ";
            died = true;
        }

        if (g_hero.health < 1) {
            message += "You died. Press any key to exit. ";
            died = true;
        }

        if (died) {
            g_hero.health = 0;
            termRend
                .setCursorPosition(Vec2i{ 0, FIELD_ROWS + 2 })
                .put(fmt::sprintf("%- 190s", message))
                .display();
            termRead.readChar();
            return 0;
        }

        termRend.setCursorPosition(Vec2i{ g_hero.posL, g_hero.posH });

        char inp = termRead.readChar();
    
        g_hero.moveHero(inp);

        if (!g_stop) {

            updateAI();
            
            ++g_turns;

            if (g_turns % 25 == 0 && g_turns != 0 && g_mode == 1) {
                g_hero.heal(1);
            }

            g_hero.hunger--;
            
            if (g_hero.turnsInvisible > 0)
                g_hero.turnsInvisible--;

            if (g_hero.turnsBlind > 1) {
                if (g_hero.turnsBlind == 1) {
                    g_vision = DEFAULT_VISION;
                }
                g_hero.turnsBlind --;
            }
        
            if (g_hero.isBurdened)
                g_hero.hunger--;

            draw();
            
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

            termRend.setCursorPosition(Vec2i{ g_hero.posL, g_hero.posH });
        } else {
            draw();
            g_stop = false;
        }
    }
        
    termRend.display();

    return 0;
}
