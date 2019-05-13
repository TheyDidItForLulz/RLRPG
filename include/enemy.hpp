#ifndef ENEMY_HPP
#define ENEMY_HPP

#include<unit.hpp>

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
    void moveTo(Coord cell);
};

extern Enemy enemyTypes[Enemy::TYPES_COUNT];

#endif // ENEMY_HPP

