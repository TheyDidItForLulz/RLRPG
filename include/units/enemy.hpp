#ifndef ENEMY_HPP
#define ENEMY_HPP

#include<units/unit.hpp>
#include<enable_clone.hpp>

#include<tl/optional.hpp>

#include<string_view>

class Ammo;

class Enemy
    : public Unit
    , public EnableClone<Enemy>
{
public:
    Ammo* ammo = nullptr;
    tl::optional<Coord2i> target;
    int lastTurnMoved = 0;
    int xpCost;

    Enemy() = default;
    Enemy(Enemy const &);
    Enemy & operator =(Enemy const &);

    void shoot();
    void updatePosition();
    void dropInventory() override;

    Type getType() const override {
        return Type::Enemy;
    }

private:
    tl::optional<Coord2i> searchForShortestPath(Coord2i to) const; // returns next cell in the path if path exists
    void moveTo(Coord2i cell);
};

#endif // ENEMY_HPP

