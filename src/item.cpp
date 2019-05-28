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
	}
    throw std::logic_error("Unknown modifier id");
}

std::string Item::getName() const {
	switch (symbol) {
		case 100: return "an egg";
		case 101: return "an apple";
		case 300: return "a chain chestplate";
		case 301: return "a leather chestplate";
		case 400: return "a copper shortsword";
		case 401: return "a bronze spear";
		case 402: return "a musket";
		case 403: return "a stick";
		case 404: return "a shotgun";
		case 405: return "a pistol";
        case 406: return "a pickaxe";
		case 450: return "steel bullets";
		case 451: return "shotgun shells";
		case 500: return "a map";
		case 501: return "an identify scroll";
		case 600:
		case 601:
		case 602:
		case 603:
		case 604: return getPotionName(symbol);
	}
    throw std::logic_error("Unknown item id");
}
Item::~Item(){};

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
Food::Food(int FoodType)
{
	switch (FoodType)
	{
		case 0:
			symbol = 100;
			nutritionalValue = 100;
			weight = 1;
			break;
		case 1:
			symbol = 101;
			nutritionalValue = 125;
			weight = 1;
			break;
	}
	isStackable = true;
};

Food::Food(): isRotten(false){};
Food::~Food(){};

////////////////////////////////
// Armor
Armor::Armor(int ArmorType) {
	switch (ArmorType) {
		case 0:
			symbol = 300;
			defence =33;
			durability = 20;
			weight = 20;
			break;
		case 1:
			symbol = 301;
			defence = 20;
			durability = 15;
			weight = 7;
			break;
	}
	mdf = 1;
	isStackable = false;
}

Armor::Armor(){};
Armor::~Armor(){};

////////////////////////////////
// Ammo
Ammo::Ammo(int AmmoType) {
	switch (AmmoType) {
		case 0:
			symbol = 450;
			weight = 0;
			range = 2;
			damage = 1;
			break;
		case 1:
			symbol = 451;
			weight = 0;
			range = 1;
			damage = 2;
			break;
	}
	isStackable = true;
};

Ammo::Ammo(){}
Ammo::~Ammo(){}

////////////////////////////////
// Weapon
Weapon::Weapon(int WeaponType) {
	switch (WeaponType) {
		case 0:
			symbol = 400;
			damage = 4;
			weight = 3;
			isRanged = false;
			break;
		case 1:
			symbol = 401;
			damage = 5;
			weight = 5;
			isRanged = false;
			break;
		case 2:
			symbol = 402;
			damage = 3;
			weight = 3;
			range = 14;
			isRanged = true;
			damageBonus = 4;
			cartridge = Cartridge(1);
			break;
		case 3:
			symbol = 403;
			damage = 2;
			weight = 1;
			isRanged = false;
			break;
		case 4:
			symbol = 404;
			damage = 2;
			weight = 4;
			isRanged = true;
			range = 4;
			damageBonus = 5;
			cartridge = Cartridge(6);
			break;
		case 5:
			symbol = 405;
			damage = 1;
			weight = 2;
			isRanged = true;
			range = 7;
			damageBonus = 2;
			cartridge = Cartridge(10);
			break;
        case 6:
			symbol = 406;
			damage = 2;
			weight = 5;
			isRanged = false;
            canDig = true;
	}
	isStackable = false;
};

Weapon::Weapon() {}

Weapon::~Weapon(){};

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
Scroll::Scroll(int s) {
	switch (s) {
		case 0:
			symbol = 500;
			weight = 1;
			effect = 1;
			break;
		case 1:
			symbol = 501;
			weight = 1;
			effect = 2;
	}
	isStackable = true;
}

Scroll::Scroll(){}
Scroll::~Scroll(){}

////////////////////////////////
// Potion
Potion::Potion(int p) {
	switch (p) {
		case 0:
			symbol = 600;
			break;
		case 1:
			symbol = 601;
			break;
		case 2:
			symbol = 602;
			break;
		case 3:
			symbol = 603;
			break;
		case 4:
			symbol = 604;
			break;
	}
	weight = 1;
	effect = 0;
	isStackable = true;
}

Potion::Potion(){}
Potion::~Potion(){}

Array2D<ItemPile, LEVEL_ROWS, LEVEL_COLS> itemsMap;

std::vector<Food> foodTypes;
std::vector<Armor> armorTypes;
std::vector<Weapon> weaponTypes;
std::vector<Ammo> ammoTypes;
std::vector<Scroll> scrollTypes;
std::vector<Potion> potionTypes;
std::vector<bool> potionTypeKnown;

std::string getPotionName(int sym) {
	sym -= 600;
	
	if (potionTypeKnown[sym]) {
		switch (potionTypes[sym].effect) {
			case 1: return "a potion of healing";
			case 2: return "a potion of invisibility";
			case 3: return "a potion of teleport";
			case 4: return "a potion of... Water?";
			case 5: return "a potion of blindness";
		}
	} else {
		switch (sym) {
			case 0: return "blue potion";
			case 1: return "green potion";
			case 2: return "dark potion";
			case 3: return "magenta potion";
			case 4: return "yellow potion";
		}
	}
    throw std::logic_error("Unknown potion id");
}

ItemPileIter findItemAt(Coord2i cell, int sym) {
    auto & pile = itemsMap[cell];
    return std::find_if(begin(pile), end(pile), [sym] (const Item::Ptr & item) {
        return item->symbol == sym;
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
        auto it = findItemAt(cell, item->symbol);
        if (it != end(itemsMap[cell])) {
            (*it)->count += item->count;
            return;
        }
    }
    itemsMap[cell].push_back(std::move(item));
}

