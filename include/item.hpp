#ifndef ITEM_HPP
#define ITEM_HPP

#include"level.hpp"

#define TypesOfFood 2
#define TypesOfArmor 2
#define TypesOfWeapon 6
#define TypesOfAmmo 2
#define TypesOfScroll 2
#define TypesOfPotion 5
#define TypesOfTools 1
#define FOODCOUNT 10
#define ARMORCOUNT 4
#define WEAPONCOUNT 25	/* JUST FOR !DEBUG!!*/
#define AMMOCOUNT 25
#define SCROLLCOUNT 15 /* JUST FOR !DEBUG!!*/
#define POTIONCOUNT 25 /* IT TOO */
#define TOOLSCOUNT 5 /* AND IT */
#define BANDOLIER TypesOfAmmo
#define MaxInvVol 53
#define TrueMaxInvVol 54+BANDOLIER
#define Depth 11
#define MaxCartridgeVol 10

enum ItemType
{
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

	const char * GetMdf();
	const char * GetAttribute();
	const char * GetName();
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
	
	Ammo cartridge[MaxCartridgeVol];

	Weapon();
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
	const char * GetPotionName();
	int effect;

	Potion();
	~Potion();
};

const char* GetPotionName(int sym);

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
	InventoryItem(InventoryItem& i);
	InventoryItem();
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
	Item& GetItem();
};

extern PossibleItem ItemsMap[FIELD_ROWS][FIELD_COLS][Depth];
extern PossibleItem inventory[TrueMaxInvVol];

extern Food differentFood[TypesOfFood];

extern Armor differentArmor[TypesOfArmor];

extern Weapon differentWeapon[TypesOfWeapon];

extern Ammo differentAmmo[TypesOfAmmo];

extern Scroll differentScroll[TypesOfScroll];

extern Potion differentPotion[TypesOfPotion];

extern Tools differentTools[TypesOfTools];

extern bool discoveredPotions[TypesOfPotion];

#endif // ITEM_HPP

