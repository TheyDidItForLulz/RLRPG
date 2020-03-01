#ifndef RLRPG_GAME_MAP_HPP
#define RLRPG_GAME_MAP_HPP

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

class GameMap {
public:
    LevelData const & level() const { return levelData; }
    LevelData       & level()       { return levelData; }

    auto const & getItemsMap() const { return itemsMap; }
    auto       & getItemsMap()       { return itemsMap; }

    auto const & getUnitsMap() const { return unitsMap; }
    auto       & getUnitsMap()       { return unitsMap; }

    void drop(Ptr<Item> item, Coord2i to);

private:
    void readItemRenderData(std::string const & id, YAMLFileCache & cache);
    void readItemRenderData(YAMLFileCache & cache);
    void readUnitRenderData(std::string const & id, YAML::Node const & renderData);
    void readEnemyRenderData(std::string const & id, YAMLFileCache & cache);
    void readHeroRenderData(YAMLFileCache & cache);
    void readUnitRenderData(YAMLFileCache & cache);

    void loadData();

    void setItems();
    void spawnUnits();
    void setRandomPotionEffects();

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

    Array2D<int, LEVEL_ROWS, LEVEL_COLS> levelData;
    Array2D<ItemPile, LEVEL_ROWS, LEVEL_COLS> itemsMap;
    Array2D<Ptr<Unit>, LEVEL_ROWS, LEVEL_COLS> unitsMap;
};

#endif //RLRPG_GAME_MAP_HPP

