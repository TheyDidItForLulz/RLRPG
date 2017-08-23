#ifndef UNIT_HPP
#define UNIT_HPP

#include"item.hpp"
#include<stdlib.h>
#define DELAY 0.07
#define ENEMIESCOUNT 17
#define DEFAULT_VISION 16
#define VISION_PRECISION 256
#define UNITINVENTORY 4
#define AMMO_SLOT 53

extern int VISION;
extern int MaxInvItemsWeight;
extern int DEFAULT_HERO_HEALTH;

class Unit
{
public:
	Unit();
	int health;
	PossibleItem unitInventory[UNITINVENTORY];
	int inventoryVol;
	PossibleItem* unitWeapon;
	PossibleItem* unitArmor;
	PossibleItem* unitAmmo;
	int posH;
	int posL;
	int symbol;
	int vision;

	const char* GetName();
	bool LinearVisibilityCheck( double from_x, double from_y, double to_x, double to_y );
	bool CanSeeCell( int h, int l );
	void Delay(double s);	
	~Unit();
};

class EmptyUnit: public Unit
{
public:
	EmptyUnit();
	~EmptyUnit();
};

class Enemy: public Unit
{
public:
	Enemy(int eType);

	int dir;
	int dist;
	int movedOnTurn;
	int xpIncreasing;
	int targetH;
	int targetL;

	void Shoot();

	Enemy();
	~Enemy();
};

class Hero: public Unit
{
public:
	Hero();

	int hunger;
	int xp;
	int level;
	PossibleItem* heroArmor;
	PossibleItem* heroWeapon;
	bool isBurdened;
	bool CanHeroMoveThroughWalls;
	
	void FindVisibleArray();
	void AttackEnemy(int& a1, int& a2);
	void mHLogic(int& a1, int& a2);
	bool isInventoryEmpty();
	int FindEmptyInventoryCell();
	int GetInventoryItemsWeight();
	void PrintList(PossibleItem items[], int len, char msg[], int mode);
	bool IsMapInInventory();
	int FindItemsCountUnderThisCell(int h, int l);
	int FindEmptyItemUnderThisCell(int h, int l);
	int FindNotEmptyItemUnderThisCell(int h, int l);
	int FindAmmoInInventory();
	int FindScrollInInventory();
	void PrintAmmoList(PossibleItem& pAmmo);										// Picked ammo
	void PickUp();
	bool isFoodInInventory();
	bool isArmorInInventory();
	bool isWeaponOrToolsInInventory();
	bool isPotionInInventory();
	void ClearRightPane();
	void Delay(double s);
	void ThrowAnimated(PossibleItem& item, char direction);
	void Shoot();
	void ShowInventory(const char& inp);
	void Eat();
	void moveHero(char& inp);
};
#endif // UNIT_HPP
