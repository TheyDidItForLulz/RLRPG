#ifndef UNIT_HPP
#define UNIT_HPP

#include"item.hpp"
#define DELAY 0.07
#define ENEMIESCOUNT 17
#define DEFAULT_VISION 16
#define VISION_PRECISION 256
#define UNITINVENTORY 4

class Unit
{
public:
	Unit();
	int health;
	PossibleItem unitInventory[UNITINVENTORY];
	int inventoryVol;
	PossibleItem* unitWeapon;
	PossibleItem* unitArmor;
	PossibleItem* unitAmmo;
	int posH;
	int posL;
	int symbol;
	int vision;

	const char* GetName();
	bool LinearVisibilityCheck( double from_x, double from_y, double to_x, double to_y );
	bool CanSeeCell( int h, int l );
	void Delay(double s);	
	~Unit();
};

#endif // UNIT_HPP
