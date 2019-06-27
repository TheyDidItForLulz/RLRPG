//
// Created by yuri12358 on 6/26/19.
//

#include<yaml_item_loader.hpp>
#include<item.hpp>
#include<yaml_file_cache.hpp>

#include<yaml-cpp/yaml.h>
#include <fmt/format.h>

void YAMLItemLoader::load() {
    YAML::Node registry = yamlFileCache["data/items.yaml"];

    foodTypes.clear();
    armorTypes.clear();
    weaponTypes.clear();
    ammoTypes.clear();
    scrollTypes.clear();
    potionTypes.clear();

    for (const auto & id : registry["food"]) {
        auto idstr = id.as<std::string>();
        foodTypes[idstr] = loadFood(idstr);
    }

    for (const auto & id : registry["armor"]) {
        auto idstr = id.as<std::string>();
        armorTypes[idstr] = loadArmor(idstr);
    }

    for (const auto & id : registry["weapon"]) {
        auto idstr = id.as<std::string>();
        weaponTypes[idstr] = loadWeapon(idstr);
    }

    for (const auto & id : registry["ammo"]) {
        auto idstr = id.as<std::string>();
        ammoTypes[idstr] = loadAmmo(idstr);
    }

    for (const auto & id : registry["scroll"]) {
        auto idstr = id.as<std::string>();
        scrollTypes[idstr] = loadScroll(idstr);
    }

    for (const auto & id : registry["potion"]) {
        auto idstr = id.as<std::string>();
        potionTypes[idstr] = loadPotion(idstr);
    }
}

void initItemBase(Item & item, const YAML::Node & data) {
    item.weight = data["weight"].as<int>();
    item.id = data["id"].as<std::string>();
    item.isStackable = data["isStackable"].as<bool>();
    item.name = data["name"].as<std::string>();
}

YAML::Node loadItemData(std::string_view id, YAMLFileCache & yamlFileCache) {
    auto filename = fmt::format("data/items/{}.yaml", id);
    return yamlFileCache[filename];
}

Food YAMLItemLoader::loadFood(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    Food loaded;
    initItemBase(loaded, item);
    loaded.nutritionalValue = item["food"]["nutritionalValue"].as<int>();
    return loaded;
}

Armor YAMLItemLoader::loadArmor(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    Armor loaded;
    initItemBase(loaded, item);
    loaded.durability = item["armor"]["durability"].as<int>();
    loaded.defence = item["armor"]["defence"].as<int>();
    return loaded;
}

Ammo YAMLItemLoader::loadAmmo(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    Ammo loaded;
    initItemBase(loaded, item);
    loaded.damage = item["ammo"]["damage"].as<int>();
    loaded.range = item["ammo"]["range"].as<int>();
    return loaded;
}

Weapon YAMLItemLoader::loadWeapon(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    Weapon loaded;
    initItemBase(loaded, item);
    loaded.damage = item["weapon"]["damage"].as<int>();
    if (item["weapon"]["ranged"]) {
        loaded.isRanged = true;
        loaded.range = item["weapon"]["ranged"]["range"].as<int>();
        loaded.damageBonus = item["weapon"]["ranged"]["damageBonus"].as<int>();
        int cartridgeSize = item["weapon"]["ranged"]["cartridgeSize"].as<int>();
        loaded.cartridge = Weapon::Cartridge(cartridgeSize);
        if (item["weapon"]["canDig"]) {
            loaded.canDig = item["weapon"]["canDig"].as<bool>();
        }
    }
    return loaded;
}

Scroll YAMLItemLoader::loadScroll(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    Scroll loaded;
    initItemBase(loaded, item);
    loaded.effect = item["scroll"]["effect"].as<int>();
    return loaded;
}

Potion YAMLItemLoader::loadPotion(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    Potion loaded;
    initItemBase(loaded, item);
    return loaded;
}
