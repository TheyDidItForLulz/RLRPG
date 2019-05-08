#ifndef ITEM_HPP
#define ITEM_HPP

#include"level.hpp"
#include<vector>
#include<string>

using namespace std;

const int TYPES_OF_FOOD = 2;
const int TYPES_OF_ARMOR = 2;
const int TYPES_OF_WEAPONS = 6;
const int TYPES_OF_AMMO = 2;
const int TYPES_OF_SCROLLS = 2;
const int TYPES_OF_POTIONS = 5;
const int TYPES_OF_TOOLS = 1;

#define FOODCOUNT 10
#define ARMORCOUNT 4
#define WEAPONCOUNT 25	/* JUST FOR !DEBUG!!*/
#define AMMOCOUNT 25
#define SCROLLCOUNT 15 /* JUST FOR !DEBUG!!*/
#define POTIONCOUNT 25 /* IT TOO */
#define TOOLSCOUNT 5 /* AND IT */
#define BANDOLIER TYPES_OF_AMMO
#define MAX_USABLE_INV_SIZE 53
#define MAX_TOTAL_INV_SIZE 54+BANDOLIER
#define FIELD_DEPTH 11
#define MAX_CARTRIDGE_SIZE 10

enum ItemType {
	ItemFood,
	ItemArmor,
	ItemEmpty,
	ItemWeapon,
	ItemAmmo,
	ItemScroll,
	ItemPotion,
	ItemTools
};

class Item
{
public:
		
	Item();
	~Item();

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

    std::string getMdf();
    std::string getAttribute();
    std::string getName();
};

class EmptyItem: public Item
{
public:
	EmptyItem();
	~EmptyItem();
};

class Food: public Item
{
public:	
	Food(int FoodType);
	
	int FoodHeal;
	bool isRotten;

	Food();
	~Food();
};

class Armor: public Item
{
public:
	Armor(int ArmorType);

	int defence;
	int durability;

	Armor();
	~Armor();
};

class Ammo: public Item
{
public:
	Ammo(int AmmoType);

	int range;
	int damage;
	int count;

	Ammo();
	~Ammo();
};

class Weapon: public Item
{
public:
	Weapon(int WeaponType);
	
	int damage;

	int range; 									// Ranged bullets have additional effect on this paramether
	int damageBonus;								// And on this too
	bool Ranged;
	int cartridgeSize;
	int currentCS;
	
	Ammo cartridge[MAX_CARTRIDGE_SIZE];
//	vector<Ammo> cartridge;

	Weapon();
	
	Weapon(const Weapon&);

	Weapon& operator=(const Weapon&);

	~Weapon();
};

class Scroll: public Item
{
public:
	Scroll(int s);
	int effect;

	Scroll();
	~Scroll();
};

class Potion: public Item
{
public:
	Potion(int p);
    std::string getPotionName();
	int effect;

	Potion();
	~Potion();
};

std::string getPotionName(int sym);

class Tools: public Item
{
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
};

union InventoryItem
{
	EmptyItem invEmpty;
	Food invFood;
	Armor invArmor;
	Weapon invWeapon;
	Ammo invAmmo;
	Scroll invScroll;
	Potion invPotion;
	Tools invTools;
	InventoryItem(EmptyItem e);
	InventoryItem(Food f);
	InventoryItem(Armor a);
	InventoryItem(Weapon w);
	InventoryItem(Ammo am);
	InventoryItem(Scroll s);
	InventoryItem(Potion p);
	InventoryItem(Tools t);
	InventoryItem();
	InventoryItem(const InventoryItem& i) = delete;
	InventoryItem& operator=(const InventoryItem& ii) = delete;
	~InventoryItem();
};

struct PossibleItem
{
	InventoryItem item;
	ItemType type;
	PossibleItem(InventoryItem i, ItemType t);
	PossibleItem();
	void operator=(const Food& f);
	void operator=(const Armor& a);
	void operator=(const EmptyItem& e);
	void operator=(const Weapon& w);
	void operator=(const Ammo& am);
	void operator=(const Scroll& s);
	void operator=(const Potion& p);
	void operator=(const Tools& t);
	void operator=(const PossibleItem& other);
	PossibleItem(const PossibleItem& other);
	Item& getItem();
	~PossibleItem();
};

extern PossibleItem itemsMap[FIELD_ROWS][FIELD_COLS][FIELD_DEPTH];
extern PossibleItem inventory[MAX_TOTAL_INV_SIZE];

extern Food differentFood[TYPES_OF_FOOD];

extern Armor differentArmor[TYPES_OF_ARMOR];

extern Weapon differentWeapon[TYPES_OF_WEAPONS];

extern Ammo differentAmmo[TYPES_OF_AMMO];

extern Scroll differentScroll[TYPES_OF_SCROLLS];

extern Potion differentPotion[TYPES_OF_POTIONS];

extern Tools differentTools[TYPES_OF_TOOLS];

extern bool discoveredPotions[TYPES_OF_POTIONS];

#endif // ITEM_HPP

