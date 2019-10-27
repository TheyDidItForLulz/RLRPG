#include<item.hpp>

#include<game.hpp>

#include<effolkronium/random.hpp>

#include<stdexcept>
#include<algorithm>

using Random = effolkronium::random_static;


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
	return name;
}

Item::~Item() = default;

ItemPtr Item::splitStack(int toSplit) {
	if (toSplit <= 0 or toSplit > count) {
		throw std::logic_error("Trying to split an item pile invalidly");
	}
	ItemPtr otherPile = cloneItem();
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

ItemPtr Item::getByID(std::string const & id) {
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
	return ItemPtr{};
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

Weapon::Cartridge::Cartridge(Cartridge const & other): capacity(other.capacity) {
	for (auto const & bullet : other) {
		loaded.push_back(std::make_unique<Ammo>(*bullet));
	}
}

Weapon::Cartridge & Weapon::Cartridge::operator =(Cartridge const & other) {
	capacity = other.capacity;
	loaded.clear();
	for (auto const & bullet : other) {
		loaded.push_back(std::make_unique<Ammo>(*bullet));
	}
	return *this;
}

AmmoPtr Weapon::Cartridge::load(AmmoPtr bullet) {
	if (loaded.size() == capacity) {
		return bullet;
	}
	loaded.push_back(std::move(bullet));
	return nullptr;
}

AmmoPtr Weapon::Cartridge::unloadOne() {
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

Ammo const & Weapon::Cartridge::next() const {
	assert(not isEmpty());
	return *loaded.back();
}

Ammo const * Weapon::Cartridge::operator [](int ind) const {
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

std::string Potion::getName() const {
	if (g_game.isPotionKnown(id)) {
		switch (g_game.getPotionTypes()[id]->effect) {
			case Potion::Heal: return "a potion of healing";
			case Potion::Invisibility: return "a potion of invisibility";
			case Potion::Teleport: return "a potion of teleport";
			case Potion::None: return "a potion of... Water?";
			case Potion::Blindness: return "a potion of blindness";
			default: throw std::logic_error("Unknown potion effect");
		}
	} else {
	    return name;
	}
}

