//////////////////////////////////////////////////// Symbols ///////////////////////////////////////////////////////////////
/*
                                                '_'(Floor) == 1
                                                '#'(Wall) == 2
                                                '^'(Pile) == 3
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
#include<random>
#include<assert.h>
#include<termlib/termlib.hpp>

#include<fmt/core.h>
#include<fmt/printf.h>

using fmt::format;

#include<yaml-cpp/yaml.h>

#include<units/unit.hpp>
#include<units/hero.hpp>
#include<units/enemy.hpp>
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
    if (item->id == "egg") {
        return '%';
    } else if (item->id == "apple") {
        return {'%', {Color::Red}};
    } else if (item->id == "chain_chestplate") {
        return {'&', {TextStyle::Bold, Color::Black}};
    } else if (item->id == "leather_chestplate") {
        return {'&', {Color::Yellow}};
    } else if (item->id == "copper_shortsword") {
        return {'/', {TextStyle::Bold, Color::Red}};
    } else if (item->id == "bronze_spear") {
        return {'/', {TextStyle::Bold, Color::Yellow}};
    } else if (item->id == "musket") {
        return {'/', {TextStyle::Bold}};
    } else if (item->id == "stick") {
        return {'/', {TextStyle::Bold, Color::Yellow}};
    } else if (item->id == "shotgun") {
        return {'/', {TextStyle::Bold, Color::Black}};
    } else if (item->id == "pistol") {
        return '/';
    } else if (item->id == "pickaxe" ) {
        return {'\\', {Color::Yellow}};
    } else if (item->id == "steel_bullets") {
        return {',', {TextStyle::Bold, Color::Black}};
    } else if (item->id == "shotgun_bullets") {
        return {',', {TextStyle::Bold, Color::Red}};
    } else if (item->id == "map") {
        return {'~', {TextStyle::Bold, Color::Yellow}};
    } else if (item->id == "identify_scroll") {
        return {'~', {TextStyle::Bold, Color::Yellow}};
    } else if (item->id == "blue_potion") {
        return {'!', {TextStyle::Bold, Color::Blue}};
    } else if (item->id == "green_potion") {
        return {'!', {Color::Green}};
    } else if (item->id == "dark_potion") {
        return {'!', {TextStyle::Bold, Color::Black}};
    } else if (item->id == "magenta_potion") {
        return {'!', {TextStyle::Bold, Color::Magenta}};
    } else if (item->id == "yellow_potion") {
        return {'!', {Color::Yellow}};
    } else {
        return { '?', { TextStyle::Bold, TerminalColor{ Color::Green, Color::Magenta } } };
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
    for (auto & potion : potionTypes) {
        potion.effect = Potion::Effect(std::rand() % Potion::EffectCount);
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

void initItemBase(Item & item, const YAML::Node & data) {
    item.weight = data["weight"].as<int>();
    item.id = data["id"].as<std::string>();
    item.isStackable = data["isStackable"].as<bool>();
}

YAML::Node loadItemData(std::string_view id) {
    auto filename = fmt::format("data/items/{}.yaml", id);
    return YAML::LoadFile(filename);
}

Food loadFood(std::string_view id) {
    YAML::Node item = loadItemData(id);
    Food loaded;
    initItemBase(loaded, item);
    loaded.nutritionalValue = item["food"]["nutritionalValue"].as<int>();
    return loaded;
}

Armor loadArmor(std::string_view id) {
    YAML::Node item = loadItemData(id);
    Armor loaded;
    initItemBase(loaded, item);
    loaded.durability = item["armor"]["durability"].as<int>();
    loaded.defence = item["armor"]["defence"].as<int>();
    return loaded;
}

Ammo loadAmmo(std::string_view id) {
    YAML::Node item = loadItemData(id);
    Ammo loaded;
    initItemBase(loaded, item);
    loaded.damage = item["ammo"]["damage"].as<int>();
    loaded.range = item["ammo"]["range"].as<int>();
    return loaded;
}

Weapon loadWeapon(std::string_view id) {
    YAML::Node item = loadItemData(id);
    Weapon loaded;
    initItemBase(loaded, item);
    loaded.damage = item["weapon"]["damage"].as<int>();
    if (item["weapon"]["ranged"]) {
        loaded.isRanged = true;
        loaded.range = item["weapon"]["ranged"]["range"].as<int>();
        loaded.damageBonus = item["weapon"]["ranged"]["damageBonus"].as<int>();
        int cartridgeSize = item["weapon"]["ranged"]["cartridgeSize"].as<int>();
        loaded.cartridge = Weapon::Cartridge(cartridgeSize);
        if (item["weapon"]["canDig"]) {
            loaded.canDig = item["weapon"]["canDig"].as<bool>();
        }
    }
    return loaded;
}

Scroll loadScroll(std::string_view id) {
    YAML::Node item = loadItemData(id);
    Scroll loaded;
    initItemBase(loaded, item);
    loaded.effect = item["scroll"]["effect"].as<int>();
    return loaded;
}

Potion loadPotion(std::string_view id) {
    YAML::Node item = loadItemData(id);
    Potion loaded;
    initItemBase(loaded, item);
    return loaded;
}

void loadItems() {
    YAML::Node registry = YAML::LoadFile("data/items.yaml");

    for (const auto & id : registry["food"])
        foodTypes.push_back(loadFood(id.as<std::string>()));

    for (const auto & id : registry["armor"])
        armorTypes.push_back(loadArmor(id.as<std::string>()));

    for (const auto & id : registry["weapon"])
        weaponTypes.push_back(loadWeapon(id.as<std::string>()));

    for (const auto & id : registry["ammo"])
        ammoTypes.push_back(loadAmmo(id.as<std::string>()));

    for (const auto & id : registry["scroll"])
        scrollTypes.push_back(loadScroll(id.as<std::string>()));

    for (const auto & id : registry["potion"])
        potionTypes.push_back(loadPotion(id.as<std::string>()));
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

    loadItems();

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
