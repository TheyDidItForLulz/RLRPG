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
    initLog();
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
                    unitMap[i][j].getUnit().health++;
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
                bool near = abs(i - g_hero.posH) <= 1 && abs(j - g_hero.posL) <= 1;
                if (itemsMap[i][j].empty() && unitMap[i][j].type == UnitEmpty) {
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
                } else if (itemsMap[i][j].size() == 1 && unitMap[i][j].type == UnitEmpty) { /* HERE */
                    const auto & item = items[i][j].front();
                    switch (item.getItem().symbol) {
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
                } else if (itemsMap[i][j].size() > 1 && unitMap[i][j].type == UnitEmpty) { /* Here */
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

    if (g_mode == 2 && !g_hero.isMapInInventory()) {
        for (int i = 0; i < FIELD_ROWS; i++) {
            for (int j = 0; j < FIELD_COLS; j++) {
                mapSaved[i][j] = 0;
            }
        }
    }

    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (seenUpdated[i][j]) {
                switch (itemsMap[i][j].size()) {
                    case 0:
                        mapSaved[i][j] = map[i][j];
                        break;
                    case 1:
                        mapSaved[i][j] = itemsMap[i][j].front().getItem().symbol;
                        break;
                    default:
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
                bool near = abs(i - g_hero.posH) <= 1 && abs(j - g_hero.posL) <= 1;
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

int getLevelUpXP(int level) {
    return level * level + 4;
}

int g_levelUpXP = getLevelUpXP(g_hero.level);

void getXP() {
    if (g_hero.xp > g_levelUpXP) {
        g_hero.level++;
        message += fmt::format("Now you are level {}. ", g_hero.level);
        g_maxBurden += g_maxBurden / 4;
        DEFAULT_HERO_HEALTH += DEFAULT_HERO_HEALTH / 4;
        g_hero.health = DEFAULT_HERO_HEALTH;
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

    g_hero.health = DEFAULT_HERO_HEALTH;
    g_hero.symbol = 200;
    g_hero.hunger = 900;

    Food Egg(0);
    Food Apple(1);
    foodTypes = { Egg, Apple };
    
    Armor ChainChestplate(0);
    Armor LeatherChestplate(1);
    armorTypes = { ChainChestplate, LeatherChestplate };

    inventory[0] = LeatherChestplate;
    inventory[0].getItem().inventorySymbol = 'a';

    g_hero.heroArmor = &inventory[0];
    g_hero.heroArmor->getItem().attribute = 201;
    if (rand() % (500 / g_hero.luck) == 0)
        g_hero.heroArmor->getItem().mdf = 2;
    
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

    g_hero.heroWeapon = &inventory[Hero::EMPTY_SLOT];

    setItems();

    spawnUnits();

    g_hero.checkVisibleCells();

    int TurnsCounter = 0;
    
    draw();
            
    termRend.setCursorPosition(Vec2i{ 0, FIELD_ROWS} );
    bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
            g_hero.health,
            g_hero.hunger,
            g_hero.heroArmor->item.invArmor.defence,
            g_hero.heroWeapon->item.invWeapon.damage,
            g_hero.level, g_hero.xp,
            g_hero.luck);

    bar += "Bul: |";
    for (int i = 0; i < BANDOLIER; i++) {
        if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
            bar += fmt::format("{}|", inventory[AMMO_SLOT + i].item.invAmmo.count);
        } else {
            bar += "0|";
        }
    }
    bar += " ";
    if (g_hero.isBurdened)
        bar += "Burdened. ";
    termRend.put(fmt::sprintf("%- 190s", bar));
    
    if (g_hero.heroWeapon->type != ItemEmpty) {
        weaponBar = "";
        weaponBar += g_hero.heroWeapon->getItem().getName();
        if (g_hero.heroWeapon->item.invWeapon.Ranged) {
            weaponBar += "[";
            for (int i = 0; i < g_hero.heroWeapon->item.invWeapon.cartridgeSize; i++) {
                if (i < g_hero.heroWeapon->item.invWeapon.currentCS && (g_hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 450 ||
                    g_hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 451)) {
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

    termRend.setCursorPosition(Vec2i{ g_hero.posL, g_hero.posH });
    
    while (true) {
        if (g_exit) {     
            termRend.display();
            return 0;
        }

        message = "";
        bar = "";
    
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
            TurnsCounter++;

            if (TurnsCounter % 25 == 0 && TurnsCounter != 0 && g_mode == 1) {
                if (g_hero.health < DEFAULT_HERO_HEALTH) {
                    g_hero.health ++;
                }
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

            updateAI();
            
            ++g_turns;

            draw();

            bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
                    g_hero.health,
                    g_hero.hunger,
                    g_hero.heroArmor->item.invArmor.defence,
                    g_hero.heroWeapon->item.invWeapon.damage,
                    g_hero.level, g_hero.xp,
                    g_hero.luck);
            
            bar += "Bul: |";
            for (int i = 0; i < BANDOLIER; i++) {
                if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
                    bar += fmt::format("{}|", inventory[AMMO_SLOT + i].item.invAmmo.count);
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

            termRend
                .setCursorPosition(Vec2i{ 0, FIELD_ROWS })
                .put(fmt::sprintf("%- 190s", bar));
        
            if (g_hero.heroWeapon->type != ItemEmpty) {
                weaponBar = "";
                weaponBar += g_hero.heroWeapon->getItem().getName();
                if (g_hero.heroWeapon->item.invWeapon.Ranged) {
                    weaponBar += "[";
                    for (int i = 0; i < g_hero.heroWeapon->item.invWeapon.cartridgeSize; i++) {
                        if (i < g_hero.heroWeapon->item.invWeapon.currentCS && (g_hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 450 ||
                            g_hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 451)) {
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

            termRend.setCursorPosition(Vec2i{ g_hero.posL, g_hero.posH });
        } else {
            draw();

            termRend.setCursorPosition(Vec2i{ 0, FIELD_ROWS} );
            bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
                    g_hero.health,
                    g_hero.hunger,
                    g_hero.heroArmor->item.invArmor.defence,
                    g_hero.heroWeapon->item.invWeapon.damage,
                    g_hero.level, g_hero.xp,
                    g_hero.luck);

            bar += "Bul: |";
            for (int i = 0; i < BANDOLIER; i++) {
                if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
                    bar += fmt::format("{}|", inventory[AMMO_SLOT + i].item.invAmmo.count);
                } else {
                    bar += "0|";
                }
            }
            bar += " ";
            if (g_hero.isBurdened) {
                bar += "Burdened. ";
            }
        
            if (g_hero.hunger < 75) {
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
