#include<time.h>
#include<ncurses.h>
#include"include/unit.hpp"
#include"include/utils.hpp"
#include"include/level.hpp"
#include"include/colors.hpp"
#include"include/controls.hpp"
#include"include/globals.hpp"
#include"include/log.hpp"

int VISION = 16;
int MaxInvItemsWeight = 25;								
int DEFAULT_HERO_HEALTH = 15;

Unit::Unit(): inventoryVol(0){};

const char* Unit::getName()
{
	switch(symbol)
	{
		case 200:
			return "Hero";
		case 201:
			return "Barbarian";
		case 202:
			return "Zombie";
	}
}

bool Unit::linearVisibilityCheck( double from_x, double from_y, double to_x, double to_y )
{
	double dx = to_x - from_x;
	double dy = to_y - from_y;
	if( ABS( dx ) > ABS( dy ) )
	{
		double k = dy / dx;
		int s = SGN( dx );
		for( int i = 0; i * s < dx * s; i += s )
		{
			int x = from_x + i;
			int y = from_y + i * k;
			if( map[y][x] == 2 )
			{
				return false;
			}
		}
	}
	else
	{
		double k = dx / dy;
		int s = SGN( dy );
		for( int i = 0; i * s < dy * s; i += s )
		{
			int x = from_x + i * k;
			int y = from_y + i;
			if( map[y][x] == 2 )
			{
				return false;
			}
		}
	}
	return true;
}

bool Unit::canSeeCell( int h, int l )
{
	double offset = 1. / VISION_PRECISION;
	return
	linearVisibilityCheck( posL + .5, posH + .5, l + offset, h + offset ) ||
	linearVisibilityCheck( posL + .5, posH + .5, l + offset, h + 1 - offset ) ||
	linearVisibilityCheck( posL + .5, posH + .5, l + 1 - offset, h + offset ) ||
	linearVisibilityCheck( posL + .5, posH + .5, l + 1 - offset, h + 1 - offset );
}

void Unit::delay(double s)
{
	clock_t clocksNow = clock();
	while(double(clock() - clocksNow) / CLOCKS_PER_SEC < s);
}	

Unit::~Unit(){};

EmptyUnit::EmptyUnit(){}

EmptyUnit::~EmptyUnit(){}

Enemy::Enemy(int eType): movedOnTurn( 0 ) /*-1*/
{
	switch(eType)
	{
		case 0:
		{
			health = 7;
			unitInventory[0] = differentFood[0];
			unitInventory[1] = differentWeapon[0];
			unitWeapon = &unitInventory[1];
			inventoryVol = 2;
			symbol = 201;
			vision = 16;
			xpIncreasing = 3;
			break;
		}
		case 1:
		{
			health = 10;
			unitInventory[0] = differentWeapon[3];
			unitWeapon = &unitInventory[0];
			inventoryVol = 1;
			symbol = 202;
			vision = 10;
			xpIncreasing = 2;
			break;
		}
		case 2:
		{
			health = 5;
			unitInventory[0] = differentWeapon[5];
			unitInventory[1] = differentAmmo[0];
			unitWeapon = &unitInventory[0];
			unitAmmo = &unitInventory[1];
			unitAmmo->item.invAmmo.count = rand() % 30 + 4;
			inventoryVol = 2;
			symbol = 203;
			vision = 16;
			xpIncreasing = 5;
			break;
		}
	}
	dist = 0;
	targetH = -1;
	targetL = -1;
}

Enemy::Enemy(){}

Enemy::~Enemy(){}

Hero::Hero(): isBurdened(false), CanHeroMoveThroughWalls(false), xp(0), level(1) {}

void Hero::findVisibleArray()
{
	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			seenUpdated[i][j] = 0;
			if( SQR( posH - i ) + SQR( posL - j ) < SQR( VISION ) )
			{
				seenUpdated[i][j] = canSeeCell( i, j );
			}
		}
	}
}

bool Hero::isInventoryEmpty()
{
	for(int i = 0; i < MaxInvVol; i++)
	{
		if(inventory[i].type != ItemEmpty) return false;
	}
	return true;
}

int Hero::findEmptyInventoryCell()
{
	for(int i = 0; i < MaxInvVol; i++)
	{
		if(inventory[i].type == ItemEmpty) return i;
	}
	return 101010;											// Magic constant, means "Inventory is full".
}	

int Hero::getInventoryItemsWeight()
{
	int toReturn = 0;
	for(int i = 0; i < MaxInvVol; i++)
	{
		if(inventory[i].type != ItemEmpty)
		{
			toReturn += inventory[i].getItem().weight;
		}
	
	}
	return toReturn;
}

