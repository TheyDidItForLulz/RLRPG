//
// Created by yuri12358 on 6/26/19.
//

#include<yaml_item_loader.hpp>

#include<item.hpp>
#include<yaml_file_cache.hpp>
#include<game.hpp>

#include<yaml-cpp/yaml.h>

#include<fmt/format.h>

#include<tl/optional.hpp>

void YAMLItemLoader::load() {
    YAML::Node registry = yamlFileCache["data/items.yaml"];

    g_game.getFoodTypes().clear();
    g_game.getArmorTypes().clear();
    g_game.getWeaponTypes().clear();
    g_game.getAmmoTypes().clear();
    g_game.getScrollTypes().clear();
    g_game.getPotionTypes().clear();

    for (auto const & id : registry["food"]) {
        auto idstr = id.as<std::string>();
        g_game.getFoodTypes()[idstr] = loadFood(idstr);
    }

    for (auto const & id : registry["armor"]) {
        auto idstr = id.as<std::string>();
        g_game.getArmorTypes()[idstr] = loadArmor(idstr);
    }

    for (auto const & id : registry["weapon"]) {
        auto idstr = id.as<std::string>();
        g_game.getWeaponTypes()[idstr] = loadWeapon(idstr);
    }

    for (auto const & id : registry["ammo"]) {
        auto idstr = id.as<std::string>();
        g_game.getAmmoTypes()[idstr] = loadAmmo(idstr);
    }

    for (auto const & id : registry["scroll"]) {
        auto idstr = id.as<std::string>();
        g_game.getScrollTypes()[idstr] = loadScroll(idstr);
    }

    for (auto const & id : registry["potion"]) {
        auto idstr = id.as<std::string>();
        g_game.getPotionTypes()[idstr] = loadPotion(idstr);
    }
}

void initItemBase(Item & item, YAML::Node const & data) {
    item.weight = data["weight"].as<int>();
    item.id = data["id"].as<std::string>();
    item.isStackable = data["isStackable"].as<bool>();
    item.name = data["name"].as<std::string>();
}

YAML::Node loadItemData(std::string_view id, YAMLFileCache & yamlFileCache) {
    auto filename = fmt::format("data/items/{}.yaml", id);
    return yamlFileCache[filename];
}

Ptr<Food> YAMLItemLoader::loadFood(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    auto loaded = std::make_unique<Food>();
    initItemBase(*loaded, item);
    loaded->nutritionalValue = item["food"]["nutritionalValue"].as<int>();
    return loaded;
}

Ptr<Armor> YAMLItemLoader::loadArmor(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    auto loaded = std::make_unique<Armor>();
    initItemBase(*loaded, item);
    loaded->durability = item["armor"]["durability"].as<int>();
    loaded->defence = item["armor"]["defence"].as<int>();
    return loaded;
}

Ptr<Ammo> YAMLItemLoader::loadAmmo(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    auto loaded = std::make_unique<Ammo>();
    initItemBase(*loaded, item);
    loaded->damage = item["ammo"]["damage"].as<int>();
    loaded->range = item["ammo"]["range"].as<int>();
    return loaded;
}

Ptr<Weapon> YAMLItemLoader::loadWeapon(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    auto loaded = std::make_unique<Weapon>();
    initItemBase(*loaded, item);
    loaded->damage = item["weapon"]["damage"].as<int>();
    if (item["weapon"]["ranged"]) {
        loaded->isRanged = true;
        loaded->range = item["weapon"]["ranged"]["range"].as<int>();
        loaded->damageBonus = item["weapon"]["ranged"]["damageBonus"].as<int>();
        int cartridgeSize = item["weapon"]["ranged"]["cartridgeSize"].as<int>();
        loaded->cartridge = Weapon::Cartridge(cartridgeSize);
        if (item["weapon"]["canDig"]) {
            loaded->canDig = item["weapon"]["canDig"].as<bool>();
        }
    }
    return loaded;
}

tl::optional<Scroll::Effect> toScrollEffect(std::string_view effectString) {
    if (effectString == "map") {
        return Scroll::Map;
    } else if (effectString == "identify") {
        return Scroll::Identify;
    } else {
        return tl::nullopt;
    }
}

Ptr<Scroll> YAMLItemLoader::loadScroll(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    auto loaded = std::make_unique<Scroll>();
    initItemBase(*loaded, item);
    auto optEffect = toScrollEffect(item["scroll"]["effect"].as<std::string>());
    if (not optEffect)
        throw std::logic_error(fmt::format("Failed to parse a scroll effect of '{}'", id));
    loaded->effect = *optEffect;
    return loaded;
}

Ptr<Potion> YAMLItemLoader::loadPotion(std::string_view id) {
    YAML::Node item = loadItemData(id, yamlFileCache);
    auto loaded = std::make_unique<Potion>();
    initItemBase(*loaded, item);
    return loaded;
}

