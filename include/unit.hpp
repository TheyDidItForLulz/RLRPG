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

enum UnitType {
    UnitEmpty,
    UnitHero,
    UnitEnemy
};

class Unit {
public:
    using Ptr = std::unique_ptr<Unit>;

	Unit() {}
    Unit(const Unit &);
	~Unit() {}

    Unit & operator=(const Unit &);

	Weapon* weapon = nullptr;
	Armor* armor = nullptr;

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
    void move(int row, int col);
    void heal(int hp);
    void dealDamage(int damage);

    virtual UnitType getType() const = 0;
};

class EmptyUnit: public Unit {
public:
	EmptyUnit() {}
	~EmptyUnit() {}

    UnitType getType() const override {
        return UnitEmpty;
    }
};

class Enemy: public Unit {
public:
    static const int TYPES_COUNT = 3;
    static const int MAX_INV_SIZE = 4;

    Item::Ptr inventory[MAX_INV_SIZE];

	Direction dir;
	int dist = 0;
	int movedOnTurn = 0;
	int xpIncreasing;
	int targetH = -1;
	int targetL = -1;
	Ammo* ammo = nullptr;

	Enemy() {}
	Enemy(int eType);
    Enemy(const Enemy &);

    Enemy & operator =(const Enemy &);

	~Enemy() {}

	void shoot();
    void updatePosition();
    void dropInventory();

    UnitType getType() const override {
        return UnitEnemy;
    }
};

extern Enemy enemyTypes[Enemy::TYPES_COUNT];

class Hero: public Unit {
public:
    static const int MAX_LUCK = 20;
    static const int EMPTY_SLOT = MAX_USABLE_INV_SIZE + 1;

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
	void attackEnemy(int row, int col);
	void printList(const std::vector<Item *> & items, std::string_view msg, int mode) const;
	void pickUp();
	void clearRightPane() const;
	void throwAnimated(Item::Ptr item, Direction direction);
	void shoot();
	void showInventory(char inp);
	void eat();
	void moveHero(char inp);
    void dealDamage(int damage);

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

    int getLevelUpXP() const;
    bool tryLevelUp(); // returns true if reaches new level

    UnitType getType() const override {
        return UnitHero;
    }

private:
	void pickUpAmmo(ItemPileIter ammo);
	void moveHeroImpl(int row, int col);

    void levelUp();
};

extern Unit::Ptr unitMap[FIELD_ROWS][FIELD_COLS];

#endif // UNIT_HPP