void Hero::printList(PossibleItem items[], int len, char msg[], int mode)
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
				if(items[i].getItem().showMdf == true && items[i].getItem().count == 1)
				{
					if( items[i].getItem().attribute == 100 )
					{
						printw("[%c] %s {%s}. ", items[i].getItem().inventorySymbol, items[i].getItem().getName(), items[i].getItem().getMdf());
					}
					else
					{
						printw("[%c] %s (%s) {%s}. ", items[i].getItem().inventorySymbol, items[i].getItem().getName(), items[i].getItem().getAttribute(), items[i].getItem().getMdf());
					}
				}
				else if(items[i].getItem().count > 1)
				{
					if( items[i].getItem().attribute == 100 )
					{
						printw("[%c] %s {%i}. ", items[i].getItem().inventorySymbol, items[i].getItem().getName(), items[i].getItem().count);
					}
					else
					{
						printw("[%c] %s (%s) {%i}. ", items[i].getItem().inventorySymbol, items[i].getItem().getName(), items[i].getItem().getAttribute(), items[i].getItem().count);
					}
				}
				else if( items[i].getItem().attribute == 100 )
				{
					printw("[%c] %s. ", items[i].getItem().inventorySymbol, items[i].getItem().getName());
				}
				else
				{
					printw("[%c] %s (%s). ", items[i].getItem().inventorySymbol, items[i].getItem().getName(), items[i].getItem().getAttribute());
				}
				num ++;
			}
			break;
		}
		case 2:
		{
			for(int i = 0; i < len; i++)
			{
				move(num, Length + 10);
				if(items[i].getItem().showMdf == true)
				{
					printw("[%c] %s (%s) {%s}. ", i + 'a', items[i].getItem().getName(), items[i].getItem().getAttribute(), items[i].getItem().getMdf());
				}
				else printw("[%c] %s (%s). ", i + 'a', items[i].getItem().getName(), items[i].getItem().getAttribute());
				num ++;
			}
			break;

		}
	}
}

bool Hero::isMapInInventory()
{
	for(int i = 0; i < MaxInvVol; i++)
	{
		if(inventory[i].type != ItemEmpty && inventory[i].getItem().symbol == 500) return true;
	}
	return false;
}

int Hero::findItemsCountUnderThisCell(int h, int l)
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

int Hero::findEmptyItemUnderThisCell(int h, int l)
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

int Hero::findNotEmptyItemUnderThisCell(int h, int l)
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

int Hero::findAmmoInInventory()
{
	for(int i = 0; i < BANDOLIER; i++)
	{
		if(inventory[AMMO_SLOT + i].type == ItemAmmo) return i;
	}
	return 101010;
}

int Hero::findScrollInInventory()
{
	for(int i = 0; i < MaxInvVol; i++)
	{
		if(inventory[i].type == ItemScroll)
		{
			return i;
		}
	}
	return 101010;
}

void Hero::printAmmoList(PossibleItem& pAmmo)										// Picked ammo
{
	clearRightPane();
	move(0, Length + 10);
	printw("In what slot do vou want to pull your ammo?");
//		move(1, Length + 10);
	int choise = 0;
	int num = 0;
	while(1)
	{
		num = 0;
		for(int i = 0; i < BANDOLIER; i++)
		{
			move(1, Length + 12 + num);
			num += 2;
			if(inventory[AMMO_SLOT + i].type == ItemAmmo)
			{
				switch(inventory[AMMO_SLOT + i].getItem().symbol)
				{
					case 450:
						if(choise == i) addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT | UL);
						else addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
						break;
					case 451:
						if(choise == i) addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT | UL);
						else addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
						break;
					default:
						if(choise == i) addch('-' | COLOR_PAIR(WHITE_BLACK) | UL);
						else addch('-' | COLOR_PAIR(WHITE_BLACK));
						break;
				}
			}
			else
			{
				if(choise == i) addch('-' | COLOR_PAIR(WHITE_BLACK) | UL);
				else addch('-' | COLOR_PAIR(WHITE_BLACK));
			}
		}
		switch(getch())
		{
			case CONTROL_LEFT:
				if(choise > 0) choise--;
				break;
			case CONTROL_RIGHT:
				if(choise < BANDOLIER - 1) choise++;
				break;
			case CONTROL_CONFIRM:
				if(inventory[AMMO_SLOT + choise].item.invAmmo.symbol != pAmmo.item.invAmmo.symbol && inventory[AMMO_SLOT + choise].type == ItemAmmo)
				{
					PossibleItem buffer;
					buffer = pAmmo;
					pAmmo = inventory[AMMO_SLOT + choise];
					inventory[AMMO_SLOT + choise] = buffer;
				}
				else if(inventory[AMMO_SLOT + choise].item.invAmmo.symbol == pAmmo.item.invAmmo.symbol && inventory[AMMO_SLOT + choise].type == ItemAmmo)
				{
					inventory[AMMO_SLOT + choise].item.invAmmo.count += pAmmo.item.invAmmo.count;
					pAmmo.type = ItemEmpty;
				}
				else if(inventory[AMMO_SLOT + choise].type == ItemEmpty)
				{
					inventory[AMMO_SLOT + choise] = pAmmo;
					pAmmo.type = ItemEmpty;
				}
				return;
				break;
			case '\033':
				return;
				break;
		}
	}
}

