#ifndef UNIT_HPP
#define UNIT_HPP

#include"item.hpp"
#include"controls.hpp"
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

	const char* getName();
	bool linearVisibilityCheck( double from_x, double from_y, double to_x, double to_y );
	bool canSeeCell( int h, int l );
	void delay(double s);	
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

	Direction dir;
	int dist;
	int movedOnTurn;
	int xpIncreasing;
	int targetH;
	int targetL;

	void shoot();

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
	
	void findVisibleArray();
	void attackEnemy(int& a1, int& a2);
	void mHLogic(int& a1, int& a2);
	bool isInventoryEmpty();
	int findEmptyInventoryCell();
	int getInventoryItemsWeight();
	void printList(PossibleItem items[], int len, char msg[], int mode);
	bool isMapInInventory();
	int findItemsCountUnderThisCell(int h, int l);
	int findEmptyItemUnderThisCell(int h, int l);
	int findNotEmptyItemUnderThisCell(int h, int l);
	int findAmmoInInventory();
	int findScrollInInventory();
	void printAmmoList(PossibleItem& pAmmo);										// Picked ammo
	void pickUp();
	bool isFoodInInventory();
	bool isArmorInInventory();
	bool isWeaponOrToolsInInventory();
	bool isPotionInInventory();
	void clearRightPane();
	void delay(double s);
	void throwAnimated(PossibleItem& item, Direction direction);
	void shoot();
	void showInventory(const char& inp);
	void eat();
	void moveHero(char& inp);
};
#endif // UNIT_HPP
