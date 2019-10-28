#include<items/item.hpp>

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

