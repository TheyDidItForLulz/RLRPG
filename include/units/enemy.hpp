#ifndef ENEMY_HPP
#define ENEMY_HPP

#include<units/unit.hpp>
#include<enable_clone.hpp>

#include<optional>
#include<string_view>

class Ammo;

class Enemy
    : public Unit
    , public EnableClone<Enemy>
{
public:
	Ammo* ammo = nullptr;
    std::optional<Coord2i> target;
	int lastTurnMoved = 0;
	int xpCost;

	Enemy() {}
	Enemy(std::string_view id);
    Enemy(Enemy const &);

    Enemy & operator =(Enemy const &);

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

#endif // ENEMY_HPP

