#ifndef UNIT_HPP
#define UNIT_HPP

#include<item.hpp>
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

extern Unit::Ptr unitMap[FIELD_ROWS][FIELD_COLS];

#endif // UNIT_HPP
