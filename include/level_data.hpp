#ifndef RLRPG_LEVEL_DATA_HPP
#define RLRPG_LEVEL_DATA_HPP

#include<array2d.hpp>
#include<render_data.hpp>
#include<level.hpp>
#include<registry.hpp>
#include<meta/check.hpp>
#include<ptr.hpp>

#include<tl/optional.hpp>

#include<string>
#include<string_view>
#include<vector>
#include<unordered_map>
#include<list>
#include<memory>

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

class Game {
public:
    void setHeroTemplate(Ptr<Hero> newHeroTemplate);

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

private:
    void readItemRenderData(std::string const & id, YAMLFileCache & cache);
    void readItemRenderData(YAMLFileCache & cache);
    void readUnitRenderData(std::string const & id, YAML::Node const & renderData);
    void readEnemyRenderData(std::string const & id, YAMLFileCache & cache);
    void readHeroRenderData(YAMLFileCache & cache);
    void readUnitRenderData(YAMLFileCache & cache);

    void loadData();

    Registry<Ptr<Food>> foodTypes;
    Registry<Ptr<Armor>> armorTypes;
    Registry<Ptr<Weapon>> weaponTypes;
    Registry<Ptr<Ammo>> ammoTypes;
    Registry<Ptr<Scroll>> scrollTypes;
    Registry<Ptr<Potion>> potionTypes;
    Registry<bool> potionTypeKnown;

    Registry<Ptr<Enemy>> enemyTypes;

    Ptr<Hero> heroTemplate;
};

#endif //RLRPG_LEVEL_DATA_HPP

