#ifndef ITEM_HPP
#define ITEM_HPP

#include"level.hpp"
#include<vector>
#include<string>
#include<optional>

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
	PossibleItem(const Food& f);
	PossibleItem(const Armor& a);
	PossibleItem(const EmptyItem& e);
	PossibleItem(const Weapon& w);
	PossibleItem(const Ammo& am);
	PossibleItem(const Scroll& s);
	PossibleItem(const Potion& p);
	PossibleItem(const Tools& t);

	PossibleItem & operator=(const Food& f);
	PossibleItem & operator=(const Armor& a);
	PossibleItem & operator=(const EmptyItem& e);
	PossibleItem & operator=(const Weapon& w);
	PossibleItem & operator=(const Ammo& am);
	PossibleItem & operator=(const Scroll& s);
	PossibleItem & operator=(const Potion& p);
	PossibleItem & operator=(const Tools& t);
	PossibleItem & operator=(const PossibleItem& other);
	PossibleItem(const PossibleItem& other);
	Item& getItem();
	~PossibleItem();
};

extern PossibleItem itemsMap[FIELD_ROWS][FIELD_COLS][FIELD_DEPTH];
extern PossibleItem inventory[MAX_TOTAL_INV_SIZE];

extern std::vector<Food> foodTypes;
extern std::vector<Armor> armorTypes;
extern std::vector<Weapon> weaponTypes;
extern std::vector<Ammo> ammoTypes;
extern std::vector<Scroll> scrollTypes;
extern std::vector<Potion> potionTypes;
extern std::vector<Tools> toolTypes;
extern std::vector<bool> potionTypeKnown;

std::optional<int> findItemAtCell(int row, int col, int sym);
bool randomlySetOnMap(const PossibleItem & item);

template<class ItemType>
ItemType selectOne(const std::vector<ItemType> & types) {
    return types[std::rand() % types.size()];
}

template<class ItemType, class Fn = decltype(selectOne<ItemType>)>
void randomlySelectAndSetOnMap(const std::vector<ItemType> & types, int n, const Fn & itemSelector = selectOne<ItemType>) {
    for (int i = 0; i < n; ++i) {
        PossibleItem item = itemSelector(types);
        randomlySetOnMap(item);
    }
}

#endif // ITEM_HPP

