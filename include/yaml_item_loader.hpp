//
// Created by yuri12358 on 6/26/19.
//

#ifndef RLRPG_YAMLITEMLOADER_HPP
#define RLRPG_YAMLITEMLOADER_HPP

#include<abstract_item_loader.hpp>
#include<ptr.hpp>

#include<string_view>
#include<memory>

class YAMLFileCache;
class Food;
class Armor;
class Ammo;
class Weapon;
class Scroll;
class Potion;

class YAMLItemLoader : public AbstractItemLoader {
    Ptr<Food> loadFood(std::string_view id);
    Ptr<Armor> loadArmor(std::string_view id);
    Ptr<Weapon> loadWeapon(std::string_view id);
    Ptr<Ammo> loadAmmo(std::string_view id);
    Ptr<Scroll> loadScroll(std::string_view id);
    Ptr<Potion> loadPotion(std::string_view id);

    YAMLFileCache & yamlFileCache;

public:
    explicit YAMLItemLoader(YAMLFileCache & cache): yamlFileCache(cache) {}

    void load() override;
};

#endif //RLRPG_YAMLITEMLOADER_HPP
