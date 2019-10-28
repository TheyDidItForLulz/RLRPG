#ifndef RLRPG_ITEMS_SCROLL_HPP
#define RLRPG_ITEMS_SCROLL_HPP

#include<items/item.hpp>
#include<ptr.hpp>
#include<enable_clone.hpp>

class Scroll
    : public Item
    , public EnableClone<Scroll>
{
public:
    static int const COUNT = 15; /* JUST FOR !DEBUG!!*/

    enum Effect {
    	Map,
    	Identify,
    	EffectCount
    };

	Effect effect;

    Type getType() const override {
        return Type::Scroll;
    }

    Ptr<Item> cloneItem() const override {
        return std::make_unique<Scroll>(*this);
    }
};

#endif // RLRPG_ITEMS_SCROLL_HPP

