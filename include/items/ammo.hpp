#ifndef RLRPG_ITEMS_AMMO_HPP
#define RLRPG_ITEMS_AMMO_HPP

#include<items/item.hpp>
#include<ptr.hpp>
#include<enable_clone.hpp>

class Ammo
    : public Item
    , public EnableClone<Ammo>
{
public:
    static int const COUNT = 25;

	int range;
	int damage;

    Type getType() const override {
        return Type::Ammo;
    }

    Ptr<Item> cloneItem() const override {
        return std::make_unique<Ammo>(*this);
    }
};

#endif // RLRPG_ITEMS_AMMO_HPP

