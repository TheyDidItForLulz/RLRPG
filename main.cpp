//////////////////////////////////////////////////////////////////////////////////////// Symbols //////////////////////////////////////////////////////////////////////////////////////////
/*
											'.'(Floor) == 1
											'#'(Wall) == 2
											'%'(Egg) == 100
											'%'(Apple) == 101
											'@'(Hero) == 200	
											'&'(Leather armor) == 301
											'&'(Chain armor) == 300
											'/'(Copper shortsword) == 400
											'/'(Bronze spear) == 401
											'/'(Musket) == 402
											','(Steel bullets) == 450
											'^'(Pile) == 3
*/
//////////////////////////////////////////////////////////////////////////////////////// Modificators /////////////////////////////////////////////////////////////////////////////////////
/*
											100 - Notning
											101..199 - Food mods
											200..299 - Armor mods
											300..399 - Weapon mods
											101 - Poisoned
											102 - Rotten
*/
//////////////////////////////////////////////////////////////////////////////////////// Attributes ///////////////////////////////////////////////////////////////////////////////////////
/*
											100 - Notning
											101..199 - Food attributes
											200..299 - Armor attributes
											300..399 - Weapon attributes
										   	201 - Worn
											301 - Wielded
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//!COMMENT! // Also it isn't needed to show to the player his satiation. And luck too.

#include<stdio.h>													//
#include<iostream>													//
#include<stdlib.h>													//
#include<time.h>													//
#include<ncurses.h>													//
#include<string.h>													//

#define CONTROL_UP 'k'
#define CONTROL_DOWN 'j'
#define CONTROL_LEFT 'h'
#define CONTROL_RIGHT 'l'
#define DEFAULT_HERO_HEALTH 10												//
#define TypesOfFood 2													//
#define TypesOfArmor 2													//
#define TypesOfWeapon 3													//
#define TypesOfAmmo 1
#define TypesOfEnemies 1
#define BLACK_BLACK 1
#define	RED_BLACK 2
#define GREEN_BLACK 3
#define YELLOW_BLACK 4
#define BLUE_BLACK 5
#define MAGENTA_BLACK 6
#define CYAN_BLACK 7
#define WHITE_BLACK 8
#define BLACK_WHITE 9
#define LIGHT A_BOLD 
#define DELAY 0.03
#define MaxInvVol 53													//
#define TrueMaxInvVol 55
#define AMMO_SLOT 53
#define EMPTY_SLOT 54
#define FOODCOUNT 10													//
#define ARMORCOUNT 4													//
#define WEAPONCOUNT 15													//
#define AMMOCOUNT 15
#define ENEMIESCOUNT 1
#define Depth 20													//
#define VISION 7													//
int MaxInvItemsWeight = 25;												//
// !COMMENT! // Level-up and items stacking
int MODE;														//
bool StopUpdating = false;												//
															//
using namespace std;													//

#define COLS 40														//
#define ROWS 10														//
#define FIELD_COLS ( COLS * 2 ) + 1											//
#define FIELD_ROWS ( ROWS * 2 ) + 1											//
															//
#define Length FIELD_COLS												//
#define Height FIELD_ROWS												//
															//
int map[ FIELD_ROWS ][ FIELD_COLS ];											//
bool seenUpdated[FIELD_ROWS][FIELD_COLS];										// <- visible array
int used[ ROWS ][ COLS ];												//
int active = 0;														//
															//
struct Pair														//
{															//
	int x;														//
	int y;														//
};															//
															//
void init_field( void )													//
{															//
	for( int i = 0; i < FIELD_ROWS; ++i )										//
	{														//
		for( int j = 0; j < FIELD_COLS; ++j )									//
		{													//
			map[ i ][ j ] = 1;										//
		}													//
	}														//
}															//
															//
void initialize( void )													//
{															//
	srand( time( 0 ) );												//
	active = 1;													//
	init_field();													//
}															//
															//
void maze_next( Pair* start, Pair* prev, Pair* curr )									//
{															//
	if( curr->x == start->x && curr->y == start->y && used[ start->y ][ start->x ])					//
	{														//
		return;													//
	}														//
															// Here's some black magic. 
															// Do NOT touch it, if you want not to shoot your leg.
	used[ curr->y ][ curr->x ] = 1;											//
	Pair f = { curr->x * 2 + 1, curr->y * 2 + 1 };									//
	for( int i = f.y - 1; i <= f.y + 1; ++i )									//
	{														//
		for( int j = f.x - 1; j <= f.x + 1; ++j )								//
		{													//
			map[ i ][ j ] = 2;										//
		}													//
	}														//
	map[ f.y ][ f.x ] = map[ f.y + prev->y - curr->y ][ f.x + prev->x - curr->x ] = 1;				//
															//
	Pair stack[ 4 ];												//
	int sp;														//
															//
	do														//
	{														//
		sp = 0;													//
		if( curr->x && !used[ curr->y ][ curr->x - 1 ] )							//
		{													//
			stack[ sp ].x = curr->x - 1;									//
			stack[ sp++ ].y = curr->y;									//
		}													//
		if( curr->x + 1 < COLS && !used[ curr->y ][ curr->x + 1 ] )						//
		{													//
			stack[ sp ].x = curr->x + 1;									//
			stack[ sp++ ].y = curr->y;									//
		}													//
		if( curr->y && !used[ curr->y - 1 ][ curr->x ] )							//
		{													//
			stack[ sp ].x = curr->x;									//
			stack[ sp++ ].y = curr->y - 1;									//
		}													//
		if( curr->y + 1 < ROWS && !used[ curr->y + 1 ][ curr->x ] )						//
		{													//
			stack[ sp ].x = curr->x;									//
			stack[ sp++ ].y = curr->y + 1;									//
		}													//
		if( !sp )												//
		{													//
			return;												//
		}													//
		int rand_i = rand() % sp;										//
		maze_next( start, curr, stack + rand_i );								//
	}														//
	while( 1 );													//
}															//
															//
void generate_maze( void )												//
{															//
	Pair point = { 0, 0 };												//
	maze_next( &point, &point, &point );										//
}															//
															//
string message = "";
string bar = "";
char tmp[100];

void MoveUnit(int, int&, int&, int, int);

class Item
{

public:
		
	Item(): mdf(100), showMdf(false), attribute(100){};

	int posH;
	int posL;
	int symbol;
	int symUnder;
	char inventorySymbol;
	int weight;
	int mdf;
	int attribute;
	bool showMdf;

	const char* GetMdf()
	{
		switch(mdf)
		{
			case 100:
				return "Nothing";
			case 101: 
				return "Poisoned";
			case 102:
				return "Rotten";
		}
	}
	const char* GetAttribute()
	{
		switch(attribute)
		{
			case 100:
				return "Nothing";
			case 201:
				return "Being worn";
			case 301:
				return "Wielded";
		}
	}
	const char* GetName()
	{
		switch(symbol)
		{
			case 100:
				return "Egg";
			case 101:
				return "Apple";
			case 300:
				return "Chain chestplate";
			case 301:
				return "Leather chestplate";
			case 400:
				return "Copper shortsword";
			case 401:
				return "Bronze spear";
			case 402:
				return "Musket";
			case 450:
				return "Steel bullets";
		}
	}
	~Item(){};

};

class EmptyItem: public Item
{
public:
	EmptyItem(){};
	~EmptyItem(){};
};

class Food: public Item
{
public:	
	Food(int FoodType)
	{
		switch(FoodType)
		{
			case 0:
				symbol = 100;
				FoodHeal = 100;
				weight = 1;
				break;
			case 1:
				symbol = 101;
				FoodHeal = 125;
				weight = 1;
				break;
		}
		symUnder = 1;
	};
	
	int FoodHeal;
	bool isRotten;

	Food(): isRotten(false){};
	~Food(){};

};

class Armor: public Item
{
public:
	Armor(int ArmorType)
	{
		switch(ArmorType)
		{
			case 0:
				symbol = 300;
				defence = 20;
				durability = 20;
				weight = 20;
				break;
			case 1:
				symbol = 301;
				defence = 10;
				durability = 15;
				weight = 7;
				break;
		}
		symUnder = 1;
	}

	int defence;
	int durability;

	Armor(){};
	~Armor(){};

};

class Weapon: public Item
{
public:
	Weapon(int WeaponType)
	{
		switch(WeaponType)
		{
			case 0:
				symbol = 400;
				damage = 2;
				weight = 3;
				range = 1;
				Ranged = false;
				break;
			case 1:
				symbol = 401;
				damage = 3;
				weight = 5;
				range = 1;
				Ranged = false;
				break;
			case 2:
				symbol = 402;
				damage = 1;
				weight = 3;
				cooldown = 1;
				range = 2;
				Ranged = true;
				break;
		}
		symUnder = 1;
	};
	
	int damage;

	int range; 									// Ranged bullets have add effect on this paramether
	bool Ranged;
	int cooldown;

	Weapon(){};
	~Weapon(){};
};

class Ammo: public Item
{
public:
	Ammo(int AmmoType)
	{
		switch(AmmoType)
		{
			case 0:
				symbol = 450;
				weight = 1;
				range = 5;
				damage = 5;
				count = 1;
				break;
		}
	};

	int range;
	int damage;
	int count;

	Ammo(){};
	~Ammo(){};
};

enum ItemType
{
	ItemFood,
	ItemArmor,
	ItemEmpty,
	ItemWeapon,
	ItemAmmo
};

union InventoryItem
{
	EmptyItem invEmpty;
	Food invFood;
	Armor invArmor;
	Weapon invWeapon;
	Ammo invAmmo;
	InventoryItem(EmptyItem e)
	{
		invEmpty = e;
	}
	InventoryItem(Food f)
	{
		invFood = f;
	}
	InventoryItem(Armor a)
	{
		invArmor = a;
	}
	InventoryItem(Weapon w)
	{
		invWeapon = w;
	}
	InventoryItem(Ammo am)
	{
		invAmmo = am;
	}
	InventoryItem(InventoryItem& i){}
	InventoryItem()
	{
		invEmpty = EmptyItem();
	}
	~InventoryItem(){}
};

struct PossibleItem
{
	InventoryItem item;
	ItemType type;
	PossibleItem(InventoryItem i, ItemType t): item(i), type(t)
	{}
	PossibleItem(){type = ItemEmpty;}
	void operator=(const Food& f)
	{
		type = ItemFood;
		item.invFood = f;
	}
	void operator=(const Armor& a)
	{
		type = ItemArmor;
		item.invArmor = a;
	}
	void operator=(const EmptyItem& e)
	{
		type = ItemEmpty;
		item.invEmpty = e;
	}
	void operator=(const Weapon& w)
	{
		type = ItemWeapon;
		item.invWeapon = w;
	}
	void operator=(const Ammo& am)
	{
		type = ItemAmmo;
		item.invAmmo = am;
	}
	Item& GetItem()
	{
		switch( type )
		{
			case ItemFood:
				return item.invFood;
			case ItemArmor:
				return item.invArmor;
			case ItemEmpty:
				return item.invEmpty;
			case ItemWeapon:
				return item.invWeapon;
			case ItemAmmo:
				return item.invAmmo;
		}
	}
};

PossibleItem ItemsMap[FIELD_ROWS][FIELD_COLS][Depth];

PossibleItem inventory[TrueMaxInvVol];
int inventoryVol = 0;

Food differentFood[TypesOfFood];

Armor differentArmor[TypesOfArmor];

Weapon differentWeapon[TypesOfWeapon];

Ammo differentAmmo[TypesOfAmmo];

class Unit
{
public:
	Unit(): inventoryVol(0){};
	int health;
	PossibleItem unitInventory[4];
	int inventoryVol;
	PossibleItem* unitWeapon;
	PossibleItem* unitArmor;
	int posH;
	int posL;
	int symbol;
	int symUnder;
	
	const char* GetName()
	{
		switch(symbol)
		{
			case 200:
				return "Hero";
			case 201:
				return "Dummy";
		}
	}
	~Unit(){};
};

class EmptyUnit: public Unit
{
public:
	EmptyUnit(){};
	~EmptyUnit(){};
};

class Enemy: public Unit
{
public:
	Enemy(int eType)
	{
		switch(eType)
		{
			case 0:
				health = 12;
				unitInventory[0] = differentFood[0];
				inventoryVol = 1;
				symbol = 201;
				vision = 5;
				dist = 0;
		}
	}
	Enemy(const Enemy& en): vision(en.vision), dist(en.dist), dir(en.dir)
	{
	
		unitWeapon = en.unitWeapon;
		unitArmor = en.unitArmor;
		posH = en.posH;
		posL = en.posL;
		health = en.health;
		inventoryVol = en.inventoryVol;
		symbol = en.symbol;
		for(int i = 0; i < inventoryVol; i++)
		{
			unitInventory[i] = en.unitInventory[i];
		}
	}
	int vision;
	int dir;
	int dist;

	Enemy(){}
	~Enemy(){}
};

int Luck;

class Hero: public Unit
{
public:
	Hero(): isBurdened(false), CanHeroMoveThroughWalls(false) {};

	int hunger;
	PossibleItem* heroArmor;
	PossibleItem* heroWeapon;
	bool isBurdened;
	bool CanHeroMoveThroughWalls;
	
	void findVisibleArray()
	{

		int dirH = posH, dirL = posL;

		while(1)
		{
			seenUpdated[dirH][posL] = 1;
			seenUpdated[dirH][posL + 1] = 1;
			seenUpdated[dirH][posL - 1] = 1;

			if(map[dirH][posL] != 2 && abs(dirH - posH) < VISION)
			{
				dirH++;
			}
			else
			{
				dirH = posH;
				break;
			}
		}
		while(1)
		{
			seenUpdated[dirH][posL] = 1;
			seenUpdated[dirH][posL + 1] = 1;
			seenUpdated[dirH][posL - 1] = 1;

			if(map[dirH][posL] != 2 && abs(dirH - posH) < VISION)
			{
				dirH--;
			}
			else
			{
				dirH = posH;
				break;
			}
		}
		while(1)
		{	
			seenUpdated[posH][dirL] = 1;
			seenUpdated[posH + 1][dirL] = 1;
			seenUpdated[posH - 1][dirL] = 1;

			if(map[posH][dirL] != 2 && abs(dirL - posL) < VISION)
			{
				dirL++;
			}
			else
			{
				dirL = posL;
				break;
			}
		}
		while(1)
		{	
			seenUpdated[posH][dirL] = 1;
			seenUpdated[posH + 1][dirL] = 1;
			seenUpdated[posH - 1][dirL] = 1;

			if(map[posH][dirL] != 2 && abs(dirL - posL) < VISION)
			{
				dirL--;
			}
			else
			{
				dirL = posL;
				break;
			}

		}

	}

	void AttackEnemy(int& a1, int& a2);

	void mHLogic(int& a1, int& a2);
	
	bool isInventoryEmpty(){
	
		for(int i = 0; i < MaxInvVol; i++){

			if(inventory[i].type != ItemEmpty) return false;

		}
		return true;
	
	}

	int FindEmptyInventoryCell()
	{
		
		for(int i = 0; i < MaxInvVol; i++){

			if(inventory[i].type == ItemEmpty) return i;

		}

		return 101010;											// Magic constant, means "Inventory is full".

	}	
	
	int GetInventoryItemsWeight()
	{

		int toReturn = 0;
	
		for(int i = 0; i < MaxInvVol; i++){
			
			if(inventory[i].type != ItemEmpty){

				toReturn += inventory[i].GetItem().weight;

			}
		
		}
		
//		sprintf(tmp, "!W: %i!", toReturn);								// !DEBUG!
//		message += tmp;											//

		return toReturn;
	
	}

	void PrintList(PossibleItem items[], int len, char msg[], int mode)
	{
		int num = 0;

		move(num, Length + 10);
		printw("%s", msg);	
		num ++;
		switch(mode)
		{
			case 1:
			{
				for(int i = 0; i < len; i++)
				{
					move(num, Length + 10);
					if(items[i].GetItem().showMdf == true)
					{
						printw("[%c] %s (%s) {%s}. ", items[i].GetItem().inventorySymbol, items[i].GetItem().GetName(), items[i].GetItem().GetAttribute(), items[i].GetItem().GetMdf());
					}
					else printw("[%c] %s (%s). ", items[i].GetItem().inventorySymbol, items[i].GetItem().GetName(), items[i].GetItem().GetAttribute());
					num ++;
				}
				break;
			}
			case 2:
			{
				for(int i = 0; i < len; i++)
				{
					move(num, Length + 10);
					if(items[i].GetItem().showMdf == true)
					{
						printw("[%c] %s (%s) {%s}. ", i + 'a', items[i].GetItem().GetName(), items[i].GetItem().GetAttribute(), items[i].GetItem().GetMdf());
					}
					else printw("[%c] %s (%s). ", i + 'a', items[i].GetItem().GetName(), items[i].GetItem().GetAttribute());
					num ++;
				}
				break;

			}
		}
	}
	
	int FindElementsNumberUnderThisCell(int h, int l)
	{
		int result = 0;
		for(int i = 0; i < Depth; i++)
		{
			if(ItemsMap[h][l][i].type != ItemEmpty)
			{
				result++;
			}
		}
		return result;
	}

	int FindEmptyElementUnderThisCell(int h, int l)
	{
		for(int i = 0; i < Depth; i++)
		{
			if(ItemsMap[h][l][i].type == ItemEmpty)
			{
				return i;
			}
		}
		return 101010;											// Magic constant. Means, that something went wrong.
	}

	int FindNotEmptyElementUnderThisCell(int h, int l)
	{
		for(int i = 0; i < Depth; i++)
		{
			if(ItemsMap[h][l][i].type != ItemEmpty)
			{
				return i;
			}
		}
		return 101010;
	}

	void PickUp(){
		
		if(FindElementsNumberUnderThisCell(posH, posL) == 0)
		{
			message += "There is nothing here to pick up. ";
			return;
		}
		else if(FindElementsNumberUnderThisCell(posH, posL) == 1)
		{
			int num = FindNotEmptyElementUnderThisCell(posH, posL);

			if(ItemsMap[posH][posL][num].type == ItemAmmo)
			{
				if(inventory[AMMO_SLOT].type != ItemEmpty)
				{
					if(ItemsMap[posH][posL][num].GetItem().symbol == inventory[AMMO_SLOT].GetItem().symbol)
					{
						inventory[AMMO_SLOT].item.invAmmo.count += ItemsMap[posH][posL][num].item.invAmmo.count;
						ItemsMap[posH][posL][num].type = ItemEmpty;
					}
					else
					{
						PossibleItem buffer;
						buffer = ItemsMap[posH][posL][num];
						ItemsMap[posH][posL][num] = inventory[AMMO_SLOT];
						inventory[AMMO_SLOT] = buffer;
					}
				}
				else
				{
					inventory[AMMO_SLOT] = ItemsMap[posH][posL][num];
					ItemsMap[posH][posL][num].type = ItemEmpty;
				}
				return;
			}

			sprintf(tmp, "You picked up %s. ", ItemsMap[posH][posL][num].GetItem().GetName());
			message += tmp;

			int eic = FindEmptyInventoryCell();
			if(eic != 101010)
			{
				inventory[eic] = ItemsMap[posH][posL][num];
				inventory[eic].GetItem().inventorySymbol = eic + 'a';
				ItemsMap[posH][posL][num].type = ItemEmpty;
				inventoryVol++;
			}
			else
			{
				message += "Your inventory is full, motherfuck'a! ";
			}

			if(GetInventoryItemsWeight() > MaxInvItemsWeight && !isBurdened)
			{
				message += "You're burdened. ";
				isBurdened = true;
			}

			return;
		}
		
		PossibleItem list[Depth];
		char hv[200] = "What do you want to pick up? ";
		int len = 0;

		for(int i = 0; i < Depth; i++)
		{	
			if(ItemsMap[posH][posL][i].type != ItemEmpty)
			{
				list[len] = ItemsMap[posH][posL][i];
				len++;
			}
		}

		PrintList(list, len, hv, 2);
		len = 0;

		char choise = getch();

		if(choise == '\033') return;

		int intch = choise - 'a';
		
		int helpfulArray[Depth], hACounter = 0;

		for(int i = 0; i < Depth; i++)
		{
			if(ItemsMap[posH][posL][i].type != ItemEmpty)
			{
				helpfulArray[hACounter] = i;
				hACounter++;
			}
		}

		if(ItemsMap[posH][posL][helpfulArray[intch]].type != ItemEmpty)
		{
			sprintf(tmp, "You picked up %s. ", ItemsMap[posH][posL][helpfulArray[intch]].GetItem().GetName());
			message += tmp;
			
			if(ItemsMap[posH][posL][helpfulArray[intch]].type == ItemAmmo)
			{
				if(inventory[AMMO_SLOT].type != ItemEmpty)
				{
					if(ItemsMap[posH][posL][helpfulArray[intch]].GetItem().symbol == inventory[AMMO_SLOT].GetItem().symbol)
					{
						inventory[AMMO_SLOT].item.invAmmo.count += ItemsMap[posH][posL][helpfulArray[intch]].item.invAmmo.count;
						ItemsMap[posH][posL][helpfulArray[intch]].type = ItemEmpty;
					}
					else
					{
						PossibleItem buffer;
						buffer = ItemsMap[posH][posL][helpfulArray[intch]];
						ItemsMap[posH][posL][helpfulArray[intch]] = inventory[AMMO_SLOT];
						inventory[AMMO_SLOT] = buffer;
					}
				}
				else
				{
					inventory[AMMO_SLOT] = ItemsMap[posH][posL][helpfulArray[intch]];
					ItemsMap[posH][posL][helpfulArray[intch]].type = ItemEmpty;
				}
				return;
			}

			int eic = FindEmptyInventoryCell();
			if(eic != 101010)
			{
				inventory[eic] = ItemsMap[posH][posL][helpfulArray[intch]];
				inventory[eic].GetItem().inventorySymbol = eic + 'a';
				ItemsMap[posH][posL][helpfulArray[intch]].type = ItemEmpty;
				inventoryVol++;
			}
			else
			{
				message += "Your inventory is full, motherfuck'a! ";
			}
		}
	
		if(GetInventoryItemsWeight() > MaxInvItemsWeight && !isBurdened)
		{
			message += "You're burdened. ";
			isBurdened = true;
		}
	}

	bool isFoodInInventory()
	{
		for(int i = 0; i < MaxInvVol; i++)
		{
			if(inventory[i].type == ItemFood) return true;
		}
		return false;
	}
	
	bool isArmorInInventory()
	{
		for(int i = 0; i < MaxInvVol; i++)
		{
			if(inventory[i].type == ItemArmor) return true;
		}
		return false;
	}

	bool isWeaponInInventory()
	{
		for(int i = 0; i < MaxInvVol; i++)
		{
			if(inventory[i].type == ItemWeapon) return true;
		}
		return false;
	}
	
	void ClearRightPane()
	{
		for(int i = 0; i < 100; i++)
		{
			for(int j = 0; j < 50; j++)
			{
				move(i, Length + 10 + j);
				addch(' ');
			}
		}
	}

	void Delay(double s)
	{
		clock_t clocksNow = clock();
		while(double(clock() - clocksNow) / CLOCKS_PER_SEC < s);
	}

	void ThrowAnimated(PossibleItem& item, char direction)
	{
		int ThrowLength = 0;

		char sym = '-';

		switch(direction)
		{
			case CONTROL_RIGHT:
			{
				for(int i = 0; i < VISION; i++)
				{
					if(map[posH][posL + i + 1] == 2) break;
					move(posH, posL + i + 1);
					addch(sym);
					refresh();
					ThrowLength++;
					Delay(DELAY);
				}
				int empty = FindEmptyElementUnderThisCell(posH, posL + ThrowLength);
				if(empty == 101010)
				{
					int empty2 = FindEmptyElementUnderThisCell(posH, posL + ThrowLength - 1);
					ItemsMap[posH][posL + ThrowLength - 1][empty2] = item;
					item.type = ItemEmpty;
				}
				else
				{
					ItemsMap[posH][posL + ThrowLength][empty] = item;
					item.type = ItemEmpty;
				}
				break;
			}
			case CONTROL_LEFT:
			{
				for(int i = 0; i < VISION; i++)
				{
					if(map[posH][posL - i - 1] == 2) break;
					move(posH, posL - i - 1);
					addch(sym);
					refresh();
					ThrowLength++;
					Delay(DELAY);
				}
				int empty = FindEmptyElementUnderThisCell(posH, posL - ThrowLength);
				if(empty == 101010)
				{
					int empty2 = FindEmptyElementUnderThisCell(posH, posL - ThrowLength + 1);
					ItemsMap[posH][posL - ThrowLength + 1][empty2] = item;
					item.type = ItemEmpty;
				}
				else
				{
					ItemsMap[posH][posL - ThrowLength][empty] = item;
					item.type = ItemEmpty;
				}
				break;
			}
			case CONTROL_UP:
			{
				for(int i = 0; i < VISION; i++)
				{
					if(map[posH - i - 1][posL] == 2) break;
					move(posH - i - 1, posL);
					addch(sym);
					refresh();
					ThrowLength++;
					Delay(DELAY);
				}
				int empty = FindEmptyElementUnderThisCell(posH - ThrowLength, posL);
				if(empty == 101010)
				{
					int empty2 = FindEmptyElementUnderThisCell(posH - ThrowLength + 1, posL);
					ItemsMap[posH - ThrowLength + 1][posL][empty2] = item;
					item.type = ItemEmpty;
				}
				else
				{
					ItemsMap[posH - ThrowLength][posL][empty] = item;
					item.type = ItemEmpty;
				}
				break;
			}
			case CONTROL_DOWN:
			{
				for(int i = 0; i < VISION; i++)
				{
					if(map[posH + i + 1][posL] == 2) break;
					move(posH + i + 1, posL);
					addch(sym);
					refresh();
					ThrowLength++;
					Delay(DELAY);
				}
				int empty = FindEmptyElementUnderThisCell(posH + ThrowLength, posL);
				if(empty == 101010)
				{
					int empty2 = FindEmptyElementUnderThisCell(posH + ThrowLength - 1, posL);
					ItemsMap[posH + ThrowLength - 1][posL][empty2] = item;
					item.type = ItemEmpty;
				}
				else
				{
					ItemsMap[posH + ThrowLength][posL][empty] = item;
					item.type = ItemEmpty;
				}
				break;
			}
		}
	}

	void Shoot();
	
	void ShowInventory(const char& inp){
		
		PossibleItem list[MaxInvVol];

		int len = 0;

		switch(inp){
			
			case 'i':
			{
				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type != ItemEmpty)
					{
						list[len] = inventory[i];
						len++;
					}
				}
				
				char hv[200] = "Here is your inventory.";

				PrintList(list, len, hv, 1);
					
				char choise = getch();
		
				if(choise == '\033') return;

				len = 0;

				break;
			}

			case 'e':
			{
				
				char hv[200] = "What do you want to eat?";

				for(int i = 0; i < MaxInvVol; i++){
				
					if(inventory[i].type == ItemFood)
					{
						list[len] = inventory[i];
						len++;
					}
		
				}
				
				PrintList(list, len, hv, 1);

				len = 0;

				char choise = getch();

				if(choise == '\033') return;
				
				int intch = choise - 'a';

				if(inventory[intch].type == ItemFood){
					
					int prob = rand() % Luck;
						
					if(prob == 0)
					{
						hunger += inventory[intch].item.invFood.FoodHeal / 3;
						health --;
						inventory[intch].type = ItemEmpty;
						message += "Fuck! This food was rotten! ";
					}
					else
					{
						hunger += inventory[intch].item.invFood.FoodHeal;
						inventory[intch].type = ItemEmpty;
					}
				
				}
				
				break;
			}	
				
			case 'W':
			{
				
				char hv[200] = "What do you want to wear?";
				
				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type == ItemArmor)
					{
						list[len] = inventory[i];
						len++;
					}
				}
				PrintList(list, len, hv, 1);
				len = 0;

				char choise = getch();

				if(choise == '\033') return;
	
				int intch = choise - 'a';

				if(inventory[intch].type == ItemArmor)
				{
					sprintf(tmp, "Now you wearing %s. ", inventory[intch].GetItem().GetName());
					message += tmp;

					if(heroArmor->type != ItemEmpty)
					{
						heroArmor->GetItem().attribute = 100;
					}
					heroArmor = &inventory[intch];
					inventory[intch].GetItem().attribute = 201;
				}
	
				break;

			}

			case 'd':
			{
				char hv[200] = "What do you want to drop?";

				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type != ItemEmpty)
					{
						list[len] = inventory[i];
						len++;
					}
				}

				PrintList(list, len, hv, 1);
				len = 0;

				char choise = getch();

				if(choise == '\033') return;
				
				int intch = choise - 'a';
				
				int num = FindEmptyElementUnderThisCell(posH, posL);
				if(num == 101010)
				{
					message += "There is too much items";
					return;
				}
				
				if(choise == heroArmor->GetItem().inventorySymbol) ShowInventory('T');
				if(choise == heroWeapon->GetItem().inventorySymbol) ShowInventory('-');

				ItemsMap[posH][posL][num] = inventory[intch];
				inventory[intch].type = ItemEmpty;

				if(GetInventoryItemsWeight() <= MaxInvItemsWeight && isBurdened)
				{
					message += "You are burdened no more. ";
					isBurdened = false;
				}

				break;
			}
			case 'T':
			{
				
				heroArmor->GetItem().attribute = 100;
				heroArmor = &inventory[EMPTY_SLOT];
				break;
			
			}
			case 'w':
			{
				char hv[200] = "What do you want to wield?";

				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type == ItemWeapon)
					{
						list[len] = inventory[i];
						len++;
					}
				}

				PrintList(list, len, hv, 1);
				len = 0;
				
				char choise = getch();

				if(choise == '\033') return;
				
				int intch = choise - 'a';
		
				if(inventory[intch].type == ItemWeapon)
				{
					sprintf(tmp, "You wield %s.", inventory[intch].GetItem().GetName());
					message += tmp;

					if(heroWeapon->type != ItemEmpty)
					{
						heroWeapon->GetItem().attribute = 100;
					}
					heroWeapon = &inventory[intch];
					inventory[intch].GetItem().attribute = 301;
				}
		
				break;
			
			}
			case '-':
			{
				heroWeapon->GetItem().attribute = 100;
				heroWeapon = &inventory[EMPTY_SLOT];
				break;
			}
			case 't':
			{
				char hv[200] = "What do you want to throw?";

				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type != ItemEmpty)
					{
						list[len] = inventory[i];
						len++;
					}
				}

				PrintList(list, len, hv, 1);
				len = 0;

				char choise = getch();
				if(choise == '\033') return;
				int intch = choise - 'a';

				if(inventory[intch].type != ItemEmpty)
				{
					ClearRightPane();
					move(0, Length + 10);
					printw("In what direction?");
					char secondChoise = getch();
					if(inventory[intch].GetItem().inventorySymbol == heroArmor->GetItem().inventorySymbol) ShowInventory('T');
					if(inventory[intch].GetItem().inventorySymbol == heroWeapon->GetItem().inventorySymbol) ShowInventory('u');
					ThrowAnimated(inventory[intch], secondChoise);
				}
			}
		}
	}
	
	void Eat()
	{
		if(isFoodInInventory())
		{
			ShowInventory('e');
		}
		else message += "You don't have anything to eat. ";
	}

	void moveHero(char& inp){
		
		int a1 = 0, a2 = 0;
		
		switch(inp){
			
			case CONTROL_UP:
			{
				a1 --;
				mHLogic(a1, a2);
				break;
			}
			case CONTROL_DOWN:
			{
				a1 ++;
				mHLogic(a1, a2);
				break;	
			}
			case CONTROL_LEFT:
			{
				a2 --;
				mHLogic(a1, a2);
				break;
			}
			case CONTROL_RIGHT:
			{
				a2 ++;
				mHLogic(a1, a2);
				break;
			}
			case 'y':
			{
				a1 --;
				a2 --;
				mHLogic(a1, a2);
				break;	
			}
			case 'u':
			{
				a2 ++;
				a1 --;
				mHLogic(a1, a2);
				break;
			}
			case 'b':
			{
				a1 ++;
				a2 --;
				mHLogic(a1, a2);
				break;
			}
			case 'n':
			{
				a1 ++;
				a2 ++;
				mHLogic(a1, a2);
				break;
			}
			case 'c':
			{
				
				hunger = 3000;
				break;
				
			}
			case ',':
			{
				
				PickUp();
				break;

			}
			case 'e':
			{

				Eat();
				break;

			}
			case 'i':
			{
				if(isInventoryEmpty() == false){
					
					ShowInventory('i');
				
				}else{
					
					message += "Your inventory is empty. ";
				
				}
				break;				

			}
			case 'W':
			{

				if(isArmorInInventory() == true){
				
					ShowInventory('W');

				}
				else message += "You don't have anything to wear. ";
				break;

			}
			case 'w':
			{
			
				if(isWeaponInInventory() == true)
				{
					ShowInventory('w');
				}
				else message += "You don't have anything to wield. ";
				break;
			
			}
			case 'T':
			{
				ShowInventory('T');
				break;
			}
			case '-':
			{
				ShowInventory('-');
				break;
			}
			case 'd':
			{
				if(isInventoryEmpty() == false)
				{
					ShowInventory('d');
				}
				break;		
			}
			case 't':
			{
				if(isInventoryEmpty() == false)
				{
					ShowInventory('t');
				}
				break;
			}
			case 's':
			{
				Shoot();
				break;
			}
			case '\\':
			{
				char hv = getch();
			
				if(hv == 'w')
				{
					if(getch() == 'a')
					{
						if(getch() == 'l')
						{
							if(getch() == 'l')
							{
								if(getch() == 's')
								{
									CanHeroMoveThroughWalls = true;
								}
							}
						}
					}
				}
				else if(hv == 'd')
				{
					if(getch() == 's')
					{
						if(getch() == 'c' && symUnder != 2)
						{
							CanHeroMoveThroughWalls = false;
						}
					}
				}
				else if(hv == 'k')
				{
					if(getch() == 'i')
					{
						if(getch() == 'l')
						{
							if(getch() == 'l')
							{
								health -= (DEFAULT_HERO_HEALTH * 2) / 3;
								message += "Ouch! ";
							}
						}
					}
				}
				break;
			}
		}
	}
};

Hero hero;

enum UnitType
{
	UnitEmpty,
	UnitHero,
	UnitEnemy
};

union UnitedUnits
{
	EmptyUnit uEmpty;
	Hero uHero;
	Enemy uEnemy;
	UnitedUnits(EmptyUnit e)
	{
		uEmpty = e;
	}
	UnitedUnits(Hero h)
	{
		uHero = h;
	}
	UnitedUnits(Enemy en)
	{
		uEnemy = en;
	}
	UnitedUnits(UnitedUnits& u){}
	UnitedUnits()
	{
		uEmpty = EmptyUnit();
	}
	~UnitedUnits(){}
};

struct PossibleUnit
{
	UnitedUnits unit;
	UnitType type;
	PossibleUnit(UnitedUnits u, UnitType t): unit(u), type(t) {}
	PossibleUnit()
	{
		type = UnitEmpty;
	}
	void operator=(const Hero& h)
	{
		type = UnitHero;
		unit.uHero = h;
	}
	void operator=(const EmptyUnit& e)
	{
		type = UnitEmpty;
		unit.uEmpty = e;
	}
	void operator=(const Enemy& en)
	{
		type = UnitEnemy;
		unit.uEnemy = en;
	}
	Unit& GetUnit()
	{
		switch(type)
		{
			case UnitEmpty:
				return unit.uEmpty;
			case UnitHero:
				return unit.uHero;
			case UnitEnemy:
				return unit.uEnemy;
		}		
	}
};

PossibleUnit UnitsMap[FIELD_ROWS][FIELD_COLS];

Enemy differentEnemies[TypesOfEnemies];

void Hero::AttackEnemy(int& a1, int& a2)
{
//	sprintf(tmp, "Attacked smth with %i hp. ", UnitsMap[posH + a1][posL + a2].GetUnit().health);
//	message += tmp;
	UnitsMap[posH + a1][posL + a2].GetUnit().health -= heroWeapon->item.invWeapon.damage;
	if(UnitsMap[posH + a1][posL + a2].GetUnit().health <= 0)
	{
		UnitsMap[posH + a1][posL + a2].type = UnitEmpty;
		PossibleUnit buffer = UnitsMap[posH][posL];
		UnitsMap[posH][posL].type = UnitEmpty;
		posH += a1;
		posL += a2;
		UnitsMap[posH][posL] = buffer;
	}
}

void Hero::Shoot()
{
	if(heroWeapon->item.invWeapon.Ranged == false)
	{
		message += "You have no ranged weapon in hands. ";
		return;
	}
	if(inventory[AMMO_SLOT].type == ItemEmpty || inventory[AMMO_SLOT].item.invAmmo.count == 0)
	{
		message += "You have no bullets. ";
		return;
	}
	ClearRightPane();
	move(Length + 10, 0);
	printw("In what direction? ");
	char choise = getch();
	switch(choise)
	{
		case CONTROL_LEFT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[AMMO_SLOT].item.invAmmo.range; i++)
			{
				if(map[posH][posL - i] == 2) break;
				if(UnitsMap[posH][posL - i].type != UnitEmpty)
				{
					UnitsMap[posH][posL - i].GetUnit().health -= inventory[AMMO_SLOT].item.invAmmo.damage;
					if(UnitsMap[posH][posL - i].GetUnit().health <= 0)
					{
						UnitsMap[posH][posL - i].type = UnitEmpty;
					}
					sprintf(tmp, "!HP:%i!", UnitsMap[posH][posL - i].GetUnit().health);
					message += tmp;
				}
// !COMMENT!			// You can make this (^) bullet moving through like a special skill
				move(posH, posL - i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_DOWN:
		{	
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[AMMO_SLOT].item.invAmmo.range; i++)
			{
				if(map[posH + i][posL] == 2) break;
				if(UnitsMap[posH + i][posL].type != UnitEmpty)
				{
					UnitsMap[posH + i][posL].GetUnit().health -= inventory[AMMO_SLOT].item.invAmmo.damage;	
					if(UnitsMap[posH][posL - i].GetUnit().health <= 0)
					{
						UnitsMap[posH][posL - i].type = UnitEmpty;
					}
					sprintf(tmp, "!HP:%i!", UnitsMap[posH + i][posL].GetUnit().health);
					message += tmp;
				}
				move(posH + i, posL);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_UP:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[AMMO_SLOT].item.invAmmo.range; i++)
			{
				if(map[posH - i][posL] == 2) break;
				if(UnitsMap[posH - i][posL].type != UnitEmpty)
				{
					UnitsMap[posH - i][posL].GetUnit().health -= inventory[AMMO_SLOT].item.invAmmo.damage;		
					if(UnitsMap[posH][posL - i].GetUnit().health <= 0)
					{
						UnitsMap[posH][posL - i].type = UnitEmpty;
					}
					sprintf(tmp, "!HP:%i!", UnitsMap[posH - i][posL].GetUnit().health);
					message += tmp;
				}
				move(posH - i, posL);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_RIGHT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[AMMO_SLOT].item.invAmmo.range; i++)
			{
				if(map[posH][posL + i] == 2) break;
				if(UnitsMap[posH][posL + i].type != UnitEmpty)
				{
					UnitsMap[posH][posL + i].GetUnit().health -= inventory[AMMO_SLOT].item.invAmmo.damage;	
					if(UnitsMap[posH][posL - i].GetUnit().health <= 0)
					{
						UnitsMap[posH][posL - i].type = UnitEmpty;
					}
					sprintf(tmp, "!HP:%i!", UnitsMap[posH + i][posL].GetUnit().health);
					message += tmp;
				}
				move(posH, posL + i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
	}
	inventory[AMMO_SLOT].item.invAmmo.count--;
	if(inventory[AMMO_SLOT].item.invAmmo.count == 0)
	{
		inventory[AMMO_SLOT].type = ItemEmpty;
	}
}

void Hero::mHLogic(int& a1, int& a2)
{
	if(map[posH + a1][posL + a2] != 2 || (map[posH + a1][posL + a2] == 2 && CanHeroMoveThroughWalls))
	{
		if(UnitsMap[posH + a1][posL + a2].type == UnitEmpty)
		{
			PossibleUnit buffer = UnitsMap[posH][posL];
			UnitsMap[posH][posL].type = UnitEmpty;
			posH += a1;
			posL += a2;
			UnitsMap[posH][posL] = buffer;
		}
		else if(UnitsMap[posH + a1][posL + a2].type == UnitEnemy)
		{
			AttackEnemy(a1, a2);
		}
	}
	else if(map[posH + a1][posL + a2] == 2)
	{
		message += "The wall is the way. ";
	}
	findVisibleArray();
}

bool CheckHeroVisibility(PossibleUnit& unit)
{
	for(int i = 1; i < unit.unit.uEnemy.vision; i++)
	{
		if(map[unit.GetUnit().posH + i][unit.GetUnit().posL] == 2) break;
		if(UnitsMap[unit.GetUnit().posH + i][unit.GetUnit().posL].type == UnitHero)
		{
			unit.unit.uEnemy.dir = 1;
			unit.unit.uEnemy.dist = i;
			return true;
		}
	}
	for(int i = 1; i < unit.unit.uEnemy.vision; i++)
	{
		if(map[unit.GetUnit().posH][unit.GetUnit().posL + i] == 2) break;
		if(UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL + i].type == UnitHero)
		{
			unit.unit.uEnemy.dir = 3;
			unit.unit.uEnemy.dist = i;
			return true;
		}
	}
	for(int i = 1; i < unit.unit.uEnemy.vision; i++)
	{
		if(map[unit.GetUnit().posH - i][unit.GetUnit().posL] == 2) break;
		if(UnitsMap[unit.GetUnit().posH - i][unit.GetUnit().posL].type == UnitHero)
		{
			unit.unit.uEnemy.dir = 2;
			unit.unit.uEnemy.dist = i;
			return true;
		}
	}
	for(int i = 1; i < unit.unit.uEnemy.vision; i++)
	{
		if(map[unit.GetUnit().posH][unit.GetUnit().posL - i] == 2) break;
		if(UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL - i].type == UnitHero)
		{
			unit.unit.uEnemy.dir = 0;
			unit.unit.uEnemy.dist = i;
			return true;
		}
	}
	return false;
}
void GetRandDir(PossibleUnit& unit)
{
	unit.unit.uEnemy.dir = rand() % 4;
	unit.unit.uEnemy.dist = 0;
//	unit.unit.uEnemy.dist = rand() % unit.unit.uEnemy.vision;
// !COMMENT!			// Go to end of dist, until end ot wall meeting and change or not dist
	int posH = unit.GetUnit().posH, posL = unit.GetUnit().posL;
	switch(unit.unit.uEnemy.dir)
	{
		case 0:
		{
			for(int i = 1; i < unit.unit.uEnemy.vision; i++)
			{
				if(map[posH][posL - i] == 2) break;
				unit.unit.uEnemy.dist++;
			}
			break;
		}
		case 1:
		{
			for(int i = 1; i < unit.unit.uEnemy.vision; i++)
			{
				if(map[posH - i][posL] == 2) break;
				unit.unit.uEnemy.dist++;
			}
			break;
		}
		case 2:
		{
			for(int i = 1; i < unit.unit.uEnemy.vision; i++)
			{
				if(map[posH + i][posL] == 2) break;
				unit.unit.uEnemy.dist++;
			}
			break;
		}
		case 3:
		{
			for(int i = 1; i < unit.unit.uEnemy.vision; i++)
			{
				if(map[posH][posL + i] == 2) break;
				unit.unit.uEnemy.dist++;
			}
			break;
		}

	}
}
void UpdatePosition(PossibleUnit& unit)
{
	if(CheckHeroVisibility(unit) == false && unit.unit.uEnemy.dist <= 0)
	{
		GetRandDir(unit);
	}
	else
	{
		switch(unit.unit.uEnemy.dir)
		{
			case 0:
			{
				unit.unit.uEnemy.dist--;
				unit.GetUnit().posL--;
				UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL] = unit;
				UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL + 1].type = UnitEmpty;
				break;
			}
			case 1:
			{
				unit.unit.uEnemy.dist--;
				unit.GetUnit().posH++;
				UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL] = unit;
				UnitsMap[unit.GetUnit().posH - 1][unit.GetUnit().posL].type = UnitEmpty;
				break;
			}
			case 2:
			{
				unit.unit.uEnemy.dist--;
				unit.GetUnit().posH--;
				UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL] = unit;
				UnitsMap[unit.GetUnit().posH + 1][unit.GetUnit().posL].type = UnitEmpty;
				break;
			}
			case 3:
			{
				unit.unit.uEnemy.dist--;
				unit.GetUnit().posL++;
				UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL] = unit;
				UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL - 1].type = UnitEmpty;
				break;
			}
		}
	}
}

void UpdateAI()
{
	for(int i = 0; i < Height; i++)
	{
		for(int j = 0; j < Length; j++)
		{
			if(UnitsMap[i][j].type == UnitEnemy)
			{
				sprintf(tmp, "{%i|%i|%i}", i, j, UnitsMap[i][j].unit.uEnemy.dist);
				message += tmp;
				UpdatePosition(UnitsMap[i][j]);
			}
		}
	}
}

#define FOODSETTER buffer.posH=h;buffer.posL=l;ItemsMap[h][l][rand()%Depth]=buffer;
#define ARMORSETTER buffer.posH=h;buffer.posL=l;ItemsMap[h][l][rand()%Depth]=buffer;
#define WEAPONSETTER buffer.posH=h;buffer.posL=l;ItemsMap[h][l][rand()%Depth]=buffer;
#define AMMOSETTER buffer.posH=h;buffer.posL=l;ItemsMap[h][l][rand()%Depth]=buffer;

void SetItems()
{

	for(int i = 0; i < FOODCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfFood;
			Food buffer;
			buffer = differentFood[p];
			FOODSETTER
		}
		else i--;

	}

	for(int i = 0; i < ARMORCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfArmor;
			Armor buffer;
			buffer = differentArmor[p];
			ARMORSETTER
		}
		else i--;
	}
	
	for(int i = 0; i < WEAPONCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		
		Weapon buffer;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfWeapon;
			buffer = differentWeapon[p];
			WEAPONSETTER
		}
		else i--;
	}

	for(int i = 0; i < AMMOCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		
		Ammo buffer;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfAmmo;
			buffer = differentAmmo[p];
			buffer.count = (rand() % Luck) / 2 + 1;
			AMMOSETTER
		}
		else i--;
	}

}

void SpawnUnits()
{
	for(int i = 0; i < 1; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		if(map[h][l] == 1 && UnitsMap[h][l].type == UnitEmpty)
		{
			UnitsMap[h][l] = hero;
			hero.posH = h;
			hero.posL = l;
			break;
		}
		else i--;
	}
	for(int i = 0; i < ENEMIESCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		if(map[h][l] == 1 && UnitsMap[h][l].type == UnitEmpty)
		{
			int p = rand() % TypesOfEnemies;
			Enemy buffer = differentEnemies[p];
			buffer.posH = h;
			buffer.posL = l;
			UnitsMap[h][l] = buffer;
		}
		else i--;
	}
}

void Draw(){
	
	move(0, 0);

	static int mapSaved[FIELD_ROWS][FIELD_COLS] = {};

	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			if(seenUpdated[i][j])mapSaved[i][j] = map[i][j];
		}
	}
	
	for(int i = 0; i < Height; i++){
		
		for(int j = 0; j < Length; j++){
			
//			printw("% i ", map[i][j]);										// !DEBUG!
/*			if(mapSaved[i][j] != 0)
			{*/
				bool near = abs(i - hero.posH) <= 1 && abs(j - hero.posL) <= 1;
				
				if(hero.FindElementsNumberUnderThisCell(i, j) == 0 && UnitsMap[i][j].type == UnitEmpty)
				{
					switch(map/*Saved*/[i][j])
					{
						case 1:
							if(near)
							{
								addch('_');
							}
							else
							{
								addch('_' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							}
							break;
						case 2:
							if(near)
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							}
							else
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK));
							}
							break;
					}
				}
				else if(hero.FindElementsNumberUnderThisCell(i, j) == 1 && UnitsMap[i][j].type == UnitEmpty)
				{
					int MeetedElement = hero.FindNotEmptyElementUnderThisCell(i, j);
					switch(ItemsMap[i][j][MeetedElement].GetItem().symbol){

						case 100:
							addch('%');
							break;
						case 101:
							addch('%' | COLOR_PAIR(RED_BLACK));
							break;
						case 300:
							addch('&' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 301:
							addch('&' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 400:
							addch('/' | COLOR_PAIR(RED_BLACK) | LIGHT);
							break;
						case 401:
							addch('/' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 402:
							addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 450:
							addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
							break;
					}
				}
				else if(hero.FindElementsNumberUnderThisCell(i, j) > 1 && UnitsMap[i][j].type == UnitEmpty)
				{
					addch('^' | COLOR_PAIR(BLACK_WHITE) | LIGHT);
				}
	
				if(UnitsMap[i][j].type == UnitHero)
				{
					addch('@' | COLOR_PAIR(GREEN_BLACK));
				}
				else if(UnitsMap[i][j].type == UnitEnemy)
				{
					switch(UnitsMap[i][j].GetUnit().symbol)
					{
						case 201:
							addch('@' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
					}
				}
/*			}
			else
			{
				addch(' ');	
			}	*/
					
		}
		
		printw("\n");
		
	}

}

void MainMenu()
{
	move(0, 0);
	printw("Choose mode:\n1.Normal\n2.Hard\n");
	char hv = getch();
	switch(hv)
	{
		case '1':
			MODE = 1;
			break;
		case '2':
			MODE = 2;
			break;
		default:
			MODE = 1;
			break;
	}
}

int main()
{
	initscr();
	
	noecho();
	
	start_color();
	
	init_pair(BLACK_BLACK, COLOR_BLACK, COLOR_BLACK);
	init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);
	init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
	init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
	init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);
	init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);
	init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
	init_pair(BLACK_WHITE, COLOR_BLACK, COLOR_WHITE);

	initialize();

	generate_maze();

	hero.health = DEFAULT_HERO_HEALTH;
	hero.symbol = 200;
	hero.hunger = 900;
	Luck = (rand() % 20 + rand() % 20) / 2;

	Food Egg(0);
	Food Apple(1);
	differentFood[0] = Egg;
	differentFood[1] = Apple;
	
	Armor ChainChestplate(0);
	Armor LeatherChestplate(1);
	differentArmor[0] = ChainChestplate;
	differentArmor[1] = LeatherChestplate;

	inventory[0].item = LeatherChestplate;
	inventory[0].type = ItemArmor;
	inventory[0].GetItem().inventorySymbol = 'a';
	inventoryVol++;
	hero.heroArmor = &inventory[0];
	hero.heroArmor->GetItem().attribute = 201;
	
	Weapon CopperShortsword(0);
	Weapon BronzeSpear(1);
	Weapon Musket(2);
	differentWeapon[0] = CopperShortsword;
	differentWeapon[1] = BronzeSpear;
	differentWeapon[2] = Musket;
	
	Ammo SteelBullets(0);
	differentAmmo[0] = SteelBullets;
	
	Enemy Dummy(0);
	differentEnemies[0] = Dummy;

	hero.heroWeapon = &inventory[EMPTY_SLOT];

	SetItems();

	SpawnUnits();
		
	hero.findVisibleArray();

	MainMenu();
	
	int TurnsCounter = 0;
	
	Draw();
			
	move(Height, 0);
	sprintf(tmp, "HP: %i ", hero.health);
	bar += tmp;
	sprintf(tmp, "Sat: %i ", hero.hunger);
	bar += tmp;
	sprintf(tmp, "Def: %i ", hero.heroArmor->item.invArmor.defence);
	bar += tmp;
	sprintf(tmp, "Dmg: %i ", hero.heroWeapon->item.invWeapon.damage);
	bar += tmp;
	sprintf(tmp, "L: %i ", Luck);								// !DEBUG!
	bar += tmp;										//
	if(hero.isBurdened) bar += "Burdened. ";
	printw("%- 190s", bar.c_str());

	move(hero.posH, hero.posL);
	
	hero.symUnder = 1;

	while(1){
		
		if(!StopUpdating)
		{
			message = "";
			bar = "";

			move(hero.posH, hero.posL);

			char inp = getch();
		
			TurnsCounter++;
	
			if(hero.hunger < 1)
			{
				message += "You died from starvation. Press any key to exit.";
				printw("%- 190s", message.c_str());
				getch();
				break;
			}

			if(hero.health < 1)
			{
				message += "You died. Press any key to exit.";
				printw("% -190s", message.c_str());
				getch();
				break;
			}

			if(TurnsCounter > 50 && MODE == 1)
			{
				if(hero.health < DEFAULT_HERO_HEALTH)
				{
					hero.health ++;
				}
				TurnsCounter = 0;
			}
			else if(MODE == 1 && TurnsCounter > 100)
			{
				TurnsCounter = 0;
			}
	
			hero.hunger--;
			
			if(hero.isBurdened) hero.hunger--;
			
			hero.moveHero(inp);

			UpdateAI();
	
			Draw();
			
			move(Height, 0);										//
			sprintf(tmp, "HP: %i ", hero.health);								//
			bar += tmp;											//
			sprintf(tmp, "Sat: %i ", hero.hunger);								//
			bar += tmp;											//
			sprintf(tmp, "Def: %i ", hero.heroArmor->item.invArmor.defence);				// 
			bar += tmp;											//
			sprintf(tmp, "Dmg: %i ", hero.heroWeapon->item.invWeapon.damage);				//
			bar += tmp;											// Condition bar	
			sprintf(tmp, "L: %i ", Luck);									// !DEBUG!
			bar += tmp;											// !!
			if(inventory[AMMO_SLOT].type != ItemEmpty)							//	
			{												//
				sprintf(tmp, "Bul: %i ", inventory[AMMO_SLOT].item.invAmmo.count);			//
				bar += tmp;										//
			}												//
			if(hero.isBurdened) bar += "Burdened. ";							//
			printw("%- 190s", bar.c_str());									//
		
			if(hero.hunger < 75)
			{	
				bar += "Hungry. ";
			}
	
			move(Height + 1, 0);
			
			printw("%- 190s", message.c_str());
			
			if(inp == '\033'){
				
				move(Height, 0);
				printw("Are you sure want to exit?\n");
				if(getch() == 'y') break;
	
			}	
			move(hero.posH, hero.posL);
		}
	}
		
	refresh();
		
	endwin();
		
	return 0;
	
}
