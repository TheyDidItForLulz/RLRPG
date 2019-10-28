#ifndef RLRPG_GAME_HPP
#define RLRPG_GAME_HPP

#include<array2d.hpp>
#include<render_data.hpp>
#include<level.hpp>
#include<registry.hpp>
#include<meta/check.hpp>
#include<ptr.hpp>

#include<effolkronium/random.hpp>

#include<termlib/termlib.hpp>

#include<tl/optional.hpp>

#include<string>
#include<string_view>
#include<vector>
#include<unordered_map>
#include<list>
#include<memory>
#include<type_traits>

class Unit;
class Hero;
class Enemy;

class Item;
class Food;
class Armor;
class Weapon;
class Ammo;
class Scroll;
class Potion;

class YAMLFileCache;

namespace YAML {
    class Node;
}

using ItemPile = std::list<Ptr<Item>>;

namespace detail {
    template<class T, meta::Check<IsClonable<T>> = meta::Checked>
    Ptr<T> cloneAny(Registry<Ptr<T>> const & reg) {
        return effolkronium::random_static::get(reg)->second->clone();
    }

    template<class Fn, class ItemType>
    constexpr bool IsItemSelector = std::is_invocable_r_v<Ptr<ItemType>, Fn, Registry<Ptr<ItemType>> const &>;
}

class Game {
public:
    void run();

    LevelData const & level() const { return levelData; }
    LevelData       & level()       { return levelData; }

    Hero const & getHero() const { return *hero; }
    Hero       & getHero()       { return *hero; }

    TerminalRenderer & getRenderer() { return termRend; }

    TerminalReader & getReader() { return termRead; }

    bool exiting() const { return exit; }

    bool needGenerateMap() const { return generateMap; }

    bool skippingUpdate() const { return stop; }
    void skipUpdate(bool skip = false) { stop = skip; }

    int getTurnNumber() const { return turns; }
    void increaseTurnNumber() { ++turns; }

    int getMode() const { return mode; }

    void setHeroTemplate(Ptr<Hero> newHeroTemplate);

    auto const & getItemsMap() const { return itemsMap; }
    auto       & getItemsMap()       { return itemsMap; }

    auto const & getUnitsMap() const { return unitsMap; }
    auto       & getUnitsMap()       { return unitsMap; }

    Registry<Ptr<Food>> const & getFoodTypes() const { return foodTypes; }
    Registry<Ptr<Food>>       & getFoodTypes()       { return foodTypes; }

    Registry<Ptr<Armor>> const & getArmorTypes() const { return armorTypes; }
    Registry<Ptr<Armor>>       & getArmorTypes()       { return armorTypes; }

    Registry<Ptr<Weapon>> const & getWeaponTypes() const { return weaponTypes; }
    Registry<Ptr<Weapon>>       & getWeaponTypes()       { return weaponTypes; }

    Registry<Ptr<Ammo>> const & getAmmoTypes() const { return ammoTypes; }
    Registry<Ptr<Ammo>>       & getAmmoTypes()       { return ammoTypes; }

    Registry<Ptr<Scroll>> const & getScrollTypes() const { return scrollTypes; }
    Registry<Ptr<Scroll>>       & getScrollTypes()       { return scrollTypes; }

    Registry<Ptr<Potion>> const & getPotionTypes() const { return potionTypes; }
    Registry<Ptr<Potion>>       & getPotionTypes()       { return potionTypes; }

    Registry<Ptr<Enemy>> const & getEnemyTypes() const { return enemyTypes; }
    Registry<Ptr<Enemy>>       & getEnemyTypes()       { return enemyTypes; }

    Ptr<Item> createItem(std::string const & id);

    bool isPotionKnown(std::string const & id) const { return potionTypeKnown.at(id); }
    void markPotionAsKnown(std::string const & id) { potionTypeKnown.at(id) = true; }

    void addMessage(std::string_view msg);
    void drop(Ptr<Item> item, Coord2i to);

private:
    void printMenu(std::vector<std::string_view> const & items, int activeItem);
    tl::optional<std::string> processMenu(std::string_view title,
            std::vector<std::string_view> const & items, bool canExit = false);
    void mSettingsMode();
    void mSettingsMap();
    void mSettings();
    void mainMenu();

    std::vector<std::string> readTips() const;

    void readItemRenderData(std::string const & id, YAMLFileCache & cache);
    void readItemRenderData(YAMLFileCache & cache);
    void readUnitRenderData(std::string const & id, YAML::Node const & renderData);
    void readEnemyRenderData(std::string const & id, YAMLFileCache & cache);
    void readHeroRenderData(YAMLFileCache & cache);
    void readUnitRenderData(YAMLFileCache & cache);
    SymbolRenderData getRenderData(Item const & item);
    SymbolRenderData getRenderData(Unit const & unit);
    tl::optional<CellRenderData> getRenderData(Coord2i cell);
    void clearCachedMap();
    void drawMap();
    void clearBuffers();
    void displayMessages();
    void draw();

    void updateAI();

    void loadData();

    void setItems();
    void spawnUnits();
    void setRandomPotionEffects();

    void initialize();
    void initField();
    void readMap();

    ItemPile::iterator findItemAt(Coord2i cell, std::string_view id);
    bool randomlySetOnMap(Ptr<Item> item);

    template<class ItemType, class Fn = decltype(&detail::cloneAny<ItemType>), meta::Check<detail::IsItemSelector<Fn, ItemType>> = meta::Checked>
    void randomlySelectAndSetOnMap(Registry<Ptr<ItemType>> const & types, int n, Fn const & selector = &detail::cloneAny<ItemType>) {
        for (int i = 0; i < n; ++i) {
            auto selected = selector(types);
            randomlySetOnMap(std::move(selected));
        }
    }

    TerminalRenderer termRend;
    TerminalReader termRead;

    Registry<SymbolRenderData> itemRenderData;
    Registry<SymbolRenderData> unitRenderData;
    Array2D<tl::optional<CellRenderData>, LEVEL_ROWS, LEVEL_COLS> cachedMap;

    Array2D<int, LEVEL_ROWS, LEVEL_COLS> levelData;
    Array2D<ItemPile, LEVEL_ROWS, LEVEL_COLS> itemsMap;
    Array2D<Ptr<Unit>, LEVEL_ROWS, LEVEL_COLS> unitsMap;

    Registry<Ptr<Food>> foodTypes;
    Registry<Ptr<Armor>> armorTypes;
    Registry<Ptr<Weapon>> weaponTypes;
    Registry<Ptr<Ammo>> ammoTypes;
    Registry<Ptr<Scroll>> scrollTypes;
    Registry<Ptr<Potion>> potionTypes;
    Registry<bool> potionTypeKnown;

    Registry<Ptr<Enemy>> enemyTypes;

    std::string message;
    std::string bar;
    std::string weaponBar;

    Hero * hero;

    Ptr<Hero> heroTemplate;

    int mode = 1;
    int turns = 0;
    bool exit = false;
    bool stop = false;
    bool generateMap = true;
};

extern Game g_game;

#endif //RLRPG_GAME_HPP

