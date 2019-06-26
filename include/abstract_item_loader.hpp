//
// Created by yuri12358 on 6/26/19.
//

#ifndef RLRPG_ABSTRACT_ITEM_LOADER_HPP
#define RLRPG_ABSTRACT_ITEM_LOADER_HPP

//////////////////////////////////////////////////
// Loads foodTypes, armorTypes and others
class AbstractItemLoader {
public:
    virtual void load() = 0;
};

#endif //RLRPG_ABSTRACT_ITEM_LOADER_HPP
