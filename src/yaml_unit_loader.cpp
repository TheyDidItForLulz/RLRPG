//
// Created by yuri12358 on 6/28/19.
//

#include<yaml_unit_loader.hpp>

#include<yaml_file_cache.hpp>
#include<item.hpp>
#include<units/hero.hpp>
#include<units/enemy.hpp>
#include<game.hpp>

#include<fmt/format.h>
#include<tl/optional.hpp>

void YAMLUnitLoader::load() {
    g_game.setHeroTemplate(loadHero());

    const auto & enemyRegistry = yamlFileCache["data/units/enemies.yaml"];
    for (const auto & id : enemyRegistry) {
        auto idString = id.as<std::string>();
        g_game.getEnemyTypes()[idString] = loadEnemy(idString);
    }
}

tl::optional<std::pair<int, int>> parseRange(const std::string & toParse) {
    auto delimIndex = toParse.find("..");
    if (delimIndex == std::string::npos) {
        try {
            int asInt = std::stoi(toParse);
            return std::make_pair(asInt, asInt);
        } catch (...) {
            return tl::nullopt;
        }
    }
    try {
        return std::make_pair(
                std::stoi(toParse.substr(0, delimIndex)),
                std::stoi(toParse.substr(delimIndex + 2)));
    } catch (...) {
        return tl::nullopt;
    }
}

ItemPtr createItem(const YAML::Node & itemData){
    auto item = Item::getByID(itemData["id"].as<std::string>());
    if (not item)
        return nullptr;

    if (itemData["count"]) {
        auto optRange = parseRange(itemData["count"].as<std::string>());
        if (not item->isStackable or not optRange)
            return nullptr;

        item->count = Random::get(optRange->first, optRange->second);
    }

    return item;
}

void loadUnitInventory(Unit & unit, const YAML::Node & invData) {
    for (const auto & entry : invData) {
        char at = entry.first.as<char>();
        auto item = createItem(entry.second);
        unit.inventory.add(std::move(item), at);
    }
}

void initUnitBase(Unit & unit, const YAML::Node & data) {
    unit.id = data["id"].as<std::string>();
    unit.name = data["name"].as<std::string>();
    unit.health = data["health"].as<int>();
    unit.maxHealth = data["maxHealth"].as<int>();
    unit.vision = data["visionDistance"].as<int>();
    if (data["inventory"])
        loadUnitInventory(unit, data["inventory"]);
    if (data["armor"]) {
        char at = data["armor"].as<char>();
        unit.armor = dynamic_cast<Armor *>(&unit.inventory[at]);
    }
    if (data["weapon"]) {
        char at = data["weapon"].as<char>();
        unit.weapon = dynamic_cast<Weapon *>(&unit.inventory[at]);
    }
}

Ptr<Hero> YAMLUnitLoader::loadHero() {
    auto heroData = yamlFileCache["data/units/hero.yaml"];
    auto hero = std::make_unique<Hero>();
    initUnitBase(*hero, heroData);
    hero->maxBurden = heroData["hero"]["maxBurden"].as<int>();
    hero->hunger = heroData["hero"]["hunger"].as<int>();
    return hero;
}

Ptr<Enemy> YAMLUnitLoader::loadEnemy(const std::string & id) {
    auto filename = fmt::format("data/units/enemies/{}.yaml", id);
    auto enemyData = yamlFileCache[filename];
    auto enemy = std::make_unique<Enemy>();
    initUnitBase(*enemy, enemyData);
    enemy->xpCost = enemyData["enemy"]["xpCost"].as<int>();
    if (enemyData["enemy"]["ammo"]) {
        char at = enemyData["enemy"]["ammo"].as<char>();
        enemy->ammo = dynamic_cast<Ammo *>(&enemy->inventory[at]);
    }
    return enemy;
}

