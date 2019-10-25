#ifndef ITEM_HPP
#define ITEM_HPP

#include<level.hpp>
#include<array2d.hpp>
#include<enable_clone.hpp>

#include<termlib/vec2.hpp>

#include<effolkronium/random.hpp>

#include<vector>
#include<string>
#include<optional>
#include<list>
#include<memory>
#include<functional>
#include<unordered_map>

using Random = effolkronium::random_static;

class Item;
using ItemPtr = std::unique_ptr<Item>;

class Item {
public:
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
	std::string id;
	std::string name;
	char inventorySymbol;
	int weight;
	int mdf;
	int count;
	bool showMdf;
	bool isStackable;

    // toSplit:
    //  - [1, count) - splits on 2 piles, returns one with count = toSplit
    //  - count - returns whole pile, this pile becomes invalid
    ItemPtr splitStack(int toSplit);
    std::string getMdf() const;
    virtual std::string getName() const;
    int getSingleWeight() const;
    int getTotalWeight() const;
    virtual Type getType() const = 0;
    virtual ItemPtr cloneItem() const = 0;

    static ItemPtr getByID(const std::string & id);
};

class Food;
using FoodPtr = std::unique_ptr<Food>;

class Food
    : public Item
    , public EnableClone<Food>
{
public:	
    static const int COUNT = 10;

	int nutritionalValue;
	bool isRotten;

	Food();
	~Food();

    Type getType() const override {
        return Type::Food;
    }

    ItemPtr cloneItem() const override {
        return std::make_unique<Food>(*this);
    }
};

class Armor;
using ArmorPtr = std::unique_ptr<Armor>;

class Armor
    : public Item
    , public EnableClone<Armor>
{
public:
    static const int COUNT = 4;

	int defence;
	int durability;

	Armor();
	~Armor();

    Type getType() const override {
        return Type::Armor;
    }

    ItemPtr cloneItem() const override {
        return std::make_unique<Armor>(*this);
    }
};

class Ammo;
using AmmoPtr = std::unique_ptr<Ammo>;

class Ammo
    : public Item
    , public EnableClone<Ammo>
{
public:
    static const int COUNT = 25;

	int range;
	int damage;

	Ammo();
	~Ammo();

    Type getType() const override {
        return Type::Ammo;
    }

    ItemPtr cloneItem() const override {
        return std::make_unique<Ammo>(*this);
    }
};

class Weapon;
using WeaponPtr = std::unique_ptr<Weapon>;

class Weapon
    : public Item
    , public EnableClone<Weapon>
{
public:
    class Cartridge {
        std::vector<AmmoPtr> loaded;
        int capacity = 0;

    public:
		explicit Cartridge(int capacity = 0);
        Cartridge(const Cartridge &);
        Cartridge & operator =(const Cartridge &); 

        // returns the bullet if fails to load it
        AmmoPtr load(AmmoPtr bullet);

        AmmoPtr unloadOne();

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

    static const int COUNT = 25; /* JUST FOR !DEBUG!!*/

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

    ItemPtr cloneItem() const override {
        return std::make_unique<Weapon>(*this);
    }
};

class Scroll;
using ScrollPtr = std::unique_ptr<Scroll>;

class Scroll
    : public Item
    , public EnableClone<Scroll>
{
public:
    static const int COUNT = 15; /* JUST FOR !DEBUG!!*/

    enum Effect {
    	Map,
    	Identify,
    	EffectCount
    };

	Scroll();
	~Scroll();

	Effect effect;

    Type getType() const override {
        return Type::Scroll;
    }

    ItemPtr cloneItem() const override {
        return std::make_unique<Scroll>(*this);
    }
};

class Potion;
using PotionPtr = std::unique_ptr<Potion>;

class Potion
    : public Item
    , public EnableClone<Potion>
{
public:
    static const int COUNT = 25; /* IT TOO */

    enum Effect {
    	None,
    	Heal,
    	Invisibility,
    	Teleport,
    	Blindness,
    	EffectCount
    };

	Potion();
	~Potion();

	Effect effect;

    std::string getName() const override;

    Type getType() const override {
        return Type::Potion;
    }

    ItemPtr cloneItem() const override {
        return std::make_unique<Potion>(*this);
    }
};

#endif // ITEM_HPP

