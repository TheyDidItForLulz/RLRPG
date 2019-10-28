#include<game.hpp>

#include<items/food.hpp>
#include<items/armor.hpp>
#include<items/ammo.hpp>
#include<items/weapon.hpp>
#include<items/potion.hpp>
#include<items/scroll.hpp>
#include<gen_map.hpp>
#include<level.hpp>
#include<units/unit.hpp>
#include<units/hero.hpp>
#include<units/enemy.hpp>
#include<yaml_item_loader.hpp>
#include<yaml_file_cache.hpp>
#include<yaml_unit_loader.hpp>
#include<controls.hpp>

#include<fmt/core.h>
#include<fmt/printf.h>

#include<effolkronium/random.hpp>

#include<memory>
#include<fstream>
#include<sstream>

using namespace std::string_view_literals;
using fmt::format;
using Random = effolkronium::random_static;

Game g_game;

void Game::setHeroTemplate(Ptr<Hero> newHeroTemplate) {
    heroTemplate = std::move(newHeroTemplate);
}

void Game::printMenu(std::vector<std::string_view> const & items, int active) {
    TextStyle activeItemStyle{ TextStyle::Bold, Color::Red };
    std::vector<TextStyle> itemStyles(items.size());
    itemStyles[active - 1] = activeItemStyle;
    for (int i = 1; i <= items.size(); ++i) {
        termRend
            .setCursorPosition(Coord2i{ 0, i })
            .put(format("{} {}", i, items[i - 1]), itemStyles[i - 1]);
    }
}

