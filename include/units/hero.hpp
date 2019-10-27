#ifndef HERO_HPP
#define HERO_HPP

#include<units/unit.hpp>
#include<utils.hpp>
#include<direction.hpp>

#include<effolkronium/random.hpp>

#include<functional>

using Random = effolkronium::random_static;

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
    int luck = avg(Random::get(0, MAX_LUCK), Random::get(0, MAX_LUCK));

	bool isBurdened = false;
	bool canMoveThroughWalls = false;

    Hero();

	void checkVisibleCells();
	void clearRightPane() const;
	void processInput(char inp);

    bool isInvisible() const;
	bool isMapInInventory() const;

	int getInventoryItemsWeight() const;

    int getLevelUpXP() const;
    bool tryLevelUp(); // returns true if reaches new level

    Type getType() const override { return Type::Hero; }

    template<class ... Args>
    bool seenUpdated(Args && ... args) const { return seenMap.at(std::forward<Args>(args)...); }

private:
	void attackEnemy(Coord2i cell);
	void throwAnimated(ItemPtr item, Direction direction);
	void shoot();
	void eat();
	void dropItems();
	void pickUp();
	void showInventory();
	void reloadWeapon();
	void readScroll();
	void drinkPotion();
	void throwItem();
	void wieldWeapon();
	void wearArmor();

	enum SelectStatus {
	    NothingToSelect,
	    Cancelled,
	    Success
	};

	std::pair<SelectStatus, char> selectOneFromInventory(
	        std::string_view title,
	        std::function<bool(const Item &)> filter = [] (const Item &) { return true; }) const;

	std::pair<SelectStatus, int> selectOneFromList(std::string_view title, const std::vector<const Item *> & items) const;

    std::pair<SelectStatus, std::vector<char>> selectMultipleFromInventory(
            std::string_view title,
            std::function<bool(const Item &)> filter = [] (const Item &) { return true; }) const;

    std::pair<SelectStatus, std::vector<int>> selectMultipleFromList(std::string_view title, const std::vector<const Item *> & items) const;

	void moveTo(Coord2i cell);

    void levelUp();

    Array2D<bool, LEVEL_ROWS, LEVEL_COLS> seenMap;
};

extern Hero heroTemplate;

#endif // HERO_HPP

