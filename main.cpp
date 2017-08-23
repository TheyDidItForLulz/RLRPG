//////////////////////////////////////////////////////////////////////////////////////// Symbols ///////////////////////////////////////////////////////////////////
/*
																				'_'(Floor) == 1
																				'#'(Wall) == 2
																				'^'(Pile) == 3
																				'%'(Egg) == 100
																				'%'(Apple) == 101
																				'@'(Hero) == 200
																				'@'(Barbarian) == 201
																				'@'(Zombie) == 202
																				'@'(Guardian) == 203
																				'&'(Leather armor) == 301
																				'&'(Chain armor) == 300
																				'/'(Copper shortsword) == 400
																				'/'(Bronze spear) == 401
																				'/'(Musket) == 402
																				'/'(Stick) == 403
																				'/'(Shotgun) == 404
																				'/'(Pistol) == 405
																				','(Steel bullets) == 450
																				','(Shotgun shells) == 451
																				'~'(Map) == 500
																				'~'(Identify scroll) == 501
																				'!'(Blue potion) == 600
																				'!'(Green potion) == 601
																				'!'(Dark potion) == 602
																				'!'(Magenta potion) == 603
																				'!'(Yellow potion) == 604
																				'\'(Pickaxe) == 700
*/
//////////////////////////////////////////////////////////////////////////////////////// Modificators //////////////////////////////////////////////////////////////
/*
																				1 - Nothing
																				2 - Thorns (chance to turn damage back)
*/
//////////////////////////////////////////////////////////////////////////////////////// Attributes ////////////////////////////////////////////////////////////////
/*
																				100 - Notning
																				101..199 - Food attributes
																				200..299 - Armor attributes
																				300..399 - Weapon attributes
																				201 - Worn
																				301 - Wielded
*/
///////////////////////////////////////////////////////////////////////////////////////// Effects //////////////////////////////////////////////////////////////////
/*
																				1 - Map recording in Hard-mode				
																				2 - Identify
*/
/////////////////////////////////////////////////////////////////////////////////////// Potion Effects /////////////////////////////////////////////////////////////
/*
																				1 - Healing 3 hp
																				2 - Invisibility
																				3 - Random location teleport
																				4 - Nothing
																				5 - Blindness
*/
/////////////////////////////////////////////////////////////////////////////////////// Tool possibilities /////////////////////////////////////////////////////////
/*
																				1 - Digging through walls
*/
///////////////////////////////////////////////////////////////////////////////////// Types of weapon construction /////////////////////////////////////////////////
/*
																				1 - One direction(Musket), scheme:

																						@--->
																				
																				2 - Triple direction(Shotgun), scheme:
																		
																						  /
																						 /
																					@--->
																					 \
																					  \

																				  !COMMENT! This isn't realized	(lol) хуй
*/
//////////////////////////////////////////////////////////////////////////////////////// Tree of skills ////////////////////////////////////////////////////////////
/*
                                                                                                                                                                   
                                                                                         Detect, is food rotten                                                     
                                                                                           /                \
                                                                                Shoot through             Chance to confuse monster                                 
                                                                                      |                               |                                             
                                                                                Combine things             Items identify by hands(?)                               
                                                                                                                                                                    
                                                                                                                                                                    
                                                                                                                                                                    
                                                                                                                                                                    
                                                                                                                                                                    
                                                                                                                                                                    
                                                                                                                                                                    
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//!COMMENT! // Also it isn't needed to show to the player his satiation. And luck too. And other stuff.

#include<stdio.h>									//
#include<iostream>									//
#include<stdlib.h>									//
#include<time.h>									//
#include<ncurses.h>									//
#include<string.h>									//
#include<vector>
#include<queue>

// !COMMENT! random potion effect, random events, modificators

#define TypesOfEnemies 3
#define EMPTY_SLOT 52
int BLINDNESS = 0;
// !COMMENT! // Level-up and items stacking
// !COMMENT! // Enemies must move at first turn
int MODE = 1;	
int MenuCondition = 0;
bool EXIT = false;
bool Stop = false;

bool GenerateMap = true;

using namespace std;									

#include"include/colors.hpp"
#include"include/controls.hpp"
#include"include/level.hpp"
#include"include/item.hpp"
#include"include/gen_map.hpp"
#include"include/unit.hpp"
#include"include/utils.hpp"
															
int map[ FIELD_ROWS ][ FIELD_COLS ];											
bool seenUpdated[FIELD_ROWS][FIELD_COLS];										// <- visible array
int active = 0;														
int turns = 0; /*-1*/
															
void init_field( void )													
{															
	for( int i = 0; i < FIELD_ROWS; ++i )										
	{														
		for( int j = 0; j < FIELD_COLS; ++j )									
		{													
			map[ i ][ j ] = 1;										
		}													
	}														
}															//
															//
void initialize( void )													//
{															//
	srand( time( 0 ) );												//
	active = 1;													//
	init_field();													//
}															//
															//
string message = "";
string bar = "";
string weapon_bar = "";
char tmp[100];

void MoveUnit(int, int&, int&, int, int);

int inventoryVol = 0;

int Luck;
int INVISIBILITY = 0;

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

