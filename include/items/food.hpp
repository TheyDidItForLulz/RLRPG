#ifndef RLRPG_ITEMS_FOOD_HPP
#define RLRPG_ITEMS_FOOD_HPP

#include<items/item.hpp>
#include<ptr.hpp>
#include<enable_clone.hpp>

class Food
    : public Item
    , public EnableClone<Food>
{
public:
    static int const COUNT = 10;

    int nutritionalValue;
    bool isRotten = false;

    Type getType() const override {
        return Type::Food;
    }

    Ptr<Item> cloneItem() const override {
        return std::make_unique<Food>(*this);
    }
};

#endif // RLRPG_ITEMS_FOOD_HPP
