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

class Unit {
public:
	Unit() {}
	~Unit() {}

	PossibleItem unitInventory[UNITINVENTORY];

	PossibleItem* unitWeapon;
	PossibleItem* unitArmor;
	PossibleItem* unitAmmo;

	int health;
    int maxHealth;
	int inventoryVol = 0;
	int posH;
	int posL;
	int symbol;
	int vision;

    std::string getName();
	bool linearVisibilityCheck(double fromX, double fromY, double toX, double toY) const;
	bool canSeeCell(int h, int l) const;
    void dropInventory();
    void move(int row, int col);
    void heal(int hp);
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

    Hero();
	
	void checkVisibleCells();
	void attackEnemy(int& a1, int& a2);
	void printList(const std::vector<PossibleItem> & items, std::string_view msg, int mode) const;
	void pickUp();
	void clearRightPane() const;
	void throwAnimated(PossibleItem& item, Direction direction);
	void shoot();
	void showInventory(char inp);
	void eat();
	void moveHero(char inp);

    bool isInvisible() const;
	bool isInventoryEmpty() const;
	bool isMapInInventory() const;
	bool isFoodInInventory() const;
	bool isArmorInInventory() const;
	bool isWeaponOrToolsInInventory() const;
	bool isPotionInInventory() const;

	int findEmptyInventoryCell() const;
	int findAmmoInInventory() const;
	int findScrollInInventory() const;

	int getInventoryItemsWeight() const;

private:
	void pickUpAmmo(ItemPileIter ammo);
	void moveHeroImpl(int& a1, int& a2);
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

    Unit & getUnit();
    const Unit & getUnit() const;
};

extern PossibleUnit unitMap[FIELD_ROWS][FIELD_COLS];

#endif // UNIT_HPP
