//
// Created by yuri12358 on 6/26/19.
//

#ifndef RLRPG_YAMLFILECACHE_HPP
#define RLRPG_YAMLFILECACHE_HPP

#include<yaml-cpp/yaml.h>
#include<unordered_map>
#include<string>

class YAMLFileCache {
    std::unordered_map<std::string, YAML::Node> cache;

public:
    YAML::Node & operator[](const std::string & filename);

    bool contains(const std::string & filename) const;

    void load(const std::string & filename);
};

#endif //RLRPG_YAMLFILECACHE_HPP