void Hero::ShowInventory(const char& inp)
{	
	PossibleItem list[MaxInvVol];

	int len = 0;

	switch(inp)
	{	
		case CONTROL_SHOWINVENTORY:
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

		case CONTROL_EAT:
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

			if(inventory[intch].type == ItemFood)
			{
				int prob = rand() % Luck;
				if(prob == 0)
				{
					hunger += inventory[intch].item.invFood.FoodHeal / 3;
					health --;
					message += "Fuck! This food was rotten! ";
				}
				else
				{
					hunger += inventory[intch].item.invFood.FoodHeal;
				}
				if(inventory[intch].GetItem().count == 1)
				{
					inventory[intch].type = ItemEmpty;
				}
				else
				{
					inventory[intch].GetItem().count--;
				}
			}
			
			break;
		}	
		case CONTROL_WEAR:
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

		case CONTROL_DROP:
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
			
			int num = FindEmptyItemUnderThisCell(posH, posL);
			if(num == 101010)
			{
				message += "There is too much items";
				return;
			}
			
			if(choise == heroArmor->GetItem().inventorySymbol) ShowInventory(CONTROL_TAKEOFF);
			if(choise == heroWeapon->GetItem().inventorySymbol) ShowInventory(CONTROL_UNEQUIP);

			ItemsMap[posH][posL][num] = inventory[intch];
			inventory[intch].type = ItemEmpty;

			if(GetInventoryItemsWeight() <= MaxInvItemsWeight && isBurdened)
			{
				message += "You are burdened no more. ";
				isBurdened = false;
			}

			break;
		}
		case CONTROL_TAKEOFF:
		{
			
			heroArmor->GetItem().attribute = 100;
			heroArmor = &inventory[EMPTY_SLOT];
			break;
		
		}
		case CONTROL_WIELD:
		{
			char hv[200] = "What do you want to wield?";

			for(int i = 0; i < MaxInvVol; i++)
			{
				if(inventory[i].type == ItemWeapon || inventory[i].type == ItemTools)
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
	
			if(inventory[intch].type == ItemWeapon || inventory[intch].type == ItemTools)
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
		case CONTROL_UNEQUIP:
		{
			heroWeapon->GetItem().attribute = 100;
			heroWeapon = &inventory[EMPTY_SLOT];
			break;
		}
		case CONTROL_THROW:
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
				if(inventory[intch].GetItem().inventorySymbol == heroArmor->GetItem().inventorySymbol) ShowInventory(CONTROL_TAKEOFF);
				if(inventory[intch].GetItem().inventorySymbol == heroWeapon->GetItem().inventorySymbol) ShowInventory(CONTROL_UNEQUIP);
				ThrowAnimated(inventory[intch], secondChoise);
			}
			break;
		}
		case CONTROL_DRINK:
		{
			char hv[200] = "What do you want to drink?";

			for(int i = 0; i < MaxInvVol; i++)
			{
				if(inventory[i].type == ItemPotion)
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

			if(inventory[intch].type == ItemPotion)
			{
				switch(inventory[intch].item.invPotion.effect)
				{
					case 1:
					{
						health += 3;
						if(health > DEFAULT_HERO_HEALTH)
						{
							health = DEFAULT_HERO_HEALTH;
						}
						message += "Now you feeling better. ";
						break;
					}
					case 2:
					{
						INVISIBILITY = 150;
						message += "Am I invisible? Oh, lol! ";
						break;
					}
					case 3:
					{
						for(int i = 0; i < 1; i++)
						{
							int l = rand() % Length;
							int h = rand() % Height;
							if(map[h][l] != 2 && UnitsMap[h][l].type == UnitEmpty)
							{
								UnitsMap[h][l] = UnitsMap[posH][posL];
								UnitsMap[posH][posL].type = UnitEmpty;
								posH = h;
								posL = l;
								FindVisibleArray();
							}
							else i--;
						}
						message += "Teleportation is so straaange thing! ";
						break;
					}
					case 4:
					{
						message += "Well.. You didn't die. Nice. ";
						break;
					}
					case 5:
					{
						VISION = 1;
						BLINDNESS = 50;
						message += "My eyes!! ";
						break;
					}
				}
				discoveredPotions[inventory[intch].item.invPotion.symbol - 600] = true;
				if( inventory[intch].GetItem().count == 1 )
				{
					inventory[intch].type = ItemEmpty;
				}
				else
				{
					--inventory[intch].GetItem().count;
				}
			}
			break;
		}
		case CONTROL_READ:
		{
			char hv[200] = "What do you want to read?";

			for(int i = 0; i < MaxInvVol; i++)
			{
				if(inventory[i].type == ItemScroll)
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

			if(inventory[intch].type == ItemScroll)
			{
				switch(inventory[intch].item.invPotion.effect)
				{
					case 1:
					{
						message += "You wrote this map. Why you read it, I don't know. ";
						break;
					}
					case 2:
					{
						ClearRightPane();
						move(0, Length + 10);
						printw("What do you want to identify?");
						char in = getch();
						int intin = in - 'a';
						if(inventory[intin].type != ItemEmpty)
						{
							if(inventory[intin].type != ItemPotion)
							{
								inventory[intin].GetItem().showMdf = true;
							}
							else if(inventory[intin].type == ItemPotion)
							{
								discoveredPotions[inventory[intin].GetItem().symbol - 600] = true;
							}	
						
							if( inventory[intch].GetItem().count == 1 )
							{
								inventory[intch].type = ItemEmpty;
							}
							else
							{
								--inventory[intch].GetItem().count;
							}
						}
						break;
					}
				}
			}
			break;
		}
		case CONTROL_OPENBANDOLIER:
		{
			ClearRightPane();
			move(0, Length + 10);
			printw("Here is your ammo.");
	//		move(1, Length + 10);
			int choise = 0;
			int num = 0;
			PossibleItem buffer;
			int pos;
			while(1)
			{
				num = 0;
				for(int i = 0; i < BANDOLIER; i++)
				{
					move(1, Length + 12 + num);
					num += 2;
					if(inventory[AMMO_SLOT + i].type == ItemAmmo)
					{
						switch(inventory[AMMO_SLOT + i].GetItem().symbol)
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
					{
						if(choise > 0) choise--;
						break;
					}
					case CONTROL_RIGHT:
					{
						if(choise < BANDOLIER - 1) choise++;
						break;
					}
					case CONTROL_EXCHANGE:
					{
						if(buffer.type != ItemEmpty)
						{
							inventory[pos] = inventory[AMMO_SLOT + choise];
							inventory[AMMO_SLOT + choise] = buffer;
							buffer.type = ItemEmpty;
						}
						else
						{
							buffer = inventory[AMMO_SLOT + choise];
							inventory[AMMO_SLOT + choise].type = ItemEmpty;
							pos = AMMO_SLOT + choise;
						}
						break;
					}
					case '\033':
					{
						if(buffer.type != ItemEmpty)
						{
							inventory[pos].type = ItemAmmo;
							buffer.type = ItemEmpty;
						}
						return;
						break;
					}
				}
			}
			break;
		}
		case CONTROL_RELOAD:
		{
			ClearRightPane();
			move(0, Length + 10);
			printw("Now you can load your weapon");
			while(1)
			{
				for(int i = 0; i < heroWeapon->item.invWeapon.cartridgeSize; i++)
				{
					move(1, 10 + Length + i);
					if(heroWeapon->item.invWeapon.cartridge[i].count == 1)
					{
						switch(heroWeapon->item.invWeapon.cartridge[i].symbol)
						{
							case 450:
								addch('i' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								break;
							case 451:
								addch('i' | COLOR_PAIR(RED_BLACK) | LIGHT);
								break;
							default:
								addch('?');
						}
					}
					else
					{
						addch('_');
					}
				}
				
				string load_string = "";
				
				for(int i = 0; i < BANDOLIER; i++)
				{
					int ac = inventory[AMMO_SLOT + i].item.invArmor.count;
					move(2, 10 + Length);
					sprintf(tmp, "[%i|", i + 1);
					load_string += tmp;
					if(inventory[AMMO_SLOT + i].type != ItemEmpty)
					{
	//					sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invArmor.count);
	//					load_string += tmp;
						switch(inventory[AMMO_SLOT + i].GetItem().symbol)
						{
							case 450:
								load_string += " steel bullets ";
								break;
							case 451:
								load_string += " shotgun shells ";
								break;
							default:
								load_string += " omgwth? ";
						}
						load_string += "]";
					}
					else load_string += " nothing ]";
				}
				
				load_string += "   [u] - unload ";
				
				printw("%s", load_string.c_str());
				
				char in = getch();
				if(in == '\033') return;
				if(in == 'u')
				{
					bool found = false;
					for(int j = 0; j < BANDOLIER; j++)
					{
						if(inventory[AMMO_SLOT + j].GetItem().symbol == 
								heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].symbol)
						{
							inventory[AMMO_SLOT + j].item.invAmmo.count++;
							heroWeapon->item.invWeapon.currentCS--;
							found = true;
							break;
						}
					}
					if(!found)
					{
						for(int j = 0; j < BANDOLIER; j++)
						{
							if(inventory[AMMO_SLOT + j].type == ItemEmpty)
							{
								inventory[AMMO_SLOT + j].item.invAmmo = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1];
								inventory[AMMO_SLOT + j].type = ItemAmmo;
								heroWeapon->item.invWeapon.currentCS--;
								found = true;
								break;
							}
						}
					}
					if(!found)
					{
						int empty = FindEmptyItemUnderThisCell(posH, posL);
						if(empty != 101010)
						{
								ItemsMap[posH][posL][empty].item.invAmmo = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1];
								ItemsMap[posH][posL][empty].type = ItemAmmo;
								heroWeapon->item.invWeapon.currentCS--;
								found = true;
						}
					}
					if(!found)
					{
						message += "You can`t unload your weapon";
					}
					sprintf(tmp, "! f:%i, cs:%i !", (int)found, heroWeapon->item.invWeapon.currentCS - 1);
					message += tmp;
				}
				else
				{
					int intin = in - '1';
					if(inventory[AMMO_SLOT + intin].type != ItemEmpty)
					{
						if(heroWeapon->item.invWeapon.currentCS >= heroWeapon->item.invWeapon.cartridgeSize)
						{
							message += "Weapon is loaded ";
							return;
						}
						heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS] = inventory[AMMO_SLOT + intin].item.invAmmo;
						heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS].count = 1;
						heroWeapon->item.invWeapon.currentCS++;
						if(inventory[AMMO_SLOT + intin].item.invAmmo.count > 1) inventory[AMMO_SLOT + intin].item.invAmmo.count --;
						else inventory[AMMO_SLOT + intin].type = ItemEmpty;
					}
				}
			}
			break;	/*!!!FUCK!!!*/// Add unload option
		}
	}
}

void DropInventory(PossibleUnit& unit)
{
	int h = unit.GetUnit().posH;
	int l =	unit.GetUnit().posL;
	for(int i = 0; i < UNITINVENTORY; i++)
	{
		if(unit.GetUnit().unitInventory[i].type != ItemEmpty)
		{
			int empty = hero.FindEmptyItemUnderThisCell(h, l);
			if(empty != 101010)
			{
				ItemsMap[h][l][empty] = unit.GetUnit().unitInventory[i];
/*				if(unit.GetUnit().unitInventory[i].GetItem().isStackable)
				{
					
				}
				for(int j = 0; j < Depth; j++)
				{

				}*/ /*!!!FUCK!!!*/
			}
		}
	}
}

void Hero::AttackEnemy(int& a1, int& a2)
{
	if(heroWeapon->type == ItemWeapon)
	{
		UnitsMap[posH + a1][posL + a2].GetUnit().health -= heroWeapon->item.invWeapon.damage;
	}
	else if(heroWeapon->type == ItemTools)
	{
		UnitsMap[posH + a1][posL + a2].GetUnit().health -= heroWeapon->item.invTools.damage;
	}
	if(UnitsMap[posH + a1][posL + a2].GetUnit().health <= 0)
	{
		DropInventory(UnitsMap[posH + a1][posL + a2]);
		UnitsMap[posH + a1][posL + a2].type = UnitEmpty;
		xp += UnitsMap[posH + a1][posL + a2].unit.uEnemy.xpIncreasing;
	}
}

