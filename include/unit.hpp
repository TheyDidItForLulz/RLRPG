#ifndef UNIT_HPP
#define UNIT_HPP

#include"item.hpp"
#include"controls.hpp"
#include"utils.hpp"
#include<stdlib.h>
#define DELAY 0.07
#define ENEMIESCOUNT 17
#define DEFAULT_VISION 16
#define VISION_PRECISION 256
#define UNITINVENTORY 4
#define AMMO_SLOT 53

extern int g_vision;
extern int g_maxBurden;
extern int DEFAULT_HERO_HEALTH;

class Unit {
public:
	Unit() {}
	~Unit() {}

	PossibleItem unitInventory[UNITINVENTORY];

	PossibleItem* unitWeapon;
	PossibleItem* unitArmor;
	PossibleItem* unitAmmo;

	int health;
	int inventoryVol = 0;
	int posH;
	int posL;
	int symbol;
	int vision;

    std::string getName();
	bool linearVisibilityCheck(double fromX, double fromY, double toX, double toY);
	bool canSeeCell(int h, int l);
    void dropInventory();
};

class EmptyUnit: public Unit {
public:
	EmptyUnit() {}
	~EmptyUnit() {}
};

class Enemy: public Unit {
public:
    static const int TYPES_COUNT = 3;
	Direction dir;
	int dist;
	int movedOnTurn = 0;
	int xpIncreasing;
	int targetH;
	int targetL;

	Enemy() {}
	Enemy(int eType);

	~Enemy() {}

	void shoot();
    void updatePosition();
};

extern Enemy differentEnemies[Enemy::TYPES_COUNT];

class Hero: public Unit {
public:
    static const int MAX_LUCK = 20;
    static const int EMPTY_SLOT = MAX_USABLE_INV_SIZE + 1;

	PossibleItem* heroArmor;
	PossibleItem* heroWeapon;

	int hunger = 900;
	int xp = 0;
	int level = 1;
    int turnsBlind = 0;
    int turnsInvisible = 0;
    int luck = avg(std::rand() % MAX_LUCK, std::rand() % MAX_LUCK);

	bool isBurdened = false;
	bool canMoveThroughWalls = false;

	Hero() {}
	
	void checkVisibleCells();
	void attackEnemy(int& a1, int& a2);
	void mHLogic(int& a1, int& a2);
	bool isInventoryEmpty();
	int findEmptyInventoryCell();
	int getInventoryItemsWeight();
	void printList(PossibleItem items[], int len, std::string_view msg, int mode);
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
	void throwAnimated(PossibleItem& item, Direction direction);
	void shoot();
	void showInventory(char inp);
	void eat();
	void moveHero(char inp);
    bool isInvisible() const;
};

enum UnitType {
    UnitEmpty,
    UnitHero,
    UnitEnemy
};

union UnitedUnits {
    EmptyUnit uEmpty;
    Hero uHero;
    Enemy uEnemy;

    UnitedUnits(EmptyUnit e) {
        uEmpty = e;
    }

    UnitedUnits(Hero h) {
        uHero = h;
    }

    UnitedUnits(Enemy en) {
        uEnemy = en;
    }

    UnitedUnits(const UnitedUnits& u) = delete;
    UnitedUnits& operator=(const UnitedUnits& u) = delete;

    UnitedUnits() {
        uEmpty = EmptyUnit();
    }

    ~UnitedUnits(){}
};

struct PossibleUnit {
    UnitedUnits unit;
    UnitType type;

    PossibleUnit(UnitedUnits u, UnitType t);

    PossibleUnit() {
        type = UnitEmpty;
    }

    void operator=(const Hero& h) {
        type = UnitHero;
        unit.uHero = h;
    }

    void operator=(const EmptyUnit& e) {
        type = UnitEmpty;
        unit.uEmpty = e;
    }

    void operator=(const Enemy& en) {
        type = UnitEnemy;
        unit.uEnemy = en;
    }

    PossibleUnit(const PossibleUnit& p);

    PossibleUnit& operator=(const PossibleUnit& p);

    Unit& getUnit();
};

extern PossibleUnit unitMap[FIELD_ROWS][FIELD_COLS];

#endif // UNIT_HPP
