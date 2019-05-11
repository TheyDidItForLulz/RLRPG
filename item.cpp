#include"include/item.hpp"
#include<stdexcept>
#include<algorithm>

////////////////////////////////
// Item
Item::Item(): mdf(1), showMdf(false), attribute(100), count(1){};

std::string Item::getMdf() const {
	switch (mdf) {
		case 1: return "nothing";
		case 2: return "thorns";
	}
    throw std::logic_error("Unknown modifier id");
}

std::string Item::getAttribute() const {
	switch (attribute) {
		case 100: return "nothing";
		case 201: return "being worn";
		case 301: return "wielded";
	}
    throw std::logic_error("Unknown attribute id");
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
		case 450: return "a steel bullets";
		case 451: return "a shotgun shells";
		case 500: return "a map";
		case 501: return "an identify scroll";
		case 600:
		case 601:
		case 602:
		case 603:
		case 604: return getPotionName(symbol);
		case 700: return "pickaxe";
	}
    throw std::logic_error("Unknown item id");
}
Item::~Item(){};

////////////////////////////////
// EmptyItem
EmptyItem::EmptyItem(){};
EmptyItem::~EmptyItem(){};

////////////////////////////////
// Food
Food::Food(int FoodType)
{
	switch (FoodType)
	{
		case 0:
			symbol = 100;
			FoodHeal = 100;
			weight = 1;
			break;
		case 1:
			symbol = 101;
			FoodHeal = 125;
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
			count = 1;
			break;
		case 1:
			symbol = 451;
			weight = 0;
			range = 1;
			damage = 2;
			count = 1;
			break;
	}
	isStackable = true;
};

Ammo::Ammo(){}
Ammo::~Ammo(){}

////////////////////////////////
// Weapon
Weapon::Weapon(int WeaponType) {
	cartridgeSize = 0;
	currentCS = 0;
	switch (WeaponType) {
		case 0:
			symbol = 400;
			damage = 4;
			weight = 3;
			Ranged = false;
			break;
		case 1:
			symbol = 401;
			damage = 5;
			weight = 5;
			Ranged = false;
			break;
		case 2:
			symbol = 402;
			damage = 3;
			weight = 3;
			range = 14;
			Ranged = true;
			damageBonus = 4;
			cartridgeSize = 1;
			break;
		case 3:
			symbol = 403;
			damage = 2;
			weight = 1;
			Ranged = false;
			break;
		case 4:
			symbol = 404;
			damage = 2;
			weight = 4;
			Ranged = true;
			range = 4;
			damageBonus = 5;
			cartridgeSize = 6;
			break;
		case 5:
			symbol = 405;
			damage = 1;
			weight = 2;
			Ranged = true;
			range = 7;
			damageBonus = 2;
			cartridgeSize = 10;
			break;
	}
	isStackable = false;
/*	if (cartridgeSize)
	{
		cartridge.resize(cartridgeSize);
	}
	else
	{
		cartridge.clear();
	}*/
};

Weapon::Weapon() {
//	cartridge.clear();
};

Weapon::Weapon(const Weapon& other) {
	symbol = other.symbol;
	damage = other.damage;
	weight = other.weight;
	Ranged = other.Ranged;
	range = other.range;
	damageBonus = other.damageBonus;
	cartridgeSize = other.cartridgeSize;
	currentCS = other.currentCS;
	if (Ranged) {
		for (int i = 0; i < cartridgeSize; i++) {
            if (other.cartridge[i])
                cartridge[i] = std::make_unique<Ammo>(*other.cartridge[i]);
		}
//		cartridge = other.cartridge;
//		cartridge.clear();
//		for (const Ammo& a : other.cartridge) {
//			cartridge.push_back(a);
//		}
	}
}

Weapon& Weapon::operator=(const Weapon& other) {
	symbol = other.symbol;
	damage = other.damage;
	weight = other.weight;
	Ranged = other.Ranged;
	range = other.range;
	damageBonus = other.damageBonus;
	cartridgeSize = other.cartridgeSize;
	currentCS = other.currentCS;
	if (Ranged) {
		for (int i = 0; i < cartridgeSize; i++) {
            if (other.cartridge[i])
                cartridge[i] = std::make_unique<Ammo>(*other.cartridge[i]);
		}
//		cartridge = other.cartridge;
//		cartridge.clear();
//		for (const Ammo& a : other.cartridge) {
//			cartridge.push_back(a);
//		}
	}
	return *this;
}

Weapon::~Weapon(){};

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

////////////////////////////////
// Tools
Tools::Tools(int t) {
	switch (t) {
		case 0:
			symbol = 700;
			weight = 4;
			damage = 2;
			possibility = 1;
			Ranged = false;
			range = 1;
			break;
	}
	isStackable = false;
}

Tools::Tools(){}
Tools::~Tools(){}

////////////////////////////////
// InventoryItem

ItemPile itemsMap[FIELD_ROWS][FIELD_COLS];

Item::Ptr inventory[MAX_TOTAL_INV_SIZE];

std::vector<Food> foodTypes;
std::vector<Armor> armorTypes;
std::vector<Weapon> weaponTypes;
std::vector<Ammo> ammoTypes;
std::vector<Scroll> scrollTypes;
std::vector<Potion> potionTypes;
std::vector<Tools> toolTypes;
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

ItemPileIter findItemAtCell(int row, int col, int sym) {
    return std::find_if(begin(itemsMap[row][col]), end(itemsMap[row][col]), [sym] (const Item::Ptr & item) {
        return item->symbol == sym;
    });
}

bool randomlySetOnMap(Item::Ptr item) {
    const int attemts = 32;

    for (int i = 0; i < attemts; ++i) {
        int row = std::rand() % FIELD_ROWS;
        int col = std::rand() % FIELD_COLS;

        if (map[row][col] == 1) {
            itemsMap[row][col].push_back(std::move(item));
            itemsMap[row][col].back()->posH = row;
            itemsMap[row][col].back()->posL = col;
            return true;
        }
    }

    return false;
}

