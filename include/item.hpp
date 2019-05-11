#ifndef ITEM_HPP
#define ITEM_HPP

#include"level.hpp"
#include<vector>
#include<string>
#include<optional>
#include<list>
#include<memory>

const int TYPES_OF_FOOD = 2;
const int TYPES_OF_ARMOR = 2;
const int TYPES_OF_WEAPONS = 6;
const int TYPES_OF_AMMO = 2;
const int TYPES_OF_SCROLLS = 2;
const int TYPES_OF_POTIONS = 5;
const int TYPES_OF_TOOLS = 1;

const int FOOD_COUNT = 10;
const int ARMOR_COUNT = 4;
const int WEAPON_COUNT = 25; /* JUST FOR !DEBUG!!*/
const int AMMO_COUNT = 25;
const int SCROLL_COUNT = 15; /* JUST FOR !DEBUG!!*/
const int POTION_COUNT = 25; /* IT TOO */
const int TOOL_COUNT = 5; /* AND IT */

#define BANDOLIER TYPES_OF_AMMO
#define MAX_USABLE_INV_SIZE 53
#define MAX_TOTAL_INV_SIZE 54+BANDOLIER
#define FIELD_DEPTH 11
#define MAX_CARTRIDGE_SIZE 10

enum ItemType {
	ItemFood = 1 << 0,
	ItemArmor = 1 << 1,
	ItemEmpty = 1 << 2,
	ItemWeapon = 1 << 3,
	ItemAmmo = 1 << 4,
	ItemScroll = 1 << 5,
	ItemPotion = 1 << 6,
	ItemTools = 1 << 7
};

class Item {
public:
    using Ptr = std::unique_ptr<Item>;

	Item();
	virtual ~Item();

	int posH;
	int posL;
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

class EmptyItem: public Item {
public:
	EmptyItem();
	~EmptyItem();

    ItemType getType() const override {
        return ItemEmpty;
    }

    Item::Ptr clone() const override {
        return std::make_unique<EmptyItem>(*this);
    }
};

class Food: public Item {
public:	
	Food(int FoodType);
	
	int FoodHeal;
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
	Ammo(int AmmoType);

	int range;
	int damage;
	//int count;

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
	Weapon(int WeaponType);
	
	int damage;

	int range; 									// Ranged bullets have additional effect on this paramether
	int damageBonus;								// And on this too
	bool Ranged;
	int cartridgeSize;
	int currentCS;
	
    std::unique_ptr<Ammo> cartridge[MAX_CARTRIDGE_SIZE];
//	vector<Ammo> cartridge;

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
	Scroll(int s);
	int effect;

	Scroll();
	~Scroll();

    ItemType getType() const override {
        return ItemScroll;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Scroll>(*this);
    }
};

class Potion: public Item {
public:
	Potion(int p);
    std::string getPotionName() const;
	int effect;

	Potion();
	~Potion();

    ItemType getType() const override {
        return ItemPotion;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Potion>(*this);
    }
};

std::string getPotionName(int sym);

class Tools: public Item {
public:
	Tools(int t);
	int possibility;
	int uses;

	int damage;									// It is nedlectful to use weapon's attributes on tools

	int range; 									// Ranged bullets have add effect on this paramether
	bool Ranged;
	int cooldown;									// The end of using attributes

	Tools();
	~Tools();

    ItemType getType() const override {
        return ItemTools;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Tools>(*this);
    }
};

using ItemPile = std::list<std::unique_ptr<Item>>;
using ItemPileIter = ItemPile::iterator;
extern ItemPile itemsMap[FIELD_ROWS][FIELD_COLS];
extern Item::Ptr inventory[MAX_TOTAL_INV_SIZE];

extern std::vector<Food> foodTypes;
extern std::vector<Armor> armorTypes;
extern std::vector<Weapon> weaponTypes;
extern std::vector<Ammo> ammoTypes;
extern std::vector<Scroll> scrollTypes;
extern std::vector<Potion> potionTypes;
extern std::vector<Tools> toolTypes;
extern std::vector<bool> potionTypeKnown;

ItemPileIter findItemAtCell(int row, int col, int sym);
bool randomlySetOnMap(Item::Ptr item);

template<class ItemType>
ItemType selectOne(const std::vector<ItemType> & types) {
    return types[std::rand() % types.size()];
}

template<class ItemType, class Fn = decltype(selectOne<ItemType>)>
void randomlySelectAndSetOnMap(const std::vector<ItemType> & types, int n, const Fn & itemSelector = selectOne<ItemType>) {
    for (int i = 0; i < n; ++i) {
        Item::Ptr item = std::make_unique<ItemType>(itemSelector(types));
        randomlySetOnMap(std::move(item));
    }
}

#endif // ITEM_HPP

