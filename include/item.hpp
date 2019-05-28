#ifndef ITEM_HPP
#define ITEM_HPP

#include<vector>
#include<string>
#include<optional>
#include<list>
#include<memory>
#include<functional>

#include<level.hpp>
#include<array2d.hpp>
#include<termlib/vec2.hpp>

class Item {
public:
    using Ptr = std::unique_ptr<Item>;

    enum class Type {
        Food,
        Armor,
        Weapon,
        Ammo,
        Scroll,
        Potion
    };

	Item();
	virtual ~Item();

    Coord2i pos;
	int symbol;
	char inventorySymbol;
	int weight;
	int mdf;
	int count;
	bool showMdf;
	bool isStackable;

    // toSplit:
    //  - [1, count) - splits on 2 piles, returns one with count = toSplit
    //  - count - returns whole pile, this pile becomes invalid
    Ptr splitStack(int toSplit);
    std::string getMdf() const;
    std::string getName() const;
    int getSingleWeight() const;
    int getTotalWeight() const;
    virtual Type getType() const = 0;
    virtual Item::Ptr clone() const = 0;
};

class Food: public Item {
public:	
    static const int TYPES_COUNT = 2;
    static const int COUNT = 10;

	Food(int FoodType);
	
	int nutritionalValue;
	bool isRotten;

	Food();
	~Food();

    Type getType() const override {
        return Type::Food;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Food>(*this);
    }
};

class Armor: public Item {
public:
    static const int TYPES_COUNT = 2;
    static const int COUNT = 4;

	Armor(int ArmorType);

	int defence;
	int durability;

	Armor();
	~Armor();

    Type getType() const override {
        return Type::Armor;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Armor>(*this);
    }
};

class Ammo: public Item {
public:
    using Ptr = std::unique_ptr<Ammo>;
    static const int TYPES_COUNT = 2;
    static const int COUNT = 25;

	Ammo(int AmmoType);

	int range;
	int damage;

	Ammo();
	~Ammo();

    Type getType() const override {
        return Type::Ammo;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Ammo>(*this);
    }
};

class Weapon: public Item {
public:
    using Ptr = std::unique_ptr<Weapon>;

private:
    class Cartridge {
        std::vector<Ammo::Ptr> loaded;
        int capacity = 0;

    public:
        Cartridge(int capacity = 0);
        Cartridge(const Cartridge &);
        Cartridge & operator =(const Cartridge &); 

        // returns the bullet if fails to load it
        Ammo::Ptr load(Ammo::Ptr bullet);

        Ammo::Ptr unloadOne();

        const Ammo & next() const;
        Ammo & next();

        const Ammo * operator [](int ind) const;

        auto begin() const -> decltype(loaded.begin());
        auto end() const -> decltype(loaded.end());

        int getCapacity() const;
        int getCurrSize() const;
        bool isEmpty() const;
        bool isFull() const;
    };

public:
    static const int TYPES_COUNT = 6;
    static const int COUNT = 25; /* JUST FOR !DEBUG!!*/

	Weapon(int WeaponType);
	
    Cartridge cartridge;
	int damage;
	int range; 									// Ranged bullets have additional effect on this paramether
	int damageBonus;								// And on this too
	bool isRanged = false;
    bool canDig = false;

	Weapon();
	Weapon(const Weapon&) = default;
	Weapon& operator=(const Weapon&) = default;
	~Weapon();

    Type getType() const override {
        return Type::Weapon;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Weapon>(*this);
    }
};

class Scroll: public Item {
public:
    static const int TYPES_COUNT = 2;
    static const int COUNT = 15; /* JUST FOR !DEBUG!!*/

	Scroll();
	Scroll(int s);
	~Scroll();

	int effect;

    Type getType() const override {
        return Type::Scroll;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Scroll>(*this);
    }
};

class Potion: public Item {
public:
    static const int TYPES_COUNT = 5;
    static const int COUNT = 25; /* IT TOO */

	Potion();
	Potion(int p);
	~Potion();

    std::string getPotionName() const;

	int effect;

    Type getType() const override {
        return Type::Potion;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Potion>(*this);
    }
};

std::string getPotionName(int sym);

/*
class Tools: public Item {
public:
    static const int TYPES_COUNT = 1;
    static const int COUNT = 5;

	Tools();
	Tools(int t);
	~Tools();

	int possibility;
	int uses;
	int damage;									// It is nedlectful to use weapon's attributes on tools
	int range; 									// Ranged bullets have add effect on this paramether
	bool isRanged;
	int cooldown;									// The end of using attributes


    ItemType getType() const override {
        return ItemTools;
    }

    Item::Ptr clone() const override {
        return std::make_unique<Tools>(*this);
    }
};*/

using ItemPile = std::list<std::unique_ptr<Item>>;
using ItemPileIter = ItemPile::iterator;

extern Array2D<ItemPile, LEVEL_ROWS, LEVEL_COLS> itemsMap;

extern std::vector<Food> foodTypes;
extern std::vector<Armor> armorTypes;
extern std::vector<Weapon> weaponTypes;
extern std::vector<Ammo> ammoTypes;
extern std::vector<Scroll> scrollTypes;
extern std::vector<Potion> potionTypes;
extern std::vector<bool> potionTypeKnown;

ItemPileIter findItemAt(Coord2i cell, int sym);
bool randomlySetOnMap(Item::Ptr item);

template<class ItemType>
ItemType selectOne(const std::vector<ItemType> & types) {
    return types[std::rand() % types.size()];
}

template<class ItemType>
using ItemSelector = std::function<ItemType(const std::vector<ItemType> &)>;

template<class ItemType>
void randomlySelectAndSetOnMap(const std::vector<ItemType> & types, int n, const ItemSelector<ItemType> & selector = selectOne<ItemType>) {
    for (int i = 0; i < n; ++i) {
        Item::Ptr item = std::make_unique<ItemType>(selector(types));
        randomlySetOnMap(std::move(item));
    }
}

void drop(Item::Ptr item, Coord2i to);

#endif // ITEM_HPP

