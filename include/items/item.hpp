#ifndef RLRPG_ITEMS_ITEM_HPP
#define RLRPG_ITEMS_ITEM_HPP

#include<ptr.hpp>

#include<termlib/vec2.hpp>

#include<string>

class Item {
public:
    enum class Type {
        Food,
        Armor,
        Weapon,
        Ammo,
        Scroll,
        Potion
    };

    virtual ~Item() = default;

    Coord2i pos;
    std::string id;
    std::string name;
    char inventorySymbol;
    int weight;
    int mdf = 1;
    int count = 1;
    bool showMdf = false;
    bool isStackable;

    // toSplit:
    //  - [1, count) - splits on 2 piles, returns one with count = toSplit
    //  - count - returns whole pile, this pile becomes invalid
    Ptr<Item> splitStack(int toSplit);
    std::string getMdf() const;
    virtual std::string getName() const;
    int getSingleWeight() const;
    int getTotalWeight() const;
    virtual Type getType() const = 0;
    virtual Ptr<Item> cloneItem() const = 0;
};

#endif // RLRPG_ITEMS_ITEM_HPP

