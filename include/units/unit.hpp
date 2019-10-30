#ifndef UNIT_HPP
#define UNIT_HPP

#include<inventory.hpp>

#include<termlib/vec2.hpp>

#include<string>

#define DELAY 0.07
#define ENEMIESCOUNT 17
#define AMMO_SLOT 53

class Armor;
class Weapon;

class Unit {
public:
    enum class Type {
        Hero,
        Enemy
    };

    Unit() = default;
    Unit(Unit const &);
    Unit & operator=(Unit const &);

    Inventory inventory;
	Weapon* weapon = nullptr;
	Armor* armor = nullptr;

	std::string id;
	std::string name;
    Coord2i pos = {-1, -1};
	int health;
    int maxHealth;
	int vision;

    std::string getName();
	bool canSee(Coord2i cell) const;
    void setTo(Coord2i cell);
    void heal(int hp);
    void dealDamage(int damage);

    virtual Type getType() const = 0;
    virtual void dropInventory();

protected:
	bool linearVisibilityCheck(Vec2d from, Vec2d to) const;

    virtual void takeArmorOff();
    virtual void unequipWeapon();
};

#endif // UNIT_HPP
