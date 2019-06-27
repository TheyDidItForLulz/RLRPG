//
// Created by yuri12358 on 6/26/19.
//

#include<yaml_file_cache.hpp>

bool YAMLFileCache::contains(const std::string & filename) const {
    return cache.count(std::string(filename)) == 1;
}

YAML::Node & YAMLFileCache::operator[](const std::string & filename) {
    if (not contains(filename)) {
        load(filename);
    }
    return cache[filename];
}

void YAMLFileCache::load(const std::string& filename) {
    cache[filename] = YAML::LoadFile(filename);
}
