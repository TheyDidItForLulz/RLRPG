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
                                                '\'(Pickaxe) == 406
                                                ','(Steel bullets) == 450
                                                ','(Shotgun shells) == 451
                                                '~'(Map) == 500
                                                '~'(Identify scroll) == 501
                                                '!'(Blue potion) == 600
                                                '!'(Green potion) == 601
                                                '!'(Dark potion) == 602
                                                '!'(Magenta potion) == 603
                                                '!'(Yellow potion) == 604
*/
////////////////////////////////////////////////// Modificators ///////////////////////////////////////////////////////////
/* 1 - Nothing
                                                2 - Thorns (chance to turn damage back)
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

using fmt::format;

#include<unit.hpp>
#include<hero.hpp>
#include<enemy.hpp>
#include<controls.hpp>
#include<level.hpp>
#include<item.hpp>
#include<gen_map.hpp>
#include<utils.hpp>

// !COMMENT! // Enemies must move at first turn
int g_mode = 1;    
bool g_exit = false;
bool g_stop = false;
bool g_generateMap = true;
int g_turns = 0; /*-1*/

TerminalRenderer termRend;
TerminalReader termRead;
                                                            
void initialize() {
    std::srand(std::time(nullptr));
    initField();
}

std::string message;
std::string bar;
std::string weaponBar;

Hero * g_hero;

void updateAI() {
    unitMap.forEach([&] (Unit::Ptr & unit) {
        if (not unit or unit->getType() != Unit::Type::Enemy)
            return;

        auto & enemy = dynamic_cast<Enemy &>(*unit);
        if (enemy.lastTurnMoved == g_turns)
            return;
        if (g_mode == 2 and g_turns % 200 == 0) {
            enemy.heal(1);
        }
        enemy.updatePosition();
    });
}

void setItems() {
    randomlySelectAndSetOnMap(foodTypes, Food::COUNT);
    randomlySelectAndSetOnMap(armorTypes, Armor::COUNT, ItemSelector<Armor>([&] (const std::vector<Armor> & types) {
        Armor item = types[std::rand() % types.size()];
        if (std::rand() % 500 < g_hero->luck) {
            item.mdf = 2;
        }
        return item;
    }));
    randomlySelectAndSetOnMap(weaponTypes, Weapon::COUNT);
    randomlySelectAndSetOnMap(ammoTypes, Ammo::COUNT, ItemSelector<Ammo>([&] (const std::vector<Ammo> & types) {
        Ammo ammo = types[std::rand() % types.size()];
        ammo.count = std::rand() % g_hero->luck + 1;
        return ammo;
    }));
    randomlySelectAndSetOnMap(scrollTypes, Scroll::COUNT);
    randomlySelectAndSetOnMap(potionTypes, Potion::COUNT);
}

