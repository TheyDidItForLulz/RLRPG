//
// Created by yuri12358 on 6/26/19.
//

#ifndef RLRPG_YAMLITEMLOADER_HPP
#define RLRPG_YAMLITEMLOADER_HPP

#include<abstract_item_loader.hpp>
#include<item.hpp>

#include<string_view>

class YAMLFileCache;

class YAMLItemLoader : public AbstractItemLoader {
    Food loadFood(std::string_view id);
    Armor loadArmor(std::string_view id);
    Weapon loadWeapon(std::string_view id);
    Ammo loadAmmo(std::string_view id);
    Scroll loadScroll(std::string_view id);
    Potion loadPotion(std::string_view id);

    YAMLFileCache & yamlFileCache;

public:
    explicit YAMLItemLoader(YAMLFileCache & cache): yamlFileCache(cache) {}

    void load() override;
};

#endif //RLRPG_YAMLITEMLOADER_HPP
