#ifndef ITEM_HPP
#define ITEM_HPP

#include<vector>
#include<string>
#include<optional>
#include<list>
#include<memory>
#include<functional>

#include<level.hpp>
#include<array2d.hpp>
#include<termlib/vec2.hpp>

enum ItemType {
	ItemFood = 1 << 0,
	ItemArmor = 1 << 1,
	ItemWeapon = 1 << 2,
	ItemAmmo = 1 << 3,
	ItemScroll = 1 << 4,
	ItemPotion = 1 << 5,
};

class Item {
public:
    using Ptr = std::unique_ptr<Item>;

	Item();
	virtual ~Item();

    Coord2i pos;
	int symbol;
	char inventorySymbol;
	int weight;
	int mdf;
	int attribute;
	int count;
	bool showMdf;
	bool isStackable;

    std::string getMdf() const;
    std::string getAttribute() const;
    std::string getName() const;
    virtual ItemType getType() const = 0;
    virtual Item::Ptr clone() const = 0;
};

class Food: public Item {
public:	
    static const int TYPES_COUNT = 2;
    static const int COUNT = 10;

	Food(int FoodType);
	
	int nutritionalValue;
	bool isRotten;

	Food();
	~Food();

    ItemType getType() const override {
        return ItemFood;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Food>(*this);
    }
};

class Armor: public Item {
public:
    static const int TYPES_COUNT = 2;
    static const int COUNT = 4;

	Armor(int ArmorType);

	int defence;
	int durability;

	Armor();
	~Armor();

    ItemType getType() const override {
        return ItemArmor;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Armor>(*this);
    }
};

class Ammo: public Item {
public:
    static const int TYPES_COUNT = 2;
    static const int COUNT = 25;

	Ammo(int AmmoType);

	int range;
	int damage;

	Ammo();
	~Ammo();

    ItemType getType() const override {
        return ItemAmmo;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Ammo>(*this);
    }
};

class Weapon: public Item {
public:
    static const int TYPES_COUNT = 6;
    static const int COUNT = 25; /* JUST FOR !DEBUG!!*/
    static const int MAX_CARTRIDGE_SIZE = 10;

	Weapon(int WeaponType);
	
	int damage;
	int range; 									// Ranged bullets have additional effect on this paramether
	int damageBonus;								// And on this too
	bool isRanged = false;
    bool canDig = false;
	int maxCartridgeSize;
	int currCartridgeSize = 0;
	
    std::unique_ptr<Ammo> cartridge[MAX_CARTRIDGE_SIZE];

	Weapon();
	
	Weapon(const Weapon&);

	Weapon& operator=(const Weapon&);

	~Weapon();

    ItemType getType() const override {
        return ItemWeapon;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Weapon>(*this);
    }
};

class Scroll: public Item {
public:
    static const int TYPES_COUNT = 2;
    static const int COUNT = 15; /* JUST FOR !DEBUG!!*/

	Scroll();
	Scroll(int s);
	~Scroll();

	int effect;

    ItemType getType() const override {
        return ItemScroll;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Scroll>(*this);
    }
};

class Potion: public Item {
public:
    static const int TYPES_COUNT = 5;
    static const int COUNT = 25; /* IT TOO */

	Potion();
	Potion(int p);
	~Potion();

    std::string getPotionName() const;

	int effect;

    ItemType getType() const override {
        return ItemPotion;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Potion>(*this);
    }
};

std::string getPotionName(int sym);

/*
class Tools: public Item {
public:
    static const int TYPES_COUNT = 1;
    static const int COUNT = 5;

	Tools();
	Tools(int t);
	~Tools();

	int possibility;
	int uses;
	int damage;									// It is nedlectful to use weapon's attributes on tools
	int range; 									// Ranged bullets have add effect on this paramether
	bool isRanged;
	int cooldown;									// The end of using attributes


    ItemType getType() const override {
        return ItemTools;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Tools>(*this);
    }
};*/

const int BANDOLIER = Ammo::TYPES_COUNT;
const int MAX_USABLE_INV_SIZE = 53;
const int MAX_TOTAL_INV_SIZE = MAX_USABLE_INV_SIZE + BANDOLIER;

using ItemPile = std::list<std::unique_ptr<Item>>;
using ItemPileIter = ItemPile::iterator;

extern Array2D<ItemPile, LEVEL_ROWS, LEVEL_COLS> itemsMap;
extern Item::Ptr inventory[MAX_TOTAL_INV_SIZE];

extern std::vector<Food> foodTypes;
extern std::vector<Armor> armorTypes;
extern std::vector<Weapon> weaponTypes;
extern std::vector<Ammo> ammoTypes;
extern std::vector<Scroll> scrollTypes;
extern std::vector<Potion> potionTypes;
extern std::vector<bool> potionTypeKnown;

ItemPileIter findItemAt(Coord2i cell, int sym);
bool randomlySetOnMap(Item::Ptr item);

template<class ItemType>
ItemType selectOne(const std::vector<ItemType> & types) {
    return types[std::rand() % types.size()];
}

template<class ItemType>
using ItemSelector = std::function<ItemType(const std::vector<ItemType> &)>;

template<class ItemType>
void randomlySelectAndSetOnMap(const std::vector<ItemType> & types, int n, const ItemSelector<ItemType> & selector = selectOne<ItemType>) {
    for (int i = 0; i < n; ++i) {
        Item::Ptr item = std::make_unique<ItemType>(selector(types));
        randomlySetOnMap(std::move(item));
    }
}

void drop(Item::Ptr item, Coord2i to);

#endif // ITEM_HPP