void Hero::ThrowAnimated(PossibleItem& item, char direction)
{
	int ThrowLength = 0;

	switch(direction)
	{
		case CONTROL_RIGHT:
		{
			for(int i = 0; i < 12 - item.GetItem().weight / 3; i++)							// 12 is "strength"
			{
				if(map[posH][posL + i + 1] == 2) break;
				if(UnitsMap[posH][posL + i + 1].type != UnitEmpty)
				{
					UnitsMap[posH][posL + i + 1].GetUnit().health -= item.GetItem().weight / 2;
					if(UnitsMap[posH][posL + i + 1].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH][posL + i + 1]);
						UnitsMap[posH][posL + i + 1].type = UnitEmpty;
						xp += UnitsMap[posH][posL + i + 1].unit.uEnemy.xpIncreasing;
					}
					break;
				}
				move(posH, posL + i + 1);
				addch('-');
				refresh();
				ThrowLength++;
				Delay(DELAY);
			}
			int empty = FindEmptyItemUnderThisCell(posH, posL + ThrowLength);
			if(empty == 101010)
			{
				int empty2 = FindEmptyItemUnderThisCell(posH, posL + ThrowLength - 1);
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
			for(int i = 0; i < 12 - item.GetItem().weight / 3; i++)
			{
				if(map[posH][posL - i - 1] == 2) break;
				if(UnitsMap[posH][posL - i - 1].type != UnitEmpty)
				{
					UnitsMap[posH][posL - i - 1].GetUnit().health -= item.GetItem().weight / 2;
					if(UnitsMap[posH][posL - i - 1].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH][posL - i - 1]);
						UnitsMap[posH][posL - i - 1].type = UnitEmpty;
						xp += UnitsMap[posH][posL - i - 1].unit.uEnemy.xpIncreasing;
					}
					break;
				}
				move(posH, posL - i - 1);
				addch('-');
				refresh();
				ThrowLength++;
				Delay(DELAY);
			}
			int empty = FindEmptyItemUnderThisCell(posH, posL - ThrowLength);
			if(empty == 101010)
			{
				int empty2 = FindEmptyItemUnderThisCell(posH, posL - ThrowLength + 1);
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
			for(int i = 0; i < 12 - item.GetItem().weight / 3; i++)
			{
				if(map[posH - i - 1][posL] == 2) break;
				if(UnitsMap[posH - i - 1][posL].type != UnitEmpty)
				{
					UnitsMap[posH - i - 1][posL].GetUnit().health -= item.GetItem().weight / 2;
					if(UnitsMap[posH - i - 1][posL].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH - i - 1][posL]);
						UnitsMap[posH - i - 1][posL].type = UnitEmpty;
						xp += UnitsMap[posH - i - 1][posL].unit.uEnemy.xpIncreasing;
					}
					break;
				}
				move(posH - i - 1, posL);
				addch('|');
				refresh();
				ThrowLength++;
				Delay(DELAY);
			}
			int empty = FindEmptyItemUnderThisCell(posH - ThrowLength, posL);
			if(empty == 101010)
			{
				int empty2 = FindEmptyItemUnderThisCell(posH - ThrowLength + 1, posL);
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
			for(int i = 0; i < 12 - item.GetItem().weight / 3; i++)
			{
				if(map[posH + i + 1][posL] == 2) break;
				if(UnitsMap[posH + i + 1][posL].type != UnitEmpty)
				{
					UnitsMap[posH + i + 1][posL].GetUnit().health -= item.GetItem().weight / 2;
					if(UnitsMap[posH + i + 1][posL].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH + i + 1][posL]);
						UnitsMap[posH + i + 1][posL].type = UnitEmpty;
						xp += UnitsMap[posH + 1 + i][posL].unit.uEnemy.xpIncreasing;
					}
					break;
				}
				move(posH + i + 1, posL);
				addch('|');
				refresh();
				ThrowLength++;
				Delay(DELAY);
			}
			int empty = FindEmptyItemUnderThisCell(posH + ThrowLength, posL);
			if(empty == 101010)
			{
				int empty2 = FindEmptyItemUnderThisCell(posH + ThrowLength - 1, posL);
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

void Hero::Shoot()
{
	if(heroWeapon->item.invWeapon.Ranged == false)
	{
		message += "You have no ranged weapon in hands. ";
		return;
	}
	if(heroWeapon->item.invWeapon.currentCS == 0)
	{
		message += "You have no bullets. ";
		Stop = true;
		return;
	}
	move(0, Length + 10);
	printw("In what direction? ");
	char choise = getch();
	if(choise != CONTROL_UP 
			&& choise != CONTROL_DOWN 
			&& choise != CONTROL_LEFT 
			&& choise != CONTROL_RIGHT 
			&& choise != CONTROL_UPLEFT 
			&& choise != CONTROL_UPRIGHT 
			&& choise != CONTROL_DOWNLEFT
			&& choise != CONTROL_DOWNRIGHT)
	{
		Stop = true;
		return;
	}
	int bullet_power = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].damage + hero.heroWeapon->item.invWeapon.damageBonus;
	switch(choise)
	{
		case CONTROL_LEFT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++)
			{
				if(map[posH][posL - i] == 2) break;
				if(UnitsMap[posH][posL - i].type != UnitEmpty)
				{
					UnitsMap[posH][posL - i].GetUnit().health -= bullet_power - i / 3;
					if(UnitsMap[posH][posL - i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH][posL - i]);
						UnitsMap[posH][posL - i].type = UnitEmpty;
						xp += UnitsMap[posH][posL - i].unit.uEnemy.xpIncreasing;
					}
				}
				move(posH, posL - i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_DOWN:
		{	
			for(int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++)
			{
				if(map[posH + i][posL] == 2) break;
				if(UnitsMap[posH + i][posL].type != UnitEmpty)
				{
					UnitsMap[posH + i][posL].GetUnit().health -= bullet_power - i / 3;
					if(UnitsMap[posH + i][posL].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH + i][posL]);
						UnitsMap[posH + i][posL].type = UnitEmpty;
						xp += UnitsMap[posH + i][posL].unit.uEnemy.xpIncreasing;
					}
				}
				move(posH + i, posL);
				addch('|');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_UP:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++)
			{
				if(map[posH - i][posL] == 2) break;
				if(UnitsMap[posH - i][posL].type != UnitEmpty)
				{
					UnitsMap[posH - i][posL].GetUnit().health -= bullet_power - i / 3;
					if(UnitsMap[posH - i][posL].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH - i][posL]);
						UnitsMap[posH - i][posL].type = UnitEmpty;
						xp += UnitsMap[posH - i][posL].unit.uEnemy.xpIncreasing;
					}
				}
				move(posH - i, posL);
				addch('|');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_RIGHT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++)
			{
				if(map[posH][posL + i] == 2) break;
				if(UnitsMap[posH][posL + i].type != UnitEmpty)
				{
					UnitsMap[posH][posL + i].GetUnit().health -= bullet_power - i / 3;
					if(UnitsMap[posH][posL + i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH][posL + i]);
						UnitsMap[posH][posL + i].type = UnitEmpty;
						xp += UnitsMap[posH][posL + i].unit.uEnemy.xpIncreasing;
					}
				}
				move(posH, posL + i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_UPLEFT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++)
			{
				if(map[posH - i][posL - i] == 2) break;
				if(UnitsMap[posH - i][posL - i].type != UnitEmpty)
				{
					UnitsMap[posH - i][posL - i].GetUnit().health -= bullet_power - i / 3;
					if(UnitsMap[posH - i][posL - i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH - i][posL - i]);
						UnitsMap[posH - i][posL - i].type = UnitEmpty;
						xp += UnitsMap[posH - i][posL - i].unit.uEnemy.xpIncreasing;
					}
				}
				move(posH - i, posL - i);
				addch('\\');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_UPRIGHT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++)
			{
				if(map[posH - i][posL + i] == 2) break;
				if(UnitsMap[posH - i][posL + i].type != UnitEmpty)
				{
					UnitsMap[posH - i][posL + i].GetUnit().health -= bullet_power - i / 3;
					if(UnitsMap[posH - i][posL + i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH - i][posL + i]);
						UnitsMap[posH - i][posL + i].type = UnitEmpty;
						xp += UnitsMap[posH - i][posL + i].unit.uEnemy.xpIncreasing;
					}
				}
				move(posH - i, posL + i);
				addch('/');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_DOWNLEFT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++)
			{
				if(map[posH + i][posL - i] == 2) break;
				if(UnitsMap[posH + i][posL - i].type != UnitEmpty)
				{
					UnitsMap[posH + i][posL - i].GetUnit().health -= bullet_power - i / 3;
					if(UnitsMap[posH + i][posL - i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH + i][posL - i]);
						UnitsMap[posH + i][posL - i].type = UnitEmpty;
						xp += UnitsMap[posH + i][posL - i].unit.uEnemy.xpIncreasing;
					}
				}
				move(posH + i, posL - i);
				addch('/');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_DOWNRIGHT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++)
			{
				if(map[posH + i][posL + i] == 2) break;
				if(UnitsMap[posH + i][posL + i].type != UnitEmpty)
				{
					UnitsMap[posH + i][posL + i].GetUnit().health -= bullet_power - i / 3;
					if(UnitsMap[posH + i][posL + i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH + i][posL + i]);
						UnitsMap[posH + i][posL + i].type = UnitEmpty;
						xp += UnitsMap[posH + i][posL + i].unit.uEnemy.xpIncreasing;
					}
				}
				move(posH + i, posL + i);
				addch('\\');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
	}
	heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count = 0;
	heroWeapon->item.invWeapon.currentCS--;
}

