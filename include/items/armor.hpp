#ifndef RLRPG_ITEMS_ARMOR_HPP
#define RLRPG_ITEMS_ARMOR_HPP

#include<items/item.hpp>
#include<ptr.hpp>
#include<enable_clone.hpp>

class Armor
    : public Item
    , public EnableClone<Armor>
{
public:
    static int const COUNT = 4;

    int defence;
    int durability;

    Type getType() const override {
        return Type::Armor;
    }

    Ptr<Item> cloneItem() const override {
        return std::make_unique<Armor>(*this);
    }
};

#endif // RLRPG_ITEMS_ARMOR_HPP

