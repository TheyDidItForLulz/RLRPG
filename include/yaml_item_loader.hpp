//
// Created by yuri12358 on 6/26/19.
//

#ifndef RLRPG_YAMLITEMLOADER_HPP
#define RLRPG_YAMLITEMLOADER_HPP

#include<abstract_item_loader.hpp>

#include<string_view>
#include<memory>

class YAMLFileCache;
class Food;
class Armor;
class Ammo;
class Weapon;
class Scroll;
class Potion;

using FoodPtr = std::unique_ptr<Food>;
using ArmorPtr = std::unique_ptr<Armor>;
using AmmoPtr = std::unique_ptr<Ammo>;
using WeaponPtr = std::unique_ptr<Weapon>;
using ScrollPtr = std::unique_ptr<Scroll>;
using PotionPtr = std::unique_ptr<Potion>;

class YAMLItemLoader : public AbstractItemLoader {
    FoodPtr loadFood(std::string_view id);
    ArmorPtr loadArmor(std::string_view id);
    WeaponPtr loadWeapon(std::string_view id);
    AmmoPtr loadAmmo(std::string_view id);
    ScrollPtr loadScroll(std::string_view id);
    PotionPtr loadPotion(std::string_view id);

    YAMLFileCache & yamlFileCache;

public:
    explicit YAMLItemLoader(YAMLFileCache & cache): yamlFileCache(cache) {}

    void load() override;
};

#endif //RLRPG_YAMLITEMLOADER_HPP