void Hero::mHLogic(int& a1, int& a2)
{
	if(map[posH + a1][posL + a2] != 2 || 
			(map[posH + a1][posL + a2] == 2 && CanHeroMoveThroughWalls) && 
			(posH + a1 > 0 && posH + a1 < Height - 1 && posL + a2 > 0 && posL + a2 < Length - 1))
	{
		if(UnitsMap[posH + a1][posL + a2].type == UnitEmpty)
		{
			UnitsMap[posH + a1][posL + a2] = UnitsMap[posH][posL];
			UnitsMap[posH][posL].type = UnitEmpty;
			posH += a1;
			posL += a2;
		}
		else if(UnitsMap[posH + a1][posL + a2].type == UnitEnemy)
		{
			AttackEnemy(a1, a2);
		}
	}
	else if(map[posH + a1][posL + a2] == 2)
	{
		if(heroWeapon->type == ItemTools)
		{
			if(heroWeapon->item.invTools.possibility == 1)
			{
				move(0, Length + 10);
				printw("Do you want to dig this wall (y or n)? ");
				char inpChar = getch();
				if(inpChar == 'y' || inpChar == 'Y')
				{
					map[posH + a1][posL + a2] = 1;
					heroWeapon->item.invTools.uses--;
					if(heroWeapon->item.invTools.uses <= 0)
					{
						sprintf(tmp, "Your %s is broken. ", heroWeapon->GetItem().GetName());
						message += tmp;
						heroWeapon->type = ItemEmpty;
						FindVisibleArray();
					}
					return;
				}
			}
		}
		message += "The wall is the way. ";
		Stop = true;
	}
	FindVisibleArray();
}

void Enemy::Shoot()
{
	if(posH == hero.posH && posL < hero.posL) dir = DIR_RIGHT;
	else if(posH == hero.posH && posL > hero.posL) dir = DIR_LEFT;
	else if(posL == hero.posL && posH > hero.posH) dir = DIR_UP;
	else if(posL == hero.posL && posH < hero.posH) dir = DIR_DOWN;
	else if(posL > hero.posL && posH > hero.posH) dir = DIR_UPLEFT;
	else if(posL > hero.posL && posH < hero.posH) dir = DIR_DOWNLEFT;
	else if(posL < hero.posL && posH < hero.posH) dir = DIR_DOWNRIGHT;
	else if(posL < hero.posL && posH > hero.posH) dir = DIR_UPRIGHT;

	switch(dir)
	{
		case DIR_LEFT:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH][posL - i] == 2) break;
				if(UnitsMap[posH][posL - i].type == UnitHero)
				{
					hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
					break;
				}
				move(posH, posL - i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_DOWN:
		{	
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH + i][posL] == 2) break;
				if(UnitsMap[posH + i][posL].type == UnitHero)
				{
					hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
					break;
				}
				move(posH + i, posL);
				addch('|');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_UP:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH - i][posL] == 2) break;
				if(UnitsMap[posH - i][posL].type == UnitHero)
				{
					hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
					break;
				}
				move(posH - i, posL);
				addch('|');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_RIGHT:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH][posL + i] == 2) break;
				if(UnitsMap[posH][posL + i].type == UnitHero)
				{
					hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
					break;
				}
				move(posH, posL + i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_UPLEFT:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH - i][posL - i] == 2) break;
				if(UnitsMap[posH - i][posL - i].type == UnitHero)
				{
					hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
					break;
				}
				move(posH - i, posL - i);
				addch('\\');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_UPRIGHT:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH - i][posL + i] == 2) break;
				if(UnitsMap[posH - i][posL + i].type == UnitHero)
				{
					hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
					break;
				}
				move(posH - i, posL + i);
				addch('/');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_DOWNLEFT:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH + i][posL - i] == 2) break;
				if(UnitsMap[posH + i][posL - i].type == UnitHero)
				{
					hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
					break;
				}
				move(posH + i, posL - i);
				addch('/');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_DOWNRIGHT:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH + i][posL + i] == 2) break;
				if(UnitsMap[posH + i][posL + i].type == UnitHero)
				{
					hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
					break;
				}
				move(posH + i, posL + i);
				addch('\\');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
	}
	unitAmmo->item.invAmmo.count--;
	if(unitAmmo->item.invAmmo.count <= 0)
	{
		unitAmmo->type = ItemEmpty;
	}
}

int bfs(int targetH, int targetL, int h, int l, int &posH, int &posL)
{
	int depth = 2 + ABS(targetH - h) + ABS(targetL - l);						// <- smth a little bit strange
	queue<int> x, y;
	x.push(l);
	y.push(h);
	int used[Height][Length] = {};
	used[h][l] = true;
	while(!x.empty() && !y.empty())
	{
		int v_x = x.front();
		int v_y = y.front();
		if(v_y == targetH && v_x == targetL) break;
		if( used[ v_y ][ v_x ] > depth )
		{
			return -1;
		}
		x.pop();
		y.pop();
	
		if(v_y < Height - 1 && !used[v_y + 1][v_x] && (UnitsMap[v_y + 1][v_x].type == UnitEmpty 
			|| UnitsMap[v_y + 1][v_x].type == UnitHero) && map[v_y + 1][v_x] != 2)
		{
			y.push(v_y + 1);
			x.push(v_x);
			used[v_y + 1][v_x] = 1 + used[v_y][v_x];
		}
		if(v_y > 0 && !used[v_y - 1][v_x] && (UnitsMap[v_y - 1][v_x].type == UnitEmpty 
			|| UnitsMap[v_y - 1][v_x].type == UnitHero) && map[v_y - 1][v_x] != 2)
		{
			y.push(v_y - 1);
			x.push(v_x);	
			used[v_y - 1][v_x] = 1 + used[v_y][v_x];
		}
		if(v_x < Length - 1 && !used[v_y][v_x + 1] && (UnitsMap[v_y][v_x + 1].type == UnitEmpty 
			|| UnitsMap[v_y][v_x + 1].type == UnitHero) && map[v_y][v_x + 1] != 2)
		{
			y.push(v_y);
			x.push(v_x + 1);
			used[v_y][v_x + 1] = 1 + used[v_y][v_x];
		}
		if(v_x > 0 && !used[v_y][v_x - 1] && (UnitsMap[v_y][v_x - 1].type == UnitEmpty 
			|| UnitsMap[v_y][v_x - 1].type == UnitHero) && map[v_y][v_x - 1] != 2)
		{
			y.push(v_y);
			x.push(v_x - 1);
			used[v_y][v_x - 1] = 1 + used[v_y][v_x];	
		}
		if( MODE == 2 )
		{
			if( v_y < Height - 1 )
			{
				if( v_x && !used[v_y + 1][v_x - 1] && (UnitsMap[v_y + 1][v_x - 1].type == UnitEmpty 
					|| UnitsMap[v_y + 1][v_x - 1].type == UnitHero) && map[v_y + 1][v_x - 1] != 2 )
				{
					y.push( v_y + 1 );
					x.push( v_x - 1 );
					used[v_y + 1][v_x - 1] = 1 + used[v_y][v_x];
				}
				if( v_x < Length - 1 && !used[v_y + 1][v_x + 1] && (UnitsMap[v_y + 1][v_x + 1].type == UnitEmpty 
					|| UnitsMap[v_y + 1][v_x + 1].type == UnitHero) && map[v_y + 1][v_x + 1] != 2 )
				{ 
					y.push( v_y + 1 );
					x.push( v_x + 1 );
					used[v_y + 1][v_x + 1] = 1 + used[v_y][v_x];
				}
			}
			if( v_y )
			{
				if( v_x && !used[v_y - 1][v_x - 1] && (UnitsMap[v_y - 1][v_x - 1].type == UnitEmpty 
					|| UnitsMap[v_y - 1][v_x - 1].type == UnitHero) && map[v_y - 1][v_x - 1] != 2 )
				{
					y.push( v_y - 1 );
					x.push( v_x - 1 );
					used[v_y - 1][v_x - 1] = 1 + used[v_y][v_x];
				}
				if( v_x < Length - 1 && !used[v_y - 1][v_x + 1] && (UnitsMap[v_y - 1][v_x + 1].type == UnitEmpty 
					|| UnitsMap[v_y - 1][v_x + 1].type == UnitHero) && map[v_y - 1][v_x + 1] != 2 )
				{
					y.push( v_y - 1 );
					x.push( v_x + 1 );
					used[v_y - 1][v_x + 1] = 1 + used[v_y][v_x];
				}
			}
		}
	}

	if( !used[ targetH ][ targetL ] )
	{
		return -1;
	}
	int v_y = targetH, v_x = targetL;
	while( used[ v_y ][ v_x ] != 2 )
	{
		if( MODE == 2 )
		{
			if( v_y && v_x && used[ v_y - 1 ][ v_x - 1 ] + 1 == used[ v_y ][ v_x ] )
			{
				--v_y;
				--v_x;
				continue;
			}
			if( v_y && v_x < Length - 1 && used[ v_y - 1 ][ v_x + 1 ] + 1 == used[ v_y ][ v_x ] )
			{
				--v_y;
				++v_x;
				continue;
			}
			if( v_y < Height - 1 && v_x && used[ v_y + 1 ][ v_x - 1 ] + 1 == used[ v_y ][ v_x ] )
			{
				++v_y;
				--v_x;
				continue;
			}
			if( v_y < Height - 1 && v_x < Length - 1 && used[ v_y + 1 ][ v_x + 1 ] + 1 == used[ v_y ][ v_x ] )
			{
				++v_y;
				++v_x;
				continue;
			}
		}
		if( v_y && used[ v_y - 1 ][ v_x ] + 1 == used[ v_y ][ v_x ] )
		{
			--v_y;
			continue;
		}
		if( v_x && used[ v_y ][ v_x - 1 ] + 1 == used[ v_y ][ v_x ] )
		{
			--v_x;
			continue;
		}
		if( v_y < Height - 1 && used[ v_y + 1 ][ v_x ] + 1 == used[ v_y ][ v_x ] )
		{
			++v_y;
			continue;
		}
		if( v_x < Length - 1 && used[ v_y ][ v_x + 1 ] + 1 == used[ v_y ][ v_x ] )
		{
			++v_x;
			continue;
		}
	}

	posH = v_y;
	posL = v_x;

	/* Lol, i finished this BFS for you, but im too lazy & busy to continue, so you must finish the AI by yourself */
	/* Bitch bitch bitch, motherfuck'a, fuk yeah */
}