tl::optional<std::string> Game::processMenu(std::string_view title, std::vector<std::string_view> const & items, bool canExit) {
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

void Game::mSettingsMode() {
    auto result = processMenu("Choose mode", {
            "Normal",
            "Hard"});

    if (result == "Normal") {
        mode = 1;
    } else if (result == "Hard") {
        mode = 2;
    }
}

void Game::mSettingsMap() {
    termRend
        .clear()
        .setCursorPosition(Coord2i{})
        .put("Do you want to load map from file?");
    char inpChar = termRead.readChar();
    if (inpChar == 'y' or inpChar == 'Y') {
        generateMap = false;
    }
    termRend.clear();
}

void Game::mSettings() {
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

std::vector<std::string> Game::readTips() const  {
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

void Game::mainMenu() {
    auto tips = readTips();

    while (true) {
        std::string title = "Welcome to RLRPG";
        if (not tips.empty()) {
            auto const & tip = *Random::get(tips);
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
            exit = true;
            return;
        }
    }
}

void Game::run() {
    termRead.setEchoing(false);

    mainMenu();
    if (exiting())
        return;

    initialize();

    draw();

    while (true) {
        if (exiting())
            return;

        clearBuffers();
    
        bool died = false;

        if (hero->hunger < 1) {
            addMessage("You died from starvation. Press any key to exit.");
            died = true;
        }

        if (hero->health < 1) {
            addMessage("You died. Press any key to exit.");
            died = true;
        }

        if (died) {
            hero->health = 0;
            displayMessages();
            termRead.readChar();
            return;
        }

        termRend.setCursorPosition(hero->pos);

        char inp = termRead.readChar();
        hero->processInput(inp);
        hero->checkVisibleCells();

        if (not skippingUpdate()) {
            updateAI();
            
            increaseTurnNumber();

            if (turns % 25 == 0 and turns != 0 and mode == 1) {
                hero->heal(1);
            }

            hero->hunger--;
            
            if (hero->turnsInvisible > 0)
                hero->turnsInvisible--;

            if (hero->turnsBlind > 0) {
                if (hero->turnsBlind == 1) {
                    hero->vision = Hero::DEFAULT_VISION;
                }
                hero->turnsBlind --;
            }
        
            if (hero->isBurdened)
                hero->hunger--;

            draw();
            
            if (inp == '\033') {    
                termRend
                    .setCursorPosition(Coord2i{ 0, LEVEL_ROWS })
                    .put("Are you sure you want to exit?\n")
                    .display();
                char confirmExit = termRead.readChar();
                if (confirmExit == 'y' or confirmExit == 'Y' or confirmExit == CONTROL_CONFIRM) {
                    return;
                }
                skipUpdate();
            }    
    
            hero->tryLevelUp();

            termRend.setCursorPosition(hero->pos);
        } else {
            draw();
            skipUpdate(false);
        }
    }
}

tl::optional<Color> toColor(std::string_view colorString) {
    if (colorString == "black") {
        return Color::Black;
    } else if (colorString == "red") {
        return Color::Red;
    } else if (colorString == "green") {
        return Color::Green;
    } else if (colorString == "yellow") {
        return Color::Yellow;
    } else if (colorString == "blue") {
        return Color::Blue;
    } else if (colorString == "magenta") {
        return Color::Magenta;
    } else if (colorString == "cyan") {
        return Color::Cyan;
    } else if (colorString == "white") {
        return Color::White;
    } else {
        return tl::nullopt;
    }
}

tl::optional<TextStyle> toTextStyle(YAML::Node const & colorData) {
    TextStyle style;
    std::istringstream iss(colorData["fg"].as<std::string>());
    std::string fgString;
    iss >> fgString;
    if (fgString == "light") {
        style += TextStyle::Bold;
        iss >> fgString;
    }
    return toColor(fgString).and_then([&colorData] (Color fg) {
        if (colorData["bg"]) {
            return toColor(colorData["bg"].as<std::string>()).and_then([fg](Color bg) {
                return tl::optional{TerminalColor{fg, bg}};
            });
        }
        return tl::optional{TerminalColor{fg}};
    }).map([&style] (TerminalColor color) {
        return style += color;
    });
}

tl::optional<SymbolRenderData> toSymbolRenderData(YAML::Node const & renderData) {
    char symbol = renderData["symbol"].as<char>();
    if (not renderData["color"])
        return SymbolRenderData{ symbol };
    else
        return toTextStyle(renderData["color"]).map([symbol](TextStyle style) {
            return SymbolRenderData{ symbol, style };
        });
}

void Game::readItemRenderData(std::string const & id, YAMLFileCache & yamlFileCache) {
    std::string filename = fmt::format("data/items/{}.yaml", id);
    auto const & itemData = yamlFileCache[filename];
    if (not itemData["render"])
        return;

    toSymbolRenderData(itemData["render"]).map([this, &id] (SymbolRenderData const & data) {
        itemRenderData.emplace(id, data);
    });
}

void Game::readItemRenderData(YAMLFileCache & yamlFileCache) {
    auto const & itemRegistry = yamlFileCache["data/items.yaml"];
    for (auto const & typeEntry : itemRegistry) {
        for (auto const & itemID : typeEntry.second) {
            readItemRenderData(itemID.as<std::string>(), yamlFileCache);
        }
    }
}

void Game::readUnitRenderData(std::string const & id, YAML::Node const & renderData) {
    if (not renderData)
        return;

    toSymbolRenderData(renderData).map([this, &id] (SymbolRenderData const & data) {
        unitRenderData.emplace(id, data);
    });
}

void Game::readHeroRenderData(YAMLFileCache & yamlFileCache) {
    YAML::Node heroData = yamlFileCache["data/units/hero.yaml"];
    readUnitRenderData("hero", heroData["render"]);
}

void Game::readEnemyRenderData(std::string const & id, YAMLFileCache & yamlFileCache) {
    std::string filename = fmt::format("data/units/enemies/{}.yaml", id);
    YAML::Node enemyData = yamlFileCache[filename];
    readUnitRenderData(id, enemyData["render"]);
}

void Game::readUnitRenderData(YAMLFileCache & yamlFileCache) {
    readHeroRenderData(yamlFileCache);

    auto const & enemyRegistry = yamlFileCache["data/units/enemies.yaml"];
    for (auto const & id : enemyRegistry) {
        readEnemyRenderData(id.as<std::string>(), yamlFileCache);
    }
}

void Game::loadData() {
    YAMLFileCache yamlFileCache;
    std::unique_ptr<AbstractItemLoader> itemLoader(new YAMLItemLoader(yamlFileCache));
    itemLoader->load();

    readItemRenderData(yamlFileCache);

    std::unique_ptr<AbstractUnitLoader> unitLoader(new YAMLUnitLoader(yamlFileCache));
    unitLoader->load();

    readUnitRenderData(yamlFileCache);
}

void Game::initialize() {
    initField();

    if (needGenerateMap())
        generateMaze();
    else
        readMap();

    loadData();

    for (auto const &[id, _] : potionTypes)
        potionTypeKnown[id] = false;

    setRandomPotionEffects();

    spawnUnits();
    setItems();

    hero->checkVisibleCells();
}

void Game::updateAI() {
    unitsMap.forEach([&] (Ptr<Unit> & unit) {
        if (not unit or unit->getType() != Unit::Type::Enemy)
            return;

        auto & enemy = dynamic_cast<Enemy &>(*unit);
        if (enemy.lastTurnMoved == turns)
            return;
        if (mode == 2 and turns % 200 == 0) {
            enemy.heal(1);
        }
        enemy.updatePosition();
    });
}

void Game::setItems() {
    randomlySelectAndSetOnMap(foodTypes, Food::COUNT);
    randomlySelectAndSetOnMap(armorTypes, Armor::COUNT, [this] (Registry<Ptr<Armor>> const & types) {
        auto item = Random::get(types)->second->clone();
        float thornsProbability = hero->luck / 500.f;
        if (Random::get<bool>(thornsProbability)) {
            item->mdf = 2;
        }
        return item;
    });
    randomlySelectAndSetOnMap(weaponTypes, Weapon::COUNT);
    randomlySelectAndSetOnMap(ammoTypes, Ammo::COUNT, [this] (Registry<Ptr<Ammo>> const & types) {
        auto ammo = Random::get(types)->second->clone();
        ammo->count = Random::get(1, hero->luck);
        return ammo;
    });
    randomlySelectAndSetOnMap(scrollTypes, Scroll::COUNT);
    randomlySelectAndSetOnMap(potionTypes, Potion::COUNT);
}

void Game::spawnUnits() {
    for (int i = 0; i < 1; i++) {
        Coord2i pos{ Random::get(0, LEVEL_COLS - 1), Random::get(0, LEVEL_ROWS - 1) };
        if (levelData[pos] == 1 and not unitsMap[pos]) {
            auto hero = heroTemplate->clone();
            this->hero = hero.get();
            this->hero->pos = pos;
            unitsMap[pos] = std::move(hero);
            break;
        } else {
            i--;
        }
    }
    for (int i = 0; i < ENEMIESCOUNT; i++) {
        Coord2i pos{ Random::get(0, LEVEL_COLS - 1), Random::get(0, LEVEL_ROWS - 1) };
        if (levelData[pos] == 1 and not unitsMap[pos]) {
            auto enemy = detail::cloneAny(enemyTypes);
            enemy->pos = pos;
            unitsMap[pos] = std::move(enemy);
        } else {
            i--;
        }
    }
}

void Game::clearBuffers() {
    message.clear();
    bar.clear();
    weaponBar.clear();
}

void Game::addMessage(std::string_view msg) {
    auto totalMessageSize = message.size() + msg.size() + 1;
    message.reserve(totalMessageSize);
    message += ' ';
    message += msg;
}

void Game::displayMessages() {
    termRend
        .setCursorPosition(Coord2i{ 0, LEVEL_ROWS + 2 })
        .put(fmt::sprintf("%- 190s", message))
        .display();
}

SymbolRenderData Game::getRenderData(Item const & item) {
    if (itemRenderData.count(item.id))
        return itemRenderData.at(item.id);
    return { '?', { TextStyle::Bold, TerminalColor{ Color::Green, Color::Magenta } } };
}

SymbolRenderData Game::getRenderData(Unit const & unit) {
    if (unitRenderData.count(unit.id))
        return unitRenderData.at(unit.id);
    return { '?', { TextStyle::Bold, TerminalColor{ Color::Magenta, Color::Green } } };
}

tl::optional<CellRenderData> Game::getRenderData(Coord2i cell) {
    bool const heroSeenThisCell = hero->seenUpdated(cell);
    if (not heroSeenThisCell)
        return tl::nullopt;

    CellRenderData renderData;
    if (unitsMap[cell]) {
        renderData.unit = getRenderData(*unitsMap[cell]);
    }
    if (itemsMap[cell].size() == 1) {
        renderData.item = getRenderData(*itemsMap[cell].front());
    } else if (itemsMap[cell].size() > 1) {
        renderData.item = SymbolRenderData{ '^', { TextStyle::Bold, TerminalColor{ Color::Black, Color::White } } };
    }
    switch (levelData[cell]) {
        case 1:
            if (heroSeenThisCell) {
                renderData.level = '.';
            } else {
                renderData.level = ' ';
            }
            break;
        case 2:
            if (heroSeenThisCell) {
                renderData.level = SymbolRenderData{ '#', { TextStyle::Bold } };
            } else {
                renderData.level = '#';
            }
            break;
        default:
            throw std::logic_error(format("Unknown block id: {}", levelData[cell]));
    }
    return renderData;
}

void Game::clearCachedMap() {
    cachedMap.forEach([] (tl::optional<CellRenderData> & cell) {
        cell = tl::nullopt;
    });
}

void Game::drawMap() {
    termRend.setCursorPosition(Coord2i{});

    if (mode == 2 and not hero->isMapInInventory())
        clearCachedMap();

    cachedMap.forEach([&] (Coord2i pos, tl::optional<CellRenderData> & cellCache) {
        auto cell = getRenderData(pos);

        if (cell.has_value())
            cellCache = cell->forCache();

        auto rendData = cell.disjunction(cellCache)->get().value_or(' ');

        termRend
            .setCursorPosition(pos)
            .put(rendData.symbol, rendData.style);
    });
}

void Game::setRandomPotionEffects() {
    for (auto & [id, potion] : potionTypes) {
        potion->effect = Potion::Effect(Random::get(0, Potion::EffectCount - 1));
    }
}

void Game::draw() {
    termRend.clear();
    drawMap();

    int defence = hero->armor ? hero->armor->defence : 0;
    int damage = hero->weapon ? hero->weapon->damage : 0;
    bar += format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
            hero->health,
            hero->hunger,
            defence,
            damage,
            hero->level, hero->xp,
            hero->luck);

    if (hero->isBurdened)
        bar += "Burdened. ";

    if (hero->hunger < 75) {
        bar += "Hungry. ";
    }

    if (hero->weapon != nullptr) {
        weaponBar = "";
        weaponBar += hero->weapon->getName();
        if (hero->weapon->isRanged) {
            weaponBar += "[";
            for (int i = 0; i < hero->weapon->cartridge.getCapacity(); i++) {
                if (hero->weapon->cartridge[i]) {
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
        .setCursorPosition(hero->pos);
}

void Game::initField() {
    levelData.forEach([] (int & cell) {
        cell = 1;
    });
}

void Game::readMap() {
    std::ifstream file{ "map.me" };
    levelData.forEach([&] (int & cell) {
        file >> cell;
    });
}

ItemPile::iterator Game::findItemAt(Coord2i cell, std::string_view id) {
    auto & pile = itemsMap[cell];
    return std::find_if(begin(pile), end(pile), [id] (Ptr<Item> const & item) {
        return item->id == id;
    });
}

bool Game::randomlySetOnMap(Ptr<Item> item) {
    int const attemts = 32;

    for (int i = 0; i < attemts; ++i) {
        Coord2i cell{ Random::get(0, LEVEL_COLS - 1),
					  Random::get(0, LEVEL_ROWS - 1) };

        if (g_game.level()[cell] == 1) {
            drop(std::move(item), cell);
            return true;
        }
    }

    return false;
}

void Game::drop(Ptr<Item> item, Coord2i cell) {
    if (g_game.level()[cell] == 2)
        throw std::logic_error("Trying to drop an item in a wall");
    if (not item)
        return;
    item->pos = cell;
    if (item->isStackable) {
        auto it = findItemAt(cell, item->id);
        if (it != end(itemsMap[cell])) {
            (*it)->count += item->count;
            return;
        }
    }
    itemsMap[cell].push_back(std::move(item));
}

