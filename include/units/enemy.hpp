#ifndef ENEMY_HPP
#define ENEMY_HPP

#include<units/unit.hpp>

#include<string_view>
#include<unordered_map>

class Ammo;

class Enemy: public Unit {
public:
	Ammo* ammo = nullptr;
    std::optional<Coord2i> target;
	int lastTurnMoved = 0;
	int xpCost;

	Enemy() {}
	Enemy(std::string_view id);
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

extern std::unordered_map<std::string, Enemy> enemyTypes;

#endif // ENEMY_HPP

