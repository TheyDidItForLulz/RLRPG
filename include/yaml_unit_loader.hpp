//
// Created by yuri12358 on 6/28/19.
//

#ifndef RLRPG_YAML_UNIT_LOADER_HPP
#define RLRPG_YAML_UNIT_LOADER_HPP

#include<abstract_unit_loader.hpp>
#include<ptr.hpp>

#include<string_view>

class YAMLFileCache;
class Hero;
class Enemy;

class YAMLUnitLoader : public AbstractUnitLoader {
    YAMLFileCache & yamlFileCache;

    Ptr<Hero> loadHero();
    Ptr<Enemy> loadEnemy(std::string const & id);

public:
    explicit YAMLUnitLoader(YAMLFileCache & cache): yamlFileCache(cache) {}

    void load() override;
};

#endif //RLRPG_YAML_UNIT_LOADER_HPP

