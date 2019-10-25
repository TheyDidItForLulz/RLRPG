#ifndef RLRPG_GAME_HPP
#define RLRPG_GAME_HPP

#include<units/unit.hpp>
#include<item.hpp>
#include<array2d.hpp>
#include<render_data.hpp>

#include<termlib/termlib.hpp>

#include<tl/optional.hpp>

#include<string>
#include<string_view>
#include<vector>
#include<unordered_map>

class Hero;
class YAMLFileCache;

namespace YAML {
    class Node;
}

class Game {
public:
    void run();

    const Hero & getHero() const { return *hero; }
    Hero & getHero() { return *hero; }

    TerminalRenderer & getRenderer() { return termRend; }

    TerminalReader & getReader() { return termRead; }

    bool exiting() const { return exit; }

    bool needGenerateMap() const { return generateMap; }

    bool skippingUpdate() const { return stop; }
    void skipUpdate(bool skip = false) { stop = skip; }

    int getTurnNumber() const { return turns; }
    void increaseTurnNumber() { ++turns; }

    int getMode() const { return mode; }

    void addMessage(std::string_view msg);

    void initialize();

    void updateAI();
    
    void setItems();

    void spawnUnits();

    void clearBuffers();

    void displayMessages();

private:
    void printMenu(const std::vector<std::string_view> & items, int activeItem);
    tl::optional<std::string> processMenu(std::string_view title,
            const std::vector<std::string_view> & items, bool canExit = false);
    void mSettingsMode();
    void mSettingsMap();
    void mSettings();
    void mainMenu();

    std::vector<std::string> readTips() const;

    void readItemRenderData(const std::string & id, YAMLFileCache & cache);
    void readItemRenderData(YAMLFileCache & cache);
    void readUnitRenderData(const std::string & id, const YAML::Node & renderData);
    void readEnemyRenderData(const std::string & id, YAMLFileCache & cache);
    void readHeroRenderData(YAMLFileCache & cache);
    void readUnitRenderData(YAMLFileCache & cache);
    SymbolRenderData getRenderData(const Item::Ptr & item);
    SymbolRenderData getRenderData(const Unit::Ptr & unit);
    tl::optional<CellRenderData> getRenderData(Coord2i cell);
    void clearCachedMap();
    void drawMap();
    void draw();

    void loadData();

    void setRandomPotionEffects();

    TerminalRenderer termRend;
    TerminalReader termRead;

    std::unordered_map<std::string, SymbolRenderData> itemRenderData;
    std::unordered_map<std::string, SymbolRenderData> unitRenderData;
    Array2D<tl::optional<CellRenderData>, LEVEL_ROWS, LEVEL_COLS> cachedMap;


    std::string message;
    std::string bar;
    std::string weaponBar;

    Hero * hero;

    int mode = 1;
    int turns = 0;
    bool exit = false;
    bool stop = false;
    bool generateMap = true;
};

extern Game g_game;

#endif //RLRPG_GAME_HPP

