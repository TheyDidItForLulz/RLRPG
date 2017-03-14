#include"include/item.hpp"

////////////////////////////////
// Item
Item::Item(): mdf(1), showMdf(false), attribute(100), count(1){};

const char* Item::GetMdf()
{
	switch(mdf)
	{
		case 1:
			return "nothing";
		case 2:
			return "thorns";
	}
}

const char* Item::GetAttribute()
{
	switch(attribute)
	{
		case 100:
			return "nothing";
		case 201:
			return "being worn";
		case 301:
			return "wielded";
	}
}
const char* Item::GetName()
{
	switch(symbol)
	{
		case 100:
			return "an egg";
		case 101:
			return "an apple";
		case 300:
			return "a chain chestplate";
		case 301:
			return "a leather chestplate";
		case 400:
			return "a copper shortsword";
		case 401:
			return "a bronze spear";
		case 402:
			return "a musket";
		case 403:
			return "a stick";
		case 404:
			return "a shotgun";
		case 405:
			return "a pistol";
		case 450:
			return "a steel bullets";
		case 451:
			return "a shotgun shells";
		case 500:
			return "a map";
		case 501:
			return "an identify scroll";
		case 600:
		case 601:
		case 602:
		case 603:
		case 604:
			return GetPotionName(symbol);
		case 700:
			return "pickaxe";
	}
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
	switch(FoodType)
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
Armor::Armor(int ArmorType)
{
	switch(ArmorType)
	{
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
Ammo::Ammo(int AmmoType)
{
	switch(AmmoType)
	{
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
Weapon::Weapon(int WeaponType)
{
	cartridgeSize = 0;
	currentCS = 0;
	switch(WeaponType)
	{
		case 0:
			symbol = 400;
			damage = 3;
			weight = 3;
			Ranged = false;
			break;
		case 1:
			symbol = 401;
			damage = 4;
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
};

Weapon::Weapon(){};
Weapon::~Weapon(){};

////////////////////////////////
// Scroll
Scroll::Scroll(int s)
{
	switch(s)
	{
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
Potion::Potion(int p)
{
	switch(p)
	{
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
Tools::Tools(int t)
{
	switch(t)
	{
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
InventoryItem::InventoryItem(EmptyItem e)
{
	invEmpty = e;
}
InventoryItem::InventoryItem(Food f)
{
	invFood = f;
}
InventoryItem::InventoryItem(Armor a)
{
	invArmor = a;
}
InventoryItem::InventoryItem(Weapon w)
{
	invWeapon = w;
}
InventoryItem::InventoryItem(Ammo am)
{
	invAmmo = am;
}
InventoryItem::InventoryItem(Scroll s)
{
	invScroll = s;
}
InventoryItem::InventoryItem(Potion p)
{
	invPotion = p;
}
InventoryItem::InventoryItem(Tools t)
{
	invTools = t;
}
InventoryItem::InventoryItem(InventoryItem& i){}
InventoryItem::InventoryItem()
{
	invEmpty = EmptyItem();
}
InventoryItem::~InventoryItem(){}

////////////////////////////////
// PossibleItem
PossibleItem::PossibleItem(InventoryItem i, ItemType t): item(i), type(t)
{}
PossibleItem::PossibleItem(){type = ItemEmpty;}
void PossibleItem::operator=(const Food& f)
{
	type = ItemFood;
	item.invFood = f;
}
void PossibleItem::operator=(const Armor& a)
{
	type = ItemArmor;
	item.invArmor = a;
}
void PossibleItem::operator=(const EmptyItem& e)
{
	type = ItemEmpty;
	item.invEmpty = e;
}
void PossibleItem::operator=(const Weapon& w)
{
	type = ItemWeapon;
	item.invWeapon = w;
}
void PossibleItem::operator=(const Ammo& am)
{
	type = ItemAmmo;
	item.invAmmo = am;
}
void PossibleItem::operator=(const Scroll& s)
{
	type = ItemScroll;
	item.invScroll = s;
}
void PossibleItem::operator=(const Potion& p)
{
	type = ItemPotion;
	item.invPotion = p;
}
void PossibleItem::operator=(const Tools& t)
{
	type = ItemTools;
	item.invTools = t;
}
Item& PossibleItem::GetItem()
{
	switch(type)
	{
		case ItemFood:
			return item.invFood;
		case ItemArmor:
			return item.invArmor;
		case ItemEmpty:
			return item.invEmpty;
		case ItemWeapon:
			return item.invWeapon;
		case ItemAmmo:
			return item.invAmmo;
		case ItemScroll:
			return item.invScroll;
		case ItemPotion:
			return item.invPotion;
		case ItemTools:
			return item.invTools;
	}
}

PossibleItem ItemsMap[FIELD_ROWS][FIELD_COLS][Depth];
