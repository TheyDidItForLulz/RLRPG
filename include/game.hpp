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

using ItemPtr = Ptr<Item>;
using FoodPtr = Ptr<Food>;
using ArmorPtr = Ptr<Armor>;
using WeaponPtr = Ptr<Weapon>;
using AmmoPtr = Ptr<Ammo>;
using ScrollPtr = Ptr<Scroll>;
using PotionPtr = Ptr<Potion>;
using ItemPile = std::list<ItemPtr>;

namespace detail {
    template<class ItemType,
        meta::Check<std::is_base_of_v<Item, ItemType> and IsClonable<ItemType>> = meta::Checked>
    Ptr<ItemType> cloneAnyItem(const Registry<Ptr<ItemType>> & reg) {
        return effolkronium::random_static::get(reg)->second->clone();
    }

    template<class Fn, class ItemType>
    constexpr bool IsItemSelector = std::is_invocable_r_v<Ptr<ItemType>, Fn, const Registry<Ptr<ItemType>> &>;
}

class Game {
public:
    void run();

    const LevelData & level() const { return levelData; }
    LevelData & level() { return levelData; }

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

    const auto & getItemsMap() const { return itemsMap; }
    auto & getItemsMap() { return itemsMap; }

    const Registry<FoodPtr> & getFoodTypes() const { return foodTypes; }
    Registry<FoodPtr> & getFoodTypes() { return foodTypes; }
    const Registry<ArmorPtr> & getArmorTypes() const { return armorTypes; }
    Registry<ArmorPtr> & getArmorTypes() { return armorTypes; }
    const Registry<WeaponPtr> & getWeaponTypes() const { return weaponTypes; }
    Registry<WeaponPtr> & getWeaponTypes() { return weaponTypes; }
    const Registry<AmmoPtr> & getAmmoTypes() const { return ammoTypes; }
    Registry<AmmoPtr> & getAmmoTypes() { return ammoTypes; }
    const Registry<ScrollPtr> & getScrollTypes() const { return scrollTypes; }
    Registry<ScrollPtr> & getScrollTypes() { return scrollTypes; }
    const Registry<PotionPtr> & getPotionTypes() const { return potionTypes; }
    Registry<PotionPtr> & getPotionTypes() { return potionTypes; }

    bool isPotionKnown(const std::string & id) const { return potionTypeKnown.at(id); }
    void markPotionAsKnown(const std::string & id) { potionTypeKnown.at(id) = true; }

    void addMessage(std::string_view msg);

    void initialize();

    void updateAI();
    
    void setItems();

    void spawnUnits();

    void clearBuffers();

    void displayMessages();

    void drop(ItemPtr item, Coord2i to);

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
    SymbolRenderData getRenderData(const Item & item);
    SymbolRenderData getRenderData(const Unit & unit);
    tl::optional<CellRenderData> getRenderData(Coord2i cell);
    void clearCachedMap();
    void drawMap();
    void draw();

    void loadData();

    void setRandomPotionEffects();

    void initField();
    void readMap();

    ItemPile::iterator findItemAt(Coord2i cell, std::string_view id);
    bool randomlySetOnMap(ItemPtr item);

    template<class ItemType, class Fn = decltype(&detail::cloneAnyItem<ItemType>), meta::Check<detail::IsItemSelector<Fn, ItemType>> = meta::Checked>
    void randomlySelectAndSetOnMap(const Registry<Ptr<ItemType>> & types, int n, const Fn & selector = &detail::cloneAnyItem<ItemType>) {
        for (int i = 0; i < n; ++i) {
            auto selected = selector(types);
            randomlySetOnMap(std::move(selected));
        }
    }

    TerminalRenderer termRend;
    TerminalReader termRead;

    std::unordered_map<std::string, SymbolRenderData> itemRenderData;
    std::unordered_map<std::string, SymbolRenderData> unitRenderData;
    Array2D<tl::optional<CellRenderData>, LEVEL_ROWS, LEVEL_COLS> cachedMap;

    Array2D<int, LEVEL_ROWS, LEVEL_COLS> levelData;
    Array2D<ItemPile, LEVEL_ROWS, LEVEL_COLS> itemsMap;

    Registry<FoodPtr> foodTypes;
    Registry<ArmorPtr> armorTypes;
    Registry<WeaponPtr> weaponTypes;
    Registry<AmmoPtr> ammoTypes;
    Registry<ScrollPtr> scrollTypes;
    Registry<PotionPtr> potionTypes;
    Registry<bool> potionTypeKnown;

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