void UpdatePosition(PossibleUnit& unit)
{
	bool HeroVisible = false;

	if(INVISIBILITY > 0)
	{
		HeroVisible = false;
	}
	else if((SQR(unit.GetUnit().posH - hero.posH) + SQR(unit.GetUnit().posL - hero.posL) < SQR(unit.GetUnit().vision)) && unit.GetUnit().CanSeeCell(hero.posH, hero.posL))
	{
		HeroVisible = true;
	}
	
	int pH = 1, pL = 1;

	if(HeroVisible)
	{
		if((unit.GetUnit().posH == hero.posH || 
				unit.GetUnit().posL == hero.posL || 
				ABS(hero.posH - unit.GetUnit().posH) == ABS(hero.posL - unit.GetUnit().posL)) && 
				unit.GetUnit().unitWeapon->item.invWeapon.Ranged == true &&
				unit.GetUnit().unitWeapon->item.invWeapon.range + unit.GetUnit().unitAmmo->item.invAmmo.range >= 
				ABS(hero.posH - unit.GetUnit().posH) + ABS(hero.posL - unit.GetUnit().posL))
		{
			unit.unit.uEnemy.Shoot();
		}
		else
		{
			unit.unit.uEnemy.targetH = hero.posH;
			unit.unit.uEnemy.targetL = hero.posL;

			if( bfs(hero.posH, hero.posL, unit.GetUnit().posH, unit.GetUnit().posL, pH, pL) == -1 )
			{
				HeroVisible = false;
			}
			else
			{
				if(UnitsMap[pH][pL].type == UnitEnemy)
				{
					return;
				}
				else if(UnitsMap[pH][pL].type == UnitHero)
				{
					if(unit.GetUnit().unitWeapon->type == ItemWeapon)
					{
						if(hero.heroArmor->item.invArmor.mdf != 2)
						{
							hero.health -= unit.GetUnit().unitWeapon->item.invWeapon.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
						}
						else
						{
							unit.GetUnit().health -= unit.GetUnit().unitWeapon->item.invWeapon.damage;
						}
					}
					else if(unit.GetUnit().unitWeapon->type == ItemTools)
					{
						if(hero.heroArmor->item.invArmor.mdf != 2)
						{
							hero.health -= unit.GetUnit().unitWeapon->item.invTools.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
						}
						else
						{
							unit.GetUnit().health -= unit.GetUnit().unitWeapon->item.invTools.damage;
						}
					}
					if(unit.GetUnit().health <= 0)
					{
						unit.type = UnitEmpty;
					}
				}
				else
				{
					unit.GetUnit().posH = pH;
					unit.GetUnit().posL = pL;
					UnitsMap[pH][pL] = unit;
					unit.type = UnitEmpty;
				}
			}
		}
	}
	if( !HeroVisible )
	{
		bool needRandDir = 0;
		if(unit.unit.uEnemy.targetH != -1 && (unit.unit.uEnemy.targetH != unit.GetUnit().posH || unit.unit.uEnemy.targetL != unit.GetUnit().posL))
		{

			if( bfs(unit.unit.uEnemy.targetH, unit.unit.uEnemy.targetL, unit.unit.uEnemy.posH, unit.unit.uEnemy.posL, pH, pL) == -1 )
			{
				needRandDir = 1;
				/*sprintf( tmp, " $$>yet another bfs error (%d;%d;%d)<$$ ", unit.GetUnit().symbol, unit.GetUnit().posH, unit.GetUnit().posL );
				message += tmp;*/
			}
			else
			{
				if(pH < Height && pH > 0 && pL < Length && pL > 0)
				{
					if(UnitsMap[pH][pL].type == UnitHero)
					{
						if(unit.GetUnit().unitWeapon->type == ItemWeapon)
						{
							if(hero.heroArmor->item.invArmor.mdf != 2)
							{
								hero.health -= unit.GetUnit().unitWeapon->item.invWeapon.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
							}
							else
							{
								unit.GetUnit().health -= unit.GetUnit().unitWeapon->item.invWeapon.damage;
							}
						}
						else if(unit.GetUnit().unitWeapon->type == ItemTools)
						{
							if(hero.heroArmor->item.invArmor.mdf != 2)
							{
								hero.health -= unit.GetUnit().unitWeapon->item.invTools.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
							}
							else
							{
								unit.GetUnit().health -= unit.GetUnit().unitWeapon->item.invTools.damage;
							}
						}
						if(unit.GetUnit().health <= 0)
						{
							unit.type = UnitEmpty;
						}
					}
					else
					{
						unit.GetUnit().posH = pH;
						unit.GetUnit().posL = pL;
						UnitsMap[pH][pL] = unit;
						unit.type = UnitEmpty;
					}
				}
			}
		}
		else
		{
			needRandDir = 1;
		}
		if( needRandDir )
		{
//			return;
			vector<int> visionArrayH;
			vector<int> visionArrayL;

			int psH = unit.GetUnit().posH, psL = unit.GetUnit().posL, vis = unit.GetUnit().vision;

			for(int i = MAX(psH - vis, 0); i < MIN(Height, psH + vis); i++)
			{
				for(int j = MAX(psL - vis, 0); j < MIN(psL + vis, Length); j++)
				{
					if((i != psH || j != psL )
					&& SQR(psH - i) + SQR(psL - j) < SQR(vis) && map[i][j] != 2 
					&& UnitsMap[i][j].type == UnitEmpty && unit.GetUnit().CanSeeCell(i, j))
					{
						visionArrayH.push_back(i);
						visionArrayL.push_back(j);
					}
				}	
			}
			while( 1 )
			{
				int r; 
				int rposH = visionArrayH[r = (rand() % visionArrayH.size())];
				int rposL = visionArrayL[r];
				
				unit.unit.uEnemy.targetH = rposH;
				unit.unit.uEnemy.targetL = rposL;

				if(bfs(unit.unit.uEnemy.targetH, unit.unit.uEnemy.targetL, unit.unit.uEnemy.posH, unit.unit.uEnemy.posL, pH, pL) == -1)
				{
					continue;
				}
				if(pH < Height && pH > 0 && pL < Length && pL > 0)
				{
					if(UnitsMap[pH][pL].type == UnitHero)
					{
						if(unit.GetUnit().unitWeapon->type == ItemWeapon)
						{
							if(hero.heroArmor->item.invArmor.mdf != 2)
							{
								hero.health -= unit.GetUnit().unitWeapon->item.invWeapon.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
							}
							else
							{
								unit.GetUnit().health -= unit.GetUnit().unitWeapon->item.invWeapon.damage;
							}
						}
						else if(unit.GetUnit().unitWeapon->type == ItemTools)
						{
							if(hero.heroArmor->item.invArmor.mdf != 2)
							{
								hero.health -= unit.GetUnit().unitWeapon->item.invTools.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
							}
							else
							{
								unit.GetUnit().health -= unit.GetUnit().unitWeapon->item.invTools.damage;
							}
						}
						if(unit.GetUnit().health <= 0)
						{
							unit.type = UnitEmpty;
						}
					}
					else
					{
						unit.GetUnit().posH = pH;
						unit.GetUnit().posL = pL;
						UnitsMap[pH][pL] = unit;
						unit.type = UnitEmpty;
					}
				}
				break;
				/* Here ^ is random moving */
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
			if(UnitsMap[i][j].type == UnitEnemy && UnitsMap[i][j].unit.uEnemy.movedOnTurn != turns)
			{
#				ifdef DEBUG
				sprintf(tmp, "{%i|%i|%i|%i}", i, j, UnitsMap[i][j].unit.uEnemy.symbol, UnitsMap[i][j].GetUnit().health);
				message += tmp;
#				endif
				UnitsMap[i][j].unit.uEnemy.movedOnTurn = turns;
				UpdatePosition(UnitsMap[i][j]);
			}
		}
	}
}

void SetItems()
{

	for(int i = 0; i < FOODCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfFood;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentFood[p];
			ItemsMap[h][l][d].GetItem().posH = h;
			ItemsMap[h][l][d].GetItem().posL = l;
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
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentArmor[p];
			ItemsMap[h][l][d].GetItem().posH = h;
			ItemsMap[h][l][d].GetItem().posL = l;
			if(rand() % 500 / Luck == 0)ItemsMap[h][l][d].item.invArmor.mdf = 2;
		}
		else i--;
	}
	
	for(int i = 0; i < WEAPONCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		
		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfWeapon;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentWeapon[p];
			ItemsMap[h][l][d].GetItem().posH = h;
			ItemsMap[h][l][d].GetItem().posL = l;
		}
		else i--;
	}

	for(int i = 0; i < AMMOCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		
		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfAmmo;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentAmmo[p];
			ItemsMap[h][l][d].GetItem().posH = h;
			ItemsMap[h][l][d].GetItem().posL = l;
			ItemsMap[h][l][d].item.invAmmo.count = (rand() % Luck) / 2 + 1;
		}
		else i--;
	}

	for(int i = 0; i < SCROLLCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfScroll;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentScroll[p];
			ItemsMap[h][l][d].GetItem().posH = h;
			ItemsMap[h][l][d].GetItem().posL = l;
		}
	}
	for(int i = 0; i < POTIONCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfPotion;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentPotion[p];
			ItemsMap[h][l][d].GetItem().posH = h;
			ItemsMap[h][l][d].GetItem().posL = l;
		}
	}
	for(int i = 0; i < TOOLSCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfTools;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentTools[p];
			ItemsMap[h][l][d].GetItem().posH = h;
			ItemsMap[h][l][d].GetItem().posL = l;
		}
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
			UnitsMap[h][l] = differentEnemies[p];
			UnitsMap[h][l].GetUnit().posH = h;
			UnitsMap[h][l].GetUnit().posL = l;