void spawnUnits() {
    for (int i = 0; i < 1; i++) {
        Coord2i pos{ std::rand() % LEVEL_COLS, std::rand() % LEVEL_ROWS };
        if (level[pos] == 1 and not unitMap[pos]) {
            auto hero = std::make_unique<Hero>();
            g_hero = hero.get();
            g_hero->pos = pos;
            unitMap[pos] = std::move(hero);
            break;
        } else {
            i--;
        }
    }
    for (int i = 0; i < ENEMIESCOUNT; i++) {
        Coord2i pos{ std::rand() % LEVEL_COLS, std::rand() % LEVEL_ROWS };
        if (level[pos] == 1 and not unitMap[pos]) {
            int p = std::rand() % Enemy::TYPES_COUNT;
            auto enemy = std::make_unique<Enemy>(enemyTypes[p]);
            enemy->pos = pos;
            unitMap[pos] = std::move(enemy);
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
        case 406: return { '\\', { Color::Yellow } };
        case 450: return { ',', { TextStyle::Bold, Color::Black } };
        case 451: return { ',', { TextStyle::Bold, Color::Red } };
        case 500: return { '~', { TextStyle::Bold, Color::Yellow } };
        case 501: return { '~', { TextStyle::Bold, Color::Yellow } };
        case 600: return { '!', { TextStyle::Bold, Color::Blue } };
        case 601: return { '!', { Color::Green } };
        case 602: return { '!', { TextStyle::Bold, Color::Black } };
        case 603: return { '!', { TextStyle::Bold, Color::Magenta } };
        case 604: return { '!', { Color::Yellow } };
        default: return { '?', { TextStyle::Bold, TerminalColor{ Color::Green, Color::Magenta } } };
    }
}

SymbolRenderData getRenderData(const Unit::Ptr & unit) {
    switch (unit->getType()) {
        case Unit::Type::Hero: return { '@', { Color::Green } };
        case Unit::Type::Enemy:
            switch (unit->symbol) {
                case 201: return { '@', { Color::Yellow } };
                case 202: return { '@', { TextStyle::Bold, Color::Green } };
                case 203: return { '@', { TextStyle::Bold, Color::Black } };
                default: throw std::logic_error("Unknown unit id");
            }
    }
    return { '?', { TextStyle::Bold, TerminalColor{ Color::Yellow, Color::Blue } } }; 
}

Array2D<std::optional<CellRenderData>, LEVEL_ROWS, LEVEL_COLS> cachedMap;

std::optional<CellRenderData> getRenderData(Coord2i cell) {
#ifndef DEBUG
    if (not seenUpdated[cell]) {
        return {};
    }
#endif
    CellRenderData renderData;
    if (unitMap[cell]) {
        renderData.unit = getRenderData(unitMap[cell]);
    }
    if (itemsMap[cell].size() == 1) {
        renderData.item = getRenderData(itemsMap[cell].front());
    } else if (itemsMap[cell].size() > 1) {
        renderData.item = SymbolRenderData{ '^', { TextStyle::Bold, TerminalColor{ Color::Black, Color::White } } };
    }
    switch (level[cell]) {
        case 1:
            if (seenUpdated[cell]) {
                renderData.level = '.';
            } else {
                renderData.level = ' ';
            }
            break;
        case 2:
            if (seenUpdated[cell]) {
                renderData.level = SymbolRenderData{ '#', { TextStyle::Bold } };
            } else {
                renderData.level = '#';
            }
            break;
        default:
            throw std::logic_error(format("Unknown block id: {}", level[cell]));
    }
    return renderData;
}

void clearCachedMap() {
    cachedMap.forEach([] (std::optional<CellRenderData> & cell) {
        cell = std::nullopt;
    });
}

void cache(Coord2i cell, const CellRenderData & renderData) {
    cachedMap[cell] = renderData.forCache();
}

void drawMap(){
    termRend.setCursorPosition(Coord2i{});

    if (g_mode == 2 and not g_hero->isMapInInventory())
        clearCachedMap();

    cachedMap.forEach([&] (Coord2i pos, std::optional<CellRenderData> & cellCache) {
        auto cell = getRenderData(pos);

        if (cell)
            cellCache = cell->forCache();
        else if (cellCache)
            cell = cellCache;

        auto rendData = cell->get().value_or(' ');

        termRend
            .setCursorPosition(pos)
            .put(rendData.symbol, rendData.style);
    });
}

void printMenu(const std::vector<std::string_view> & items, int active) {
    TextStyle activeItemStyle{ TextStyle::Bold, Color::Red };
    std::vector<TextStyle> itemStyles(items.size());
    itemStyles[active - 1] = activeItemStyle;
    for (int i = 1; i <= items.size(); ++i) {
        termRend
            .setCursorPosition(Coord2i{ 0, i })
            .put(format("{} {}", i, items[i - 1]), itemStyles[i - 1]);
    }
}

std::optional<std::string> processMenu(std::string_view title, const std::vector<std::string_view> & items, bool canExit = true) {
    int selected = 1;
    int itemsCount = (int) items.size();
    while (true) {
        termRend
            .clear()
            .setCursorPosition(Coord2i{})
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
            default:
                break;
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
        .setCursorPosition(Coord2i{})
        .put("Do you want to load map from file?");
    char inpChar = termRead.readChar();
    if (inpChar == 'y' or inpChar == 'Y') {
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
        std::string title = "Welcome to RLRPG";
        if (not tips.empty()) {
            const auto & tip = tips[std::rand() % tips.size()];
            title = format("{} /* Tip of the day: {} */", title, tip);
        }

        auto result = processMenu(title, {
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

void setRandomPotionEffects() {
    for (int i = 0; i < Potion::TYPES_COUNT; i++) {
        int rv = rand() % Potion::TYPES_COUNT;
        if (potionTypes[rv].effect == 0) {
            potionTypes[rv].effect = i + 1;
        } else {
            i--;
        }
    }
}

void draw() {
    termRend.clear();
    drawMap();

    int defence = g_hero->armor ? g_hero->armor->defence : 0;
    int damage = g_hero->weapon ? g_hero->weapon->damage : 0;
    bar += format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
            g_hero->health,
            g_hero->hunger,
            defence,
            damage,
            g_hero->level, g_hero->xp,
            g_hero->luck);

    if (g_hero->isBurdened)
        bar += "Burdened. ";

    if (g_hero->hunger < 75) {    
        bar += "Hungry. ";
    }

    if (g_hero->weapon != nullptr) {
        weaponBar = "";
        weaponBar += g_hero->weapon->getName();
        if (g_hero->weapon->isRanged) {
            weaponBar += "[";
            for (int i = 0; i < g_hero->weapon->cartridge.getCapacity(); i++) {
                if (g_hero->weapon->cartridge[i]) {
                    weaponBar += "i";
                } else {
                    weaponBar += "_";
                }
            }
            weaponBar += "]";
        }
    }
    termRend
        .setCursorPosition(Coord2i{ 0, LEVEL_ROWS })
        .put(fmt::sprintf("%- 190s", bar))
        .setCursorPosition(Coord2i{ 0, LEVEL_ROWS + 1 })
        .put(fmt::sprintf("%- 190s", weaponBar))
        .setCursorPosition(Coord2i{ 0, LEVEL_ROWS + 2 })
        .put(fmt::sprintf("%- 190s", message))
        .setCursorPosition(g_hero->pos);
}

int main() {
    termRead.setEchoing(false);

    initialize();
    
    mainMenu();
    if (g_exit) { 
        return 0;
    }

    if (g_generateMap) {
        generateMaze();
    } else {
        readMap();
    }

    Food Egg(0);
    Food Apple(1);
    foodTypes = { Egg, Apple };
    
    Armor ChainChestplate(0);
    Armor LeatherChestplate(1);
    armorTypes = { ChainChestplate, LeatherChestplate };

    Weapon CopperShortsword(0);
    Weapon BronzeSpear(1);
    Weapon Musket(2);
    Weapon Stick(3);
    Weapon Shotgun(4);
    Weapon Pistol(5);
    Weapon Pickaxe(6);
    weaponTypes = {
        CopperShortsword,
        BronzeSpear,
        Musket,
        Stick,
        Shotgun,
        Pistol,
        Pickaxe
    };

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
    enemyTypes[0] = Barbarian;
    enemyTypes[1] = Zombie;
    enemyTypes[2] = Guardian;

    spawnUnits();

    setItems();

    g_hero->checkVisibleCells();

    draw();
    
    while (true) {
        if (g_exit) {     
            return 0;
        }

        message = "";
        bar = "";
        weaponBar = "";
    
        bool died = false;

        if (g_hero->hunger < 1) {
            message += "You died from starvation. Press any key to exit. ";
            died = true;
        }

        if (g_hero->health < 1) {
            message += "You died. Press any key to exit. ";
            died = true;
        }

        if (died) {
            g_hero->health = 0;
            termRend
                .setCursorPosition(Coord2i{ 0, LEVEL_ROWS + 2 })
                .put(fmt::sprintf("%- 190s", message))
                .display();
            termRead.readChar();
            return 0;
        }

        termRend.setCursorPosition(g_hero->pos);

        char inp = termRead.readChar();
        g_hero->processInput(inp);
        g_hero->checkVisibleCells();

        if (!g_stop) {
            updateAI();
            
            ++g_turns;

            if (g_turns % 25 == 0 and g_turns != 0 and g_mode == 1) {
                g_hero->heal(1);
            }

            g_hero->hunger--;
            
            if (g_hero->turnsInvisible > 0)
                g_hero->turnsInvisible--;

            if (g_hero->turnsBlind > 1) {
                if (g_hero->turnsBlind == 1) {
                    g_hero->vision = Hero::DEFAULT_VISION;
                }
                g_hero->turnsBlind --;
            }
        
            if (g_hero->isBurdened)
                g_hero->hunger--;

            draw();
            
            if (inp == '\033') {    
                termRend
                    .setCursorPosition(Coord2i{ 0, LEVEL_ROWS })
                    .put("Are you sure you want to exit?\n")
                    .display();
                char confirmExit = termRead.readChar();
                if (confirmExit == 'y' or confirmExit == 'Y' or confirmExit == CONTROL_CONFIRM) {
                    return 0;
                }
                g_stop = true;
            }    
    
            g_hero->tryLevelUp();

            termRend.setCursorPosition(g_hero->pos);
        } else {
            draw();
            g_stop = false;
        }
    }
}
