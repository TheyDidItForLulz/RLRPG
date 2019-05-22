#ifndef UNIT_HPP
#define UNIT_HPP

#include<item.hpp>
#include<array2d.hpp>
#include<termlib/vec2.hpp>
#include<inventory.hpp>

#define DELAY 0.07
#define ENEMIESCOUNT 17
#define AMMO_SLOT 53

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

    Inventory inventory;
	Weapon* weapon = nullptr;
	Armor* armor = nullptr;

    Coord2i pos = {-1, -1};
	int health;
    int maxHealth;
	int symbol;
	int vision;

    std::string getName();
	bool canSee(Coord2i cell) const;
    void setTo(Coord2i cell);
    void heal(int hp);
    void dealDamage(int damage);

    virtual UnitType getType() const = 0;
    virtual void dropInventory();

protected:
	bool linearVisibilityCheck(Vec2d from, Vec2d to) const;
};

extern Array2D<Unit::Ptr, LEVEL_ROWS, LEVEL_COLS> unitMap;

#endif // UNIT_HPP
