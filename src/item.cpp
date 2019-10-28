#include<items/item.hpp>
#include<items/food.hpp>
#include<items/armor.hpp>
#include<items/ammo.hpp>
#include<items/weapon.hpp>
#include<items/scroll.hpp>
#include<items/potion.hpp>

#include<game.hpp>

#include<stdexcept>

std::string Item::getMdf() const {
	switch (mdf) {
		case 1: return "nothing";
		case 2: return "thorns";
		default:break;
	}
    throw std::logic_error("Unknown modifier id");
}

std::string Item::getName() const {
	return name;
}

Ptr<Item> Item::splitStack(int toSplit) {
	if (toSplit <= 0 or toSplit > count) {
		throw std::logic_error("Trying to split an item pile invalidly");
	}
	auto otherPile = cloneItem();
	otherPile->count = toSplit;
	count -= toSplit;
	return otherPile;
}

int Item::getSingleWeight() const {
    return weight;
}

int Item::getTotalWeight() const {
    return weight * count;
}

Ptr<Item> Item::getByID(std::string const & id) {
	{
		auto it = g_game.getFoodTypes().find(id);
		if (it != g_game.getFoodTypes().end())
			return it->second->clone();
	}
	{
		auto it = g_game.getArmorTypes().find(id);
		if (it != g_game.getArmorTypes().end())
			return it->second->clone();
	}
	{
		auto it = g_game.getAmmoTypes().find(id);
		if (it != g_game.getAmmoTypes().end())
			return it->second->clone();
	}
	{
		auto it = g_game.getPotionTypes().find(id);
		if (it != g_game.getPotionTypes().end())
			return it->second->clone();
	}
	{
		auto it = g_game.getScrollTypes().find(id);
		if (it != g_game.getScrollTypes().end())
			return it->second->clone();
	}
	{
		auto it = g_game.getWeaponTypes().find(id);
		if (it != g_game.getWeaponTypes().end())
			return it->second->clone();
	}
	return {};
}