//			sprintf(tmp, ">%i<", UnitsMap[h][l].GetUnit().unitInventory[0].GetItem().symbol);
//			message += tmp;
		}
		else i--;
	}
}

#ifdef DEBUG

void Draw(){
	
	move(0, 0);

	static int mapSaved[FIELD_ROWS][FIELD_COLS] = {};

	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			mapSaved[i][j] = map[i][j];
		}
	}
	
	for(int i = 0; i < Height; i++){
		
		for(int j = 0; j < Length; j++){
			
//			printw("% i ", map[i][j]);										// !DEBUG!
			if(mapSaved[i][j] != 0)
			{
				bool near = abs(i - hero.posH) <= 1 && abs(j - hero.posL) <= 1;
				
/* Here */			if(hero.FindItemsCountUnderThisCell(i, j) == 0 && UnitsMap[i][j].type == UnitEmpty)
				{
					switch(mapSaved[i][j])
					{
						case 1:
							if(seenUpdated[i][j])
							{
								addch(ACS_BULLET);
							}
							else
							{
								//addch('_' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								addch(' ');
							}
							break;
						case 2:
							/*
							if(near)
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							}
							else
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK));
							}
							*/
							{
								bool u = (i && mapSaved[i - 1][j] == 2);
								bool r = (j < Length - 1 && mapSaved[i][j + 1] == 2);
								bool d = (i < Height - 1 && mapSaved[i + 1][j] == 2);
								bool l = (j && mapSaved[i][j - 1] == 2);
								int count = u + r + d + l;
								int attrib = COLOR_PAIR(WHITE_BLACK) | (LIGHT * seenUpdated[i][j]);
								switch (count) {
								case 0:
								case 4:
									addch(ACS_PLUS | attrib);
									break;
								case 1:
									if (u || d)
										addch(ACS_VLINE | attrib);
									else
										addch(ACS_HLINE | attrib);
									break;
								case 2:
									if (u && r)
										addch(ACS_LLCORNER | attrib);
									else if (u && d)
										addch(ACS_VLINE | attrib);
									else if (u && l)
										addch(ACS_LRCORNER | attrib);
									else if (r && d)
										addch(ACS_ULCORNER | attrib);
									else if (r && l)
										addch(ACS_HLINE | attrib);
									else
										addch(ACS_URCORNER | attrib);
									break;
								case 3:
									if (!u)
										addch(ACS_TTEE | attrib);
									else if (!r)
										addch(ACS_RTEE | attrib);
									else if (!d)
										addch(ACS_BTEE | attrib);
									else
										addch(ACS_LTEE | attrib);
									break;
								}
								break;
							}
					}
				}
/* Here */			else if(hero.FindItemsCountUnderThisCell(i, j) == 1 && UnitsMap[i][j].type == UnitEmpty)
				{
					int MeetedItem = hero.FindNotEmptyItemUnderThisCell(i, j);
					switch(ItemsMap[i][j][MeetedItem].GetItem().symbol){

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
							addch('/' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							break;
						case 403:
							addch('/' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;
						case 404:
							addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 405:
							addch('/' | COLOR_PAIR(WHITE_BLACK));
							break;
						case 450:
							addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
							break;
						case 451:
							addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
							break;
						case 500:
							addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;
						case 501:
							addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;
						case 600:
							addch('!' | COLOR_PAIR(BLUE_BLACK) | LIGHT);
							break;
						case 601:
							addch('!' | COLOR_PAIR(GREEN_BLACK));
							break;
						case 602:
							addch('!' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 603:
							addch('!' | COLOR_PAIR(MAGENTA_BLACK) | LIGHT);
							break;
						case 604:
							addch('!' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 700:
							addch('\\' | COLOR_PAIR(YELLOW_BLACK));
							break;
					}
				}
/* Here */			else if(hero.FindItemsCountUnderThisCell(i, j) > 1 && UnitsMap[i][j].type == UnitEmpty)
				{
					addch('^' | COLOR_PAIR(BLACK_WHITE) | LIGHT);
				}
/* Here */			if(UnitsMap[i][j].type == UnitHero)
				{
					addch('@' | COLOR_PAIR(GREEN_BLACK));
				}
/* Here */			else if(UnitsMap[i][j].type == UnitEnemy)
				{
/* Here */				switch(UnitsMap[i][j].GetUnit().symbol)
					{
						case 201:
							addch('@' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 202:
							addch('@' | COLOR_PAIR(GREEN_BLACK) | LIGHT);
							break;
						case 203:
							addch('@' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
					}
				}
			}
			else
			{
				addch(' ');	
			}	
					
		}
		
		printw("\n");
		
	}

}

#else

void Draw(){
	
	move(0, 0);

	static int mapSaved[FIELD_ROWS][FIELD_COLS] = {};

	if(MODE == 2 && !hero.IsMapInInventory())
	{
		for(int i = 0; i < FIELD_ROWS; i++)
		{
			for(int j = 0; j < FIELD_COLS; j++)
			{
				mapSaved[i][j] = 0;
			}
		}
	}

	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			if(seenUpdated[i][j])
			{
				int itemsOnCell = hero.FindItemsCountUnderThisCell(i, j);
				if( itemsOnCell == 0 )
				{
					mapSaved[i][j] = map[i][j];
				}
				else if( itemsOnCell == 1 )
				{
					mapSaved[i][j] = ItemsMap[i][j][hero.FindNotEmptyItemUnderThisCell( i, j )].GetItem().symbol;
				}
				else
				{
					mapSaved[i][j] = 100500; // Magic constant that means 'pile'
				}
			}
		}
	}
	
	for(int i = 0; i < Height; i++){
		
		for(int j = 0; j < Length; j++){
			
//			printw("% i ", map[i][j]);										// !DEBUG!
			if(mapSaved[i][j] != 0)
			{
				bool near = abs(i - hero.posH) <= 1 && abs(j - hero.posL) <= 1;
				if( seenUpdated[i][j] )
				{
					if( UnitsMap[i][j].type == UnitEmpty )
					{
						switch( mapSaved[i][j] )
						{
						case 1:
							//if(near)
							//{
								addch(ACS_BULLET);
							//}
							//else
							//{
								//addch('_' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							//	addch(' ');
							//}
							break;
						case 2:
							/*
							if(near)
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							}
							else
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK));
							}
							*/
							{
								bool u = (i && mapSaved[i - 1][j] == 2);
								bool r = (j < Length - 1 && mapSaved[i][j + 1] == 2);
								bool d = (i < Height - 1 && mapSaved[i + 1][j] == 2);
								bool l = (j && mapSaved[i][j - 1] == 2);
								int count = u + r + d + l;
								int attrib = COLOR_PAIR(WHITE_BLACK) | (LIGHT);
								switch (count) {
								case 0:
								case 4:
									addch(ACS_PLUS | attrib);
									break;
								case 1:
									if (u || d)
										addch(ACS_VLINE | attrib);
									else
										addch(ACS_HLINE | attrib);
									break;
								case 2:
									if (u && r)
										addch(ACS_LLCORNER | attrib);
									else if (u && d)
										addch(ACS_VLINE | attrib);
									else if (u && l)
										addch(ACS_LRCORNER | attrib);
									else if (r && d)
										addch(ACS_ULCORNER | attrib);
									else if (r && l)
										addch(ACS_HLINE | attrib);
									else
										addch(ACS_URCORNER | attrib);
									break;
								case 3:
									if (!u)
										addch(ACS_TTEE | attrib);
									else if (!r)
										addch(ACS_RTEE | attrib);
									else if (!d)
										addch(ACS_BTEE | attrib);
									else
										addch(ACS_LTEE | attrib);
									break;
								}
								break;
							}/*
							case 1:
								if(seenUpdated[i][j])
								{
									addch('_');
								}
								else
								{
									addch('_' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								}
								break;
							case 2:
								if(seenUpdated[i][j])
								{
									addch('#' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
								}
								else
								{
									addch('#' | COLOR_PAIR(WHITE_BLACK));
								}
								break;*/
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
								addch('/' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
								break;
							case 403:
								addch('/' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
								break;	
							case 404:
								addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								break;
							case 405:
								addch('/' | COLOR_PAIR(WHITE_BLACK));
								break;
							case 450:
								addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
								break;
							case 451:
								addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
								break;
							case 100500:
								addch('^' | COLOR_PAIR(BLACK_WHITE) | LIGHT);
								break;
							case 500:
								addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
								break;
							case 501:
								addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
								break;
							case 600:
								addch('!' | COLOR_PAIR(BLUE_BLACK) | LIGHT);
								break;
							case 601:
								addch('!' | COLOR_PAIR(GREEN_BLACK));
								break;
							case 602:
								addch('!' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								break;
							case 603:
								addch('!' | COLOR_PAIR(MAGENTA_BLACK) | LIGHT);
								break;
							case 604:
								addch('!' | COLOR_PAIR(YELLOW_BLACK));
								break;
							case 700:
								addch('\\' | COLOR_PAIR(YELLOW_BLACK));
								break;
						}
					}
					else
					{
						if(UnitsMap[i][j].type == UnitHero)
						{
							addch('@' | COLOR_PAIR(GREEN_BLACK));
						}
						else if(UnitsMap[i][j].type == UnitEnemy && seenUpdated[i][j])
						{
							switch(UnitsMap[i][j].GetUnit().symbol)
							{
								case 201:
									addch('@' | COLOR_PAIR(YELLOW_BLACK));
									break;
								case 202:
									addch('@' | COLOR_PAIR(GREEN_BLACK) | LIGHT);
									break;
								case 203:
									addch('@' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
									break;							
							}
						}
					}
				}
				else
				{
					switch( mapSaved[i][j] )
					{
						case 1:
							//if(near)
							//{
							//	addch('_');
							//}
							//else
							//{
								//addch('_' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								addch(' ');
							//}
							break;
						case 2:
							/*
							if(near)
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							}
							else
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK));
							}
							*/
							{
								bool u = (i && mapSaved[i - 1][j] == 2);
								bool r = (j < Length - 1 && mapSaved[i][j + 1] == 2);
								bool d = (i < Height - 1 && mapSaved[i + 1][j] == 2);
								bool l = (j && mapSaved[i][j - 1] == 2);
								int count = u + r + d + l;
								int attrib = COLOR_PAIR(WHITE_BLACK)/* | (LIGHT * near)*/;
								switch (count) {
								case 0:
								case 4:
									addch(ACS_PLUS | attrib);
									break;
								case 1:
									if (u || d)
										addch(ACS_VLINE | attrib);
									else
										addch(ACS_HLINE | attrib);
									break;
								case 2:
									if (u && r)
										addch(ACS_LLCORNER | attrib);
									else if (u && d)
										addch(ACS_VLINE | attrib);
									else if (u && l)
										addch(ACS_LRCORNER | attrib);
									else if (r && d)
										addch(ACS_ULCORNER | attrib);
									else if (r && l)
										addch(ACS_HLINE | attrib);
									else
										addch(ACS_URCORNER | attrib);
									break;
								case 3:
									if (!u)
										addch(ACS_TTEE | attrib);
									else if (!r)
										addch(ACS_RTEE | attrib);
									else if (!d)
										addch(ACS_BTEE | attrib);
									else
										addch(ACS_LTEE | attrib);
									break;
								}
								break;
							}/*
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
							break;*/
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
							addch('/' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							break;
						case 403:
							addch('/' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;	
						case 404:
							addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 405:
							addch('/' | COLOR_PAIR(WHITE_BLACK));
							break;
						case 450:
							addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
							break;
						case 451:
							addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
							break;
						case 100500:
							addch('^' | COLOR_PAIR(BLACK_WHITE) | LIGHT);
							break;
						case 500:
							addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;
						case 501:
							addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;
						case 600:
							addch('!' | COLOR_PAIR(BLUE_BLACK) | LIGHT);
							break;
						case 601:
							addch('!' | COLOR_PAIR(GREEN_BLACK));
							break;
						case 602:
							addch('!' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 603:
							addch('!' | COLOR_PAIR(MAGENTA_BLACK) | LIGHT);
							break;
						case 604:
							addch('!' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 700:
							addch('\\' | COLOR_PAIR(YELLOW_BLACK));
							break;
					}
				}
			}
			else
			{
				addch(' ');	
			}	
					
		}
		
		printw("\n");
		
	}

}

#endif

void ClearScreen()
{
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 180; j++)
		{
			move(i, j);
			addch(' ');
		}
	}
}

void mSettingsMode()
{
	int SwitchMode = 1;
	while(1)
	{
		move(0, 0);
		printw("Choose mode");

		move(1, 0);
		if(SwitchMode == 1)
		{
			addch('1' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('1');
		printw(" Normal");
		
		move(2, 0);
		if(SwitchMode == 2)
		{
			addch('2' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('2');
		printw(" Hard");

		switch(getch())
		{
			case CONTROL_DOWN:
				if(SwitchMode < 2) SwitchMode++;
				break;
			case CONTROL_UP:
				if(SwitchMode > 1) SwitchMode--;
				break;
			case '\033':
				ClearScreen();
				return;
				break;
			case CONTROL_CONFIRM:
				switch(SwitchMode)
				{
					case 1:
						MODE = 1;
						break;
					case 2:
						MODE = 2;
						break;
				}
				break;
		}
		ClearScreen();
	}
}

void mSettingsMap()
{
	ClearScreen();
	move(0, 0);
	printw("Do you want to load map from file?");
	char inpChar = getch();
	if(inpChar == 'y' || inpChar == 'Y')
	{
		GenerateMap = false;
	}
	ClearScreen();
}

void mSettings()
{
	int SwitchSettings = 1;
	ClearScreen();
	while(1)
	{
		move(0, 0);
		printw("Settings");

		move(1, 0);
		if(SwitchSettings == 1)
		{
			addch('1' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('1');
		printw(" Mode");

		move(2, 0);
		if(SwitchSettings == 2)
		{
			addch('2' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('2');
		printw(" Maps");

		switch(getch())
		{
			case CONTROL_DOWN:
				if(SwitchSettings < 2) SwitchSettings ++;
				break;
			case CONTROL_UP:
				if(SwitchSettings > 1) SwitchSettings --;
				break;
			case CONTROL_CONFIRM:
			{
				switch(SwitchSettings)
				{
					case 1:
					{
						mSettingsMode();
						break;
					}
					case 2:
					{
						mSettingsMap();
						break;
					}
				}
				break;
			}
			case '\033':
			{
				MenuCondition = 0;
				ClearScreen();
				return;
				break;
			}
		}
	}
}

void MainMenu()
{
	string Tips[200];
	int TipsCount = 14;
	Tips[0] = "lol";
	Tips[1] = "kek";
	Tips[2] = "azaza";
	Tips[3] = "I fukd bugs";
	Tips[4] = "seriously? Again?";
	Tips[5] = "it's all about the.. Bugs";
	Tips[6] = "I used to fuck bugs. Now I fukd them too, but it doesn't matter";
	Tips[7] = "bugs, bugs, bugs...";
	Tips[8] = "Tip of the day";
	Tips[9] = "nice to meet you, lol";
	Tips[10] = "now with fixed AI!";
	Tips[11] = "guns suck.";
	Tips[12] = "do not touch the walls";
	Tips[13] = "maybe, some coffee?";
	Tips[14] = "by TheyDidItForLulz && Yuri12358!";

	int Switch = 1;
	int tip = rand() % TipsCount;
	while(1)
	{
		if(MenuCondition == 0)
		{
			ClearScreen();
			move(0, 0);
			printw("Welcome to RLRPG /*Tip of the day: %s*/", Tips[/*rand() % TipsCount*/tip].c_str());

			move(1, 0);
			if(Switch == 1)
			{
				addch('1' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('1');
			printw(" Start game");

			move(2, 0);
			if(Switch == 2)
			{
				addch('2' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('2');
			printw(" Settings");

			move(3, 0);
			if(Switch == 3)
			{
				addch('3' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('3');
			printw(" About");

			move(4, 0);
			if(Switch == 4)
			{
				addch('4' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('4');
			printw(" Help");

			move(5, 0);
			if(Switch == 5)
			{
				addch('5' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('5');
			printw(" Exit");

			switch(getch())
			{
				case CONTROL_DOWN:
					Switch ++;
					if(Switch > 5) Switch = 5;
					break;
				case CONTROL_UP:
					Switch --;
					if(Switch < 1) Switch = 1;
					break;
				case CONTROL_CONFIRM:
					if(Switch == 1)
					{
						MenuCondition = 1;
					}
					if(Switch == 2)
					{
						MenuCondition = 2;
					}
					if(Switch == 5)
					{
						EXIT = true;
						return;
					}
					break;
			}
		}
		else if(MenuCondition == 2)
		{
			mSettings();
		}
		else if(MenuCondition == 1)
		{
			return;
		}
	}
}

void ReadMap()
{
	FILE* file = fopen("map.me", "r");
	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			fscanf(file, "%d", &map[i][j]);
		}
	}
	fclose(file);
}

int LEVELUP = hero.level * hero.level + 4/* * hero.level + 8*/;

void GetXP()
{
	if(hero.xp > LEVELUP)
	{
		hero.level++;
		sprintf(tmp, "Now you are level %i. ", hero.level);
		message += tmp;
		MaxInvItemsWeight += MaxInvItemsWeight / 4;
		DEFAULT_HERO_HEALTH += DEFAULT_HERO_HEALTH / 4;
		hero.health = DEFAULT_HERO_HEALTH;
		LEVELUP = hero.level * hero.level + 4/* * hero.level + 8*/;
	}
}

void SetRandomPotionEffects()
{
	for(int i = 0; i < TypesOfPotion; i++)
	{
		int rv = rand() % TypesOfPotion;
		if(differentPotion[rv].effect == 0)
		{
			differentPotion[rv].effect = i + 1;
		}
		else i--;
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
	init_pair(BLACK_RED, COLOR_BLACK, COLOR_RED);

	initialize();
	
	MainMenu();
	if(EXIT)
	{ 	
		endwin();
		return 0;
	}

	if(GenerateMap)
	{
		generate_maze();
	}
	else
	{
		ReadMap();
	}

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
	if(rand() % (500 / Luck) == 0)hero.heroArmor->GetItem().mdf = 2;
	
	Weapon CopperShortsword(0);
	Weapon BronzeSpear(1);
	Weapon Musket(2);
	Weapon Stick(3);
	Weapon Shotgun(4);
	Weapon Pistol(5);
	differentWeapon[0] = CopperShortsword;
	differentWeapon[1] = BronzeSpear;
	differentWeapon[2] = Musket;
	differentWeapon[3] = Stick;
	differentWeapon[4] = Shotgun;
	differentWeapon[5] = Pistol;
	
	Tools Pickaxe(0);
	differentTools[0] = Pickaxe;

	Ammo SteelBullets(0);
	Ammo ShotgunShells(1);
	differentAmmo[0] = SteelBullets;
	differentAmmo[1] = ShotgunShells;
	
	Scroll MapScroll(0);
	Scroll IdentifyScrollBitch(1);
	differentScroll[0] = MapScroll;
	differentScroll[1] = IdentifyScrollBitch;

	Potion BluePotion(0);
	Potion GreenPotion(1);
	Potion DarkPotion(2);
	Potion lol(3);
	Potion kek(4);
	differentPotion[0] = BluePotion;
	differentPotion[1] = GreenPotion;
	differentPotion[2] = DarkPotion;
	differentPotion[3] = lol;
	differentPotion[4] = kek;
	
	SetRandomPotionEffects();

	Enemy Barbarian(0);
	Enemy Zombie(1);
	Enemy Guardian(2);
	differentEnemies[0] = Barbarian;
	differentEnemies[1] = Zombie;
	differentEnemies[2] = Guardian;

	hero.heroWeapon = &inventory[EMPTY_SLOT];

	SetItems();

	SpawnUnits();

	hero.FindVisibleArray();

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
	sprintf(tmp, "L/XP: %i/%i ", hero.level, hero.xp);
	bar += tmp;
	sprintf(tmp, "Lu: %i ", Luck);								// !DEBUG!
	bar += tmp;										//
	bar += "Bul: |";
	for(int i = 0; i < BANDOLIER; i++)
	{
		if(inventory[AMMO_SLOT + i].type != ItemEmpty)
		{
			sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invAmmo.count);
			bar += tmp;
		}
		else
		{
			bar += "0|";
		}
	}
	bar += " ";
	if(hero.isBurdened) bar += "Burdened. ";
	printw("%- 190s", bar.c_str());
	
	if(hero.heroWeapon->type != ItemEmpty)
	{
		weapon_bar = "";
		weapon_bar += hero.heroWeapon->GetItem().GetName();
		if(hero.heroWeapon->item.invWeapon.Ranged)
		{
			weapon_bar += "[";
			for(int i = 0; i < hero.heroWeapon->item.invWeapon.cartridgeSize; i++)
			{
				if(i < hero.heroWeapon->item.invWeapon.currentCS && (hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 450 ||
					hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 451))
				{
					weapon_bar += "i";
				}
				else
				{
					weapon_bar += "_";
				}
			}
			weapon_bar += "]";
		}		
		move(Height + 1, 0);
		printw("%- 190s", weapon_bar.c_str());
	}

	move(hero.posH, hero.posL);
	
	while(1)
	{
		if(EXIT)
		{ 	
			refresh();
			endwin();
			return 0;
		}

		message = "";
		bar = "";
	
		sprintf(tmp, "w: %i; Imap: %i; Umap: %i ! ! ", sizeof(Weapon), sizeof(ItemsMap), sizeof(UnitsMap));
		message += tmp;

		if(hero.hunger < 1)
		{
			move(Height + 2, 0);
			message += "You died from starvation. Press any key to exit. ";
			printw("%- 190s", message.c_str());
			getch();
			refresh();
			endwin();
			return 0;
		}

		if(hero.health < 1)
		{
			hero.health = 0;
			move(Height + 2, 0);
			message += "You died. Press any key to exit. ";
			printw("% -190s", message.c_str());
			getch();		
			refresh();
			endwin();
			return 0;
		}

		move(hero.posH, hero.posL);

		char inp = getch();
	
		hero.moveHero(inp);

		if(!Stop)
		{
			TurnsCounter++;

			if(TurnsCounter % 25 == 0 && TurnsCounter != 0 && MODE == 1)
			{
				if(hero.health < DEFAULT_HERO_HEALTH)
				{
					hero.health ++;
				}
			}

			hero.hunger--;
			
			if(INVISIBILITY > 0) INVISIBILITY--;

			if(BLINDNESS > 1) BLINDNESS --;
			else if(BLINDNESS == 1)
			{
				BLINDNESS--;
				VISION = DEFAULT_VISION;
			}
		
			if(hero.isBurdened) hero.hunger--;

			UpdateAI();
			
			++turns;

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
			sprintf(tmp, "L/XP: %i/%i ", hero.level, hero.xp);
			bar += tmp;
			sprintf(tmp, "Lu: %i ", Luck);									// !DEBUG!
			bar += tmp;											// !!
			bar += "Bul: |";
			for(int i = 0; i < BANDOLIER; i++)
			{
				if(inventory[AMMO_SLOT + i].type != ItemEmpty)
				{
					sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invAmmo.count);
					bar += tmp;
				}	
				else
				{
					bar += "0|";
				}
			}
			bar += " ";
			if(hero.isBurdened) bar += "Burdened. ";							//
	
			if(hero.hunger < 75)
			{	
				bar += "Hungry. ";
			}

			printw("%- 190s", bar.c_str());									//
		
			if(hero.heroWeapon->type != ItemEmpty)
			{
				weapon_bar = "";
				weapon_bar += hero.heroWeapon->GetItem().GetName();
				if(hero.heroWeapon->item.invWeapon.Ranged)
				{
					weapon_bar += "[";
					for(int i = 0; i < hero.heroWeapon->item.invWeapon.cartridgeSize; i++)
					{
						if(i < hero.heroWeapon->item.invWeapon.currentCS && (hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 450 ||
							hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 451))
						{
							weapon_bar += "i";
						}
						else
						{
							weapon_bar += "_";
						}
					}
					weapon_bar += "]";
				}
				move(Height + 1, 0);
				printw("%- 190s", weapon_bar.c_str());
			}

			move(Height + 2, 0);
			
			printw("%- 190s", message.c_str());
			
			if(inp == '\033')
			{	
				move(Height, 0);
				printw("Are you sure want to exit?\n");
				char inp = getch();
				if(inp == 'y' || inp == 'Y' || inp == CONTROL_CONFIRM)
				{
					refresh();
					endwin();
					return 0;
				}
				Stop = true;
			}	
	
			GetXP();

			move(hero.posH, hero.posL);
		}
		else
		{
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
			sprintf(tmp, "L/XP: %i/%i ", hero.level, hero.xp);
			bar += tmp;
			sprintf(tmp, "Lu: %i ", Luck);									// !DEBUG!
			bar += tmp;											// !!
			bar += "Bul: |";
			for(int i = 0; i < BANDOLIER; i++)
			{
				if(inventory[AMMO_SLOT + i].type != ItemEmpty)
				{
					sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invAmmo.count);
					bar += tmp;
				}	
				else
				{
					bar += "0|";
				}
			}
			bar += " ";
			if(hero.isBurdened) bar += "Burdened. ";							//
			printw("%- 190s", bar.c_str());									//
		
			if(hero.hunger < 75)
			{	
				bar += "Hungry. ";
			}

			move(Height + 2, 0);
			
			printw("%- 190s", message.c_str());

			Stop = false;
		}
	}
		
	refresh();
	endwin();

	return 0;
}