void Hero::pickUp()
{
	if(findItemsCountUnderThisCell(posH, posL) == 0)
	{
		message += "There is nothing here to pick up. ";
		Stop = true;
		return;
	}
	else if(findItemsCountUnderThisCell(posH, posL) == 1)
	{
		int num = findNotEmptyItemUnderThisCell(posH, posL);

		sprintf(tmp, "You picked up %s. ", ItemsMap[posH][posL][num].getItem().getName());
		message += tmp;

		if(ItemsMap[posH][posL][num].type == ItemAmmo)
		{
			printAmmoList(ItemsMap[posH][posL][num]);
			return;
		}

		bool couldStack = false;

		if( ItemsMap[posH][posL][num].getItem().isStackable )
		{
			for( int i = 0; i < MaxInvVol; ++i )
			{
				if( inventory[i].type != ItemEmpty && inventory[i].getItem().symbol == ItemsMap[posH][posL][num].getItem().symbol )
				{
					couldStack = true;
					inventory[i].getItem().count += ItemsMap[posH][posL][num].getItem().count;
					ItemsMap[posH][posL][num].type = ItemEmpty;
				}
			}
		}

		if( !couldStack )
		{
			int eic = findEmptyInventoryCell();
			if(eic != 101010)
			{
				inventory[eic] = ItemsMap[posH][posL][num];
				inventory[eic].getItem().inventorySymbol = eic + 'a';
				log("Item index: '%c'\n", inventory[eic].getItem().inventorySymbol);
				ItemsMap[posH][posL][num].type = ItemEmpty;
				inventoryVol++;
			}
			else
			{
				message += "Your inventory is full, motherfuck'a! ";
			}
		}

		if(getInventoryItemsWeight() > MaxInvItemsWeight && !isBurdened)
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

	printList(list, len, hv, 2);
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
		sprintf(tmp, "You picked up %s. ", ItemsMap[posH][posL][helpfulArray[intch]].getItem().getName());
		message += tmp;
		
		if(ItemsMap[posH][posL][helpfulArray[intch]].type == ItemAmmo)
		{
			printAmmoList(ItemsMap[posH][posL][helpfulArray[intch]]);
			return;
		}

		bool couldStack = false;

		if( ItemsMap[posH][posL][helpfulArray[intch]].getItem().isStackable )
		{
			for( int i = 0; i < MaxInvVol; ++i )
			{
				if( inventory[i].type != ItemEmpty && inventory[i].getItem().symbol == ItemsMap[posH][posL][helpfulArray[intch]].getItem().symbol )
				{
					couldStack = true;
					inventory[i].getItem().count += ItemsMap[posH][posL][helpfulArray[intch]].getItem().count;
					ItemsMap[posH][posL][helpfulArray[intch]].type = ItemEmpty;
				}
			}
		}

		if( !couldStack )
		{
			int eic = findEmptyInventoryCell();
			if(eic != 101010)
			{
				inventory[eic] = ItemsMap[posH][posL][helpfulArray[intch]];
				inventory[eic].getItem().inventorySymbol = eic + 'a';
				ItemsMap[posH][posL][helpfulArray[intch]].type = ItemEmpty;
				inventoryVol++;
			}
			else
			{
				message += "Your inventory is full, motherfuck'a! ";
			}
		}
	}

	if(getInventoryItemsWeight() > MaxInvItemsWeight && !isBurdened)
	{
		message += "You're burdened. ";
		isBurdened = true;
	}
}

bool Hero::isFoodInInventory()
{
	for(int i = 0; i < MaxInvVol; i++)
	{
		if(inventory[i].type == ItemFood) return true;
	}
	return false;
}

bool Hero::isArmorInInventory()
{
	for(int i = 0; i < MaxInvVol; i++)
	{
		if(inventory[i].type == ItemArmor) return true;
	}
	return false;
}

