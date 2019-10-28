#ifndef RLRPG_ITEMS_POTION_HPP
#define RLRPG_ITEMS_POTION_HPP

#include<items/item.hpp>
#include<ptr.hpp>
#include<enable_clone.hpp>

class Potion
    : public Item
    , public EnableClone<Potion>
{
public:
    static int const COUNT = 25; /* IT TOO */

    enum Effect {
    	None,
    	Heal,
    	Invisibility,
    	Teleport,
    	Blindness,
    	EffectCount
    };

	Effect effect;

    std::string getName() const override;

    Type getType() const override {
        return Type::Potion;
    }

    Ptr<Item> cloneItem() const override {
        return std::make_unique<Potion>(*this);
    }
};

#endif // RLRPG_ITEMS_POTION_HPP

