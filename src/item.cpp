#include"item.hpp"
#include<stdexcept>
#include<algorithm>
#include <item.hpp>


////////////////////////////////
// Item
Item::Item(): mdf(1), showMdf(false), count(1){};

std::string Item::getMdf() const {
	switch (mdf) {
		case 1: return "nothing";
		case 2: return "thorns";
		default:break;
	}
    throw std::logic_error("Unknown modifier id");
}

std::string Item::getName() const {
    if (id == "egg") {
		return "an egg";
	} else if (id == "apple") {
		return "an apple";
	} else if (id == "chain_chestplate") {
		return "a chain chestplate";
	} else if (id == "leather_chestplate") {
		return "a leather chestplate";
	} else if (id == "copper_shortsword") {
		return "a copper shortsword";
	} else if (id == "bronze_spear") {
		return "a bronze spear";
	} else if (id == "musket") {
		return "a musket";
	} else if (id == "stick") {
	    return "a stick";
	} else if (id == "shotgun") {
	    return "a shotgun";
	} else if (id == "pistol") {
	    return "a pistol";
    } else if (id == "pickaxe") {
        return "a pickaxe";
	} else if (id == "steel_bullets") {
	    return "steel bullets";
	} else if (id == "shotgun_bullets") {
	    return "shotgun shells";
	} else if (id == "map") {
	    return "a map";
	} else if (id == "identify_scroll") {
	    return "an identify scroll";
	} else if (
			id == "blue_potion" or
			id == "green_potion" or
			id == "yellow_potion" or
			id == "magenta_potion" or
			id == "dark_potion"
			) {
		return getPotionName(id);
	} else {
		throw std::logic_error("Unknown item id");
	}
}
Item::~Item() = default;

Item::Ptr Item::splitStack(int toSplit) {
	if (toSplit <= 0 or toSplit > count) {
		throw std::logic_error("Trying to split an item pile invalidly");
	}
	Item::Ptr otherPile = clone();
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

////////////////////////////////
// Food
Food::Food(): isRotten(false){};
Food::~Food() = default;;

////////////////////////////////
// Armor
Armor::Armor() = default;
Armor::~Armor() = default;

////////////////////////////////
// Ammo
Ammo::Ammo() = default;

Ammo::~Ammo() = default;

////////////////////////////////
// Weapon
Weapon::Weapon() = default;

Weapon::~Weapon() = default;

Weapon::Cartridge::Cartridge(int capacity): capacity(capacity) {
	assert(capacity >= 0);
}

Weapon::Cartridge::Cartridge(const Cartridge & other): capacity(other.capacity) {
	for (const auto & bullet : other) {
		loaded.push_back(std::make_unique<Ammo>(*bullet));
	}
}

Weapon::Cartridge & Weapon::Cartridge::operator =(const Cartridge & other) {
	capacity = other.capacity;
	loaded.clear();
	for (const auto & bullet : other) {
		loaded.push_back(std::make_unique<Ammo>(*bullet));
	}
	return *this;
}

Ammo::Ptr Weapon::Cartridge::load(Ammo::Ptr bullet) {
	if (loaded.size() == capacity) {
		return bullet;
	}
	loaded.push_back(std::move(bullet));
	return nullptr;
}

Ammo::Ptr Weapon::Cartridge::unloadOne() {
	if (loaded.empty()) {
		return nullptr;
	}
	auto bullet = std::move(loaded.back());
	loaded.pop_back();
	return bullet;
}

Ammo & Weapon::Cartridge::next() {
	assert(not isEmpty());
	return *loaded.back();
}

const Ammo & Weapon::Cartridge::next() const {
	assert(not isEmpty());
	return *loaded.back();
}

const Ammo * Weapon::Cartridge::operator [](int ind) const {
	assert(ind >= 0 and ind < capacity);
	if (ind < loaded.size()) {
		return loaded[ind].get();
	}
	return nullptr;
}

auto Weapon::Cartridge::begin() const -> decltype(loaded.begin()) {
	return loaded.begin();
}

auto Weapon::Cartridge::end() const -> decltype(loaded.end()) {
	return loaded.end();
}

int Weapon::Cartridge::getCapacity() const {
	return capacity;
}

int Weapon::Cartridge::getCurrSize() const {
	return (int) loaded.size();
}

bool Weapon::Cartridge::isEmpty() const {
	return loaded.empty();
}

bool Weapon::Cartridge::isFull() const {
	return loaded.size() == capacity;
}
////////////////////////////////
// Scroll
Scroll::Scroll() = default;

Scroll::~Scroll() = default;

////////////////////////////////
// Potion
Potion::Potion() = default;

Potion::~Potion() = default;

Array2D<ItemPile, LEVEL_ROWS, LEVEL_COLS> itemsMap;

ItemRegistry<Food> foodTypes;
ItemRegistry<Armor> armorTypes;
ItemRegistry<Weapon> weaponTypes;
ItemRegistry<Ammo> ammoTypes;
ItemRegistry<Scroll> scrollTypes;
ItemRegistry<Potion> potionTypes;
ItemRegistry<bool> potionTypeKnown;

std::string getPotionName(const std::string & id) {
	auto it = potionTypeKnown.find(id);
	if (it != potionTypeKnown.end() and it->second) {
		switch (potionTypes[id].effect) {
			case Potion::Heal: return "a potion of healing";
			case Potion::Invisibility: return "a potion of invisibility";
			case Potion::Teleport: return "a potion of teleport";
			case Potion::None: return "a potion of... Water?";
			case Potion::Blindness: return "a potion of blindness";
			default: throw std::logic_error("Unknown potion effect");
		}
	} else {
		if (id == "blue_potion") {
			return "blue potion";
		} else if (id == "green_potion") {
			return "green potion";
		} else if (id == "dark_potion") {
			return "dark potion";
		} else if (id == "magenta_potion") {
			return "magenta potion";
		} else if (id == "yellow_potion") {
			return "yellow potion";
		}
	}
    throw std::logic_error("Unknown potion id");
}

ItemPileIter findItemAt(Coord2i cell, std::string_view id) {
    auto & pile = itemsMap[cell];
    return std::find_if(begin(pile), end(pile), [id] (const Item::Ptr & item) {
        return item->id == id;
    });
}

bool randomlySetOnMap(Item::Ptr item) {
    const int attemts = 32;

    for (int i = 0; i < attemts; ++i) {
        Coord2i cell{ std::rand() % LEVEL_COLS, std::rand() % LEVEL_ROWS };

        if (level[cell] == 1) {
            drop(std::move(item), cell);
            return true;
        }
    }

    return false;
}

void drop(Item::Ptr item, Coord2i cell) {
    if (level[cell] == 2)
        throw std::logic_error("Trying to drop an item in a wall");
    if (not item)
        return;
    item->pos = cell;
    if (item->isStackable) {
        auto it = findItemAt(cell, item->id);
        if (it != end(itemsMap[cell])) {
            (*it)->count += item->count;
            return;
        }
    }
    itemsMap[cell].push_back(std::move(item));
}

