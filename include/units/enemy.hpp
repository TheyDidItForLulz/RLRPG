#ifndef ENEMY_HPP
#define ENEMY_HPP

#include<units/unit.hpp>

class Enemy: public Unit {
public:
    static const int TYPES_COUNT = 3;

	Ammo* ammo = nullptr;
    std::optional<Coord2i> target;
	int lastTurnMoved = 0;
	int xpCost;

	Enemy() {}
	Enemy(int eType);
    Enemy(const Enemy &);

    Enemy & operator =(const Enemy &);

	~Enemy() {}

	void shoot();
    void updatePosition();
    void dropInventory() override;

    Type getType() const override {
        return Type::Enemy;
    }

private:
    std::optional<Coord2i> searchForShortestPath(Coord2i to) const; // returns next cell in the path if path exists
    void moveTo(Coord2i cell);
};

extern Enemy enemyTypes[Enemy::TYPES_COUNT];

#endif // ENEMY_HPP

