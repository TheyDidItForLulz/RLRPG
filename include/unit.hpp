#ifndef UNIT_HPP
#define UNIT_HPP

#include"item.hpp"
#include"controls.hpp"
#include"utils.hpp"
#include<stdlib.h>
#include<termlib/vec2.hpp>

#define DELAY 0.07
#define ENEMIESCOUNT 17
#define DEFAULT_VISION 16
#define VISION_PRECISION 256
#define AMMO_SLOT 53

extern int g_vision;
extern int g_maxBurden;

enum UnitType {
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

    Coord pos = {-1, -1};
	int health;
    int maxHealth;
	int symbol;
	int vision;

    std::string getName();
	bool canSee(Coord cell) const;
    void setTo(Coord cell);
    void heal(int hp);
    void dealDamage(int damage);

    virtual UnitType getType() const = 0;

protected:
	bool linearVisibilityCheck(Vec2d from, Vec2d to) const;
};

class Enemy: public Unit {
public:
    static const int TYPES_COUNT = 3;
    static const int MAX_INV_SIZE = 4;

    Item::Ptr inventory[MAX_INV_SIZE];

	Ammo* ammo = nullptr;
    std::optional<Coord> target;
	int lastTurnMoved = 0;
	int xpCost;

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

private:
    std::optional<Coord> searchForShortestPath(Coord to) const; // returns next cell in the path if path exists
    void moveOrAttack(Coord cell);
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
	void clearRightPane() const;
    void dealDamage(int damage);
	void processInput(char inp);

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
	void attackEnemy(Coord cell);
	void throwAnimated(Item::Ptr item, Direction direction);
	void shoot();
	void eat();
	void pickUp();
	void showInventory(char inp);

	void printList(const std::vector<Item *> & items, std::string_view msg, int mode) const;

	void pickUpAmmo(ItemPileIter ammo);
	void moveTo(Coord cell);

    void levelUp();
};

extern Unit::Ptr unitMap[FIELD_ROWS][FIELD_COLS];

#endif // UNIT_HPP