bool Hero::isWeaponOrToolsInInventory()
{
	for(int i = 0; i < MaxInvVol; i++)
	{
		if(inventory[i].type == ItemWeapon || inventory[i].type == ItemTools) return true;
	}
	return false;
}

bool Hero::isPotionInInventory()
{
	for(int i = 0; i < MaxInvVol; i++)
	{
		if(inventory[i].type == ItemPotion) return true;
	}
	return false;
}

void Hero::clearRightPane()
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

void Hero::delay(double s)
{
	clock_t clocksNow = clock();
	while(double(clock() - clocksNow) / CLOCKS_PER_SEC < s);
}

void Hero::eat()
{
	if(isFoodInInventory())
	{
		showInventory(CONTROL_EAT);
	}
	else message += "You don't have anything to eat. ";
}

void Hero::moveHero(char& inp)
{
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
		case CONTROL_UPLEFT:
		{
			a1 --;
			a2 --;
			mHLogic(a1, a2);
			break;	
		}
		case CONTROL_UPRIGHT:
		{
			a2 ++;
			a1 --;
			mHLogic(a1, a2);
			break;
		}
		case CONTROL_DOWNLEFT:
		{
			a1 ++;
			a2 --;
			mHLogic(a1, a2);
			break;
		}
		case CONTROL_DOWNRIGHT:
		{
			a1 ++;
			a2 ++;
			mHLogic(a1, a2);
			break;
		}
		case CONTROL_PICKUP:
		{
			pickUp();
			break;
		}
		case CONTROL_EAT:
		{
			eat();
			break;
		}
		case CONTROL_SHOWINVENTORY:
		{
			if(isInventoryEmpty() == false)
			{
				showInventory(CONTROL_SHOWINVENTORY);
			}
			else
			{
				message += "Your inventory is empty. ";
			}
/*->*/				Stop = true;
			break;				
		}
		case CONTROL_WEAR:
		{

			if(isArmorInInventory() == true)
			{
				showInventory(CONTROL_WEAR);
			}
			else message += "You don't have anything to wear. ";
/*->*/				Stop = true;
			break;

		}
		case CONTROL_WIELD:
		{
			if(isWeaponOrToolsInInventory() == true)
			{
				showInventory(CONTROL_WIELD);
			}
			else message += "You don't have anything to wield. ";
/*->*/				Stop = true;
			break;
		}
		case CONTROL_TAKEOFF:
		{
			showInventory(CONTROL_TAKEOFF);
/*->*/				Stop = true;
			break;
		}
		case CONTROL_UNEQUIP:
		{
			showInventory(CONTROL_UNEQUIP);
/*->*/				Stop = true;
			break;
		}
		case CONTROL_DROP:
		{
			if(isInventoryEmpty() == false)
			{
				showInventory(CONTROL_DROP);
			}
/*->*/				Stop = true;
			break;		
		}
		case CONTROL_THROW:
		{
			if(isInventoryEmpty() == false)
			{
				showInventory(CONTROL_THROW);
			}
			break;
		}
		case CONTROL_SHOOT:
		{
			shoot();
			break;
		}
		case CONTROL_DRINK:
		{
			if(isPotionInInventory() == true)
			{
				showInventory(CONTROL_DRINK);
			}
/*->*/				Stop = true;
			break;
		}
		case CONTROL_OPENBANDOLIER:
		{
			if(findAmmoInInventory() != 101010)
			{
				showInventory(CONTROL_OPENBANDOLIER);
			}
			else message += "Your bandolier is empty. ";
/*->*/				Stop = true;
			break;
		}
		case CONTROL_RELOAD:
		{
			if(!heroWeapon->item.invWeapon.Ranged)
			{
				message += "You have no ranged weapon in hands. ";
				Stop = true;
			}
			else if(findAmmoInInventory() != 101010)
			{
				showInventory(CONTROL_RELOAD);
			}
			else
			{
				message += "You have no bullets to reload. ";
				Stop = true;
			}
			break;
		}
		case CONTROL_READ:
		{
			if(findScrollInInventory() != 101010)
			{
				showInventory(CONTROL_READ);
			}
			else message += "You don't have anything to read. ";
/*->*/				Stop = true;
			break;
		}
		case '\\':
		{
			char hv = getch();
			
			if(hv == 'h')
			{
				if(getch() == 'e')
				{
					if(getch() == 'a')
					{
						if(getch() == 'l')
						{
							hunger = 3000;
							health = DEFAULT_HERO_HEALTH * 100;
						}
					}
				}
			}
		
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
					if(getch() == 'c')
					{
						CanHeroMoveThroughWalls = false;
					}
				}
				else
				{
					ItemsMap[1][1][0] = differentFood[0];
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
