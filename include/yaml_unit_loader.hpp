//
// Created by yuri12358 on 6/28/19.
//

#ifndef RLRPG_YAML_UNIT_LOADER_HPP
#define RLRPG_YAML_UNIT_LOADER_HPP

#include<abstract_unit_loader.hpp>
#include<units/unit.hpp>
#include<units/hero.hpp>
#include<units/enemy.hpp>

#include<string_view>

class YAMLFileCache;

class YAMLUnitLoader : public AbstractUnitLoader {
    YAMLFileCache & yamlFileCache;

    Hero loadHero();
    Enemy loadEnemy(const std::string & id);

public:
    explicit YAMLUnitLoader(YAMLFileCache & cache): yamlFileCache(cache) {}

    void load() override;
};
#endif //RLRPG_YAML_UNIT_LOADER_HPP
