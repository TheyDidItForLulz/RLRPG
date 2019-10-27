//
// Created by yuri12358 on 6/26/19.
//

#include<yaml_file_cache.hpp>

bool YAMLFileCache::contains(std::string const & filename) const {
    return cache.count(std::string(filename)) == 1;
}

YAML::Node & YAMLFileCache::operator[](std::string const & filename) {
    if (not contains(filename)) {
        load(filename);
    }
    return cache[filename];
}

void YAMLFileCache::load(std::string const & filename) {
    cache[filename] = YAML::LoadFile(filename);
}
