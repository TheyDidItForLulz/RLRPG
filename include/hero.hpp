#ifndef HERO_HPP
#define HERO_HPP

#include<unit.hpp>
#include<utils.hpp>
#include<direction.hpp>

class Hero: public Unit {
public:
    static const int MAX_LUCK = 20;
    static const int DEFAULT_VISION = 16;

	int hunger = 900;
	int xp = 0;
    int maxBurden = 25;
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
	bool isMapInInventory() const;
	bool isFoodInInventory() const;
	bool isArmorInInventory() const;
	bool isWeaponInInventory() const;
	bool isPotionInInventory() const;

	std::optional<char> findAmmoInInventory() const;
	std::optional<char> findScrollInInventory() const;

	int getInventoryItemsWeight() const;

    int getLevelUpXP() const;
    bool tryLevelUp(); // returns true if reaches new level

    Type getType() const override {
        return Type::Hero;
    }

private:
	void attackEnemy(Coord2i cell);
	void throwAnimated(Item::Ptr item, Direction direction);
	void shoot();
	void eat();
	void pickUp();
	void showInventory(char inp);

	void printList(std::vector<Item *> items, std::string_view msg, int mode) const;

	void moveTo(Coord2i cell);

    void levelUp();
};

#endif // HERO_HPP

