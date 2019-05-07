//////////////////////////////////////////////////////////////////////////////////////// Symbols ////////////////////////////////////////////////////////////////
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
#include<fstream>
#include<stdlib.h>									//
#include<time.h>									//
#include<ncurses.h>									//
#include<string.h>									//
#include<vector>
#include<queue>
#include<assert.h>
#include<termlib/termlib.hpp>
#include<fmt/core.h>
#include<fmt/printf.h>

using namespace fmt::literals;

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
#include"include/log.hpp"
															
int map[FIELD_ROWS][FIELD_COLS];											
bool seenUpdated[FIELD_ROWS][FIELD_COLS];										// <- visible array
int active = 0;														
int turns = 0; /*-1*/

TerminalRenderer termRend;
TerminalReader termRead;
															
void initField() {															
	for (int i = 0; i < FIELD_ROWS; ++i) {														
		for (int j = 0; j < FIELD_COLS; ++j) {													
			map[i][j] = 1;										
		}
	}
}

void initialize() {
	srand(time(0));
	active = 1;
	initField();
	initLog();
}

string message = "";
string bar = "";
string weapon_bar = "";
char tmp[100];

void moveUnit(int, int&, int&, int, int);

int inventoryVol = 0;

int Luck;
int INVISIBILITY = 0;

int findStackable(int h, int l, int sym) {
	for (int i = 0; i < Depth; i++) {
		if (ItemsMap[h][l][i].type != ItemEmpty && ItemsMap[h][l][i].getItem().symbol == sym)
            return i;
	}
	return 101010;
}

Hero hero;

enum UnitType {
	UnitEmpty,
	UnitHero,
	UnitEnemy
};

union UnitedUnits {
	EmptyUnit uEmpty;
	Hero uHero;
	Enemy uEnemy;

	UnitedUnits(EmptyUnit e) {
		uEmpty = e;
	}

	UnitedUnits(Hero h) {
		uHero = h;
	}

	UnitedUnits(Enemy en) {
		uEnemy = en;
	}

	UnitedUnits(const UnitedUnits& u) = delete;
	UnitedUnits& operator=(const UnitedUnits& u) = delete;

	UnitedUnits() {
		uEmpty = EmptyUnit();
	}

	~UnitedUnits(){}
};

struct PossibleUnit {
	UnitedUnits unit;
	UnitType type;

	PossibleUnit(UnitedUnits u, UnitType t): type(t) {
		switch (type) {
			case UnitEmpty:
				unit.uEmpty = u.uEmpty;
				break;
			case UnitHero:
				unit.uHero = u.uHero;
				break;
			case UnitEnemy:
				unit.uEnemy = u.uEnemy;
		}
	}

	PossibleUnit() {
		type = UnitEmpty;
	}

	void operator=(const Hero& h) {
		type = UnitHero;
		unit.uHero = h;
	}

	void operator=(const EmptyUnit& e) {
		type = UnitEmpty;
		unit.uEmpty = e;
	}

	void operator=(const Enemy& en) {
		type = UnitEnemy;
		unit.uEnemy = en;
	}

	PossibleUnit(const PossibleUnit& p) {
		type = p.type;
		switch (type) {
			case UnitEmpty:
				unit.uEmpty = p.unit.uEmpty;
				break;
			case UnitHero:
				unit.uHero = p.unit.uHero;
				break;
			case UnitEnemy:
				unit.uEnemy = p.unit.uEnemy;
		}
	}

	PossibleUnit& operator=(const PossibleUnit& p) {
		type = p.type;
		switch (type) {
			case UnitEmpty:
				unit.uEmpty = p.unit.uEmpty;
				break;
			case UnitHero:
				unit.uHero = p.unit.uHero;
				break;
			case UnitEnemy:
				unit.uEnemy = p.unit.uEnemy;
		}		
        return *this;
	}

	Unit& getUnit() {
		switch (type) {
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

void Hero::showInventory(const char& inp) {	
	PossibleItem list[MaxInvVol];
	int len = 0;
	switch (inp) {	
		case CONTROL_SHOWINVENTORY: {
			for (int i = 0; i < MaxInvVol; i++) {
				if (inventory[i].type != ItemEmpty) {
					list[len] = inventory[i];
					len++;
				}
			}
			
			char hv[] = "Here is your inventory.";
			printList(list, len, hv, 1);
			char choise = termRead.readChar();
			if (choise == '\033')
                return;
			len = 0;
			break;
		}
		case CONTROL_EAT: {
			char hv[200] = "What do you want to eat?";
			for (int i = 0; i < MaxInvVol; i++) {
				if (inventory[i].type == ItemFood) {
					list[len] = inventory[i];
					len++;
				}
			}
			printList(list, len, hv, 1);
			len = 0;
			char choise = termRead.readChar();
			if (choise == '\033')
                return;
			int intch = choise - 'a';
			if (inventory[intch].type == ItemFood) {
				int prob = rand() % Luck;
				if (prob == 0) {
					hunger += inventory[intch].item.invFood.FoodHeal / 3;
					health --;
					message += "Fuck! This food was rotten! ";
				} else {
					hunger += inventory[intch].item.invFood.FoodHeal;
				}
				if (inventory[intch].getItem().count == 1) {
					inventory[intch].type = ItemEmpty;
				} else {
					inventory[intch].getItem().count--;
				}
			}
			break;
		}	
		case CONTROL_WEAR: {
			char hv[200] = "What do you want to wear?";
			for (int i = 0; i < MaxInvVol; i++) {
				if (inventory[i].type == ItemArmor) {
					list[len] = inventory[i];
					len++;
				}
			}
			printList(list, len, hv, 1);
			len = 0;
			char choise = termRead.readChar();
			if (choise == '\033')
                return;
			int intch = choise - 'a';
			if (inventory[intch].type == ItemArmor) {
				//sprintf(tmp, "Now you wearing %s. ", inventory[intch].getItem().getName());
				//message += tmp;
                message += "Now you wearing {}. "_format(inventory[intch].getItem().getName());

				if (heroArmor->type != ItemEmpty) {
					heroArmor->getItem().attribute = 100;
				}
				heroArmor = &inventory[intch];
				inventory[intch].getItem().attribute = 201;
			}
			break;
		}
		case CONTROL_DROP: {
			char hv[200] = "What do you want to drop?";
			for (int i = 0; i < MaxInvVol; i++) {
				if (inventory[i].type != ItemEmpty) {
					list[len] = inventory[i];
					len++;
				}
			}

			printList(list, len, hv, 1);
			len = 0;
			char choise = termRead.readChar();
			if (choise == '\033')
                return;
			int intch = choise - 'a';
			int num = findEmptyItemUnderThisCell(posH, posL);
			if (num == 101010) {
				message += "There is too much items";
				return;
			}
			if (choise == heroArmor->getItem().inventorySymbol)
                showInventory(CONTROL_TAKEOFF);
			if (choise == heroWeapon->getItem().inventorySymbol)
                showInventory(CONTROL_UNEQUIP);
			if (inventory[intch].getItem().isStackable && inventory[intch].getItem().count > 1) {
				clearRightPane();
				//move(0, Length + 10);
				//printw("How much items you want to drop?[1-9]");
                termRend
                    .setCursorPosition(Vec2i{ Length + 10 })
                    .put("How much items do you want to drop? [1-9]");

				int dropCount = termRead.readChar() - '0';
                dropCount = std::min(dropCount, inventory[intch].getItem().count);
                dropCount = std::max(dropCount, 1);

				int dep = findStackable(posH, posL, inventory[intch].getItem().symbol);
				if (dep == 101010) {		
					ItemsMap[posH][posL][num] = inventory[intch];
					ItemsMap[posH][posL][num].getItem().count = dropCount;
					inventory[intch].getItem().count -= dropCount;
					if (inventory[intch].getItem().count < 1) {
						inventory[intch].type = ItemEmpty;
					}
				} else {			
					ItemsMap[posH][posL][dep].getItem().count += dropCount;
					inventory[intch].getItem().count -= dropCount;
					if (inventory[intch].getItem().count < 1) {
						inventory[intch].type = ItemEmpty;
					}
				}
			} else if (inventory[intch].getItem().isStackable && inventory[intch].getItem().count == 1) {
				int dep = findStackable(posH, posL, inventory[intch].getItem().symbol);
				if (dep == 101010) {		
					ItemsMap[posH][posL][num] = inventory[intch];
				} else {			
					ItemsMap[posH][posL][dep].getItem().count++;
					inventory[intch].getItem().count--;
				}
				inventory[intch].type = ItemEmpty;
			} else {
				ItemsMap[posH][posL][num] = inventory[intch];
				inventory[intch].type = ItemEmpty;
			}

			if (getInventoryItemsWeight() <= MaxInvItemsWeight && isBurdened) {
				message += "You are burdened no more. ";
				isBurdened = false;
			}

			break;
		}
		case CONTROL_TAKEOFF: {
			
			heroArmor->getItem().attribute = 100;
			heroArmor = &inventory[EMPTY_SLOT];
			break;
		
		}
		case CONTROL_WIELD: {
			char hv[200] = "What do you want to wield?";

			for (int i = 0; i < MaxInvVol; i++) {
				if (inventory[i].type == ItemWeapon || inventory[i].type == ItemTools) {
					list[len] = inventory[i];
					len++;
				}
			}

			printList(list, len, hv, 1);
			len = 0;
			
			char choise = termRead.readChar();
			if (choise == '\033')
                return;
			int intch = choise - 'a';
			if (inventory[intch].type == ItemWeapon || inventory[intch].type == ItemTools) {
				//sprintf(tmp, "You wield %s.", inventory[intch].getItem().getName());
				//message += tmp;
                message += "You wield {}. "_format(inventory[intch].getItem().getName());

				if (heroWeapon->type != ItemEmpty) {
					heroWeapon->getItem().attribute = 100;
				}
				heroWeapon = &inventory[intch];
				inventory[intch].getItem().attribute = 301;
			}
	
			break;
		
		}
		case CONTROL_UNEQUIP: {
			heroWeapon->getItem().attribute = 100;
			heroWeapon = &inventory[EMPTY_SLOT];
			break;
		}
		case CONTROL_THROW: {
			char hv[200] = "What do you want to throw?";

			for (int i = 0; i < MaxInvVol; i++) {
				if (inventory[i].type != ItemEmpty) {
					list[len] = inventory[i];
					len++;
				}
			}

			printList(list, len, hv, 1);
			len = 0;

			char choise = termRead.readChar();
			if (choise == '\033') return;
			int intch = choise - 'a';

			if (inventory[intch].type != ItemEmpty) {
				clearRightPane();
				//move(0, Length + 10);
				//printw("In what direction?");
                termRend
                    .setCursorPosition(Vec2i{ Length + 10, 0 })
                    .put("In what direction?");
				char secondChoise = termRead.readChar();

				if (inventory[intch].getItem().inventorySymbol == heroArmor->getItem().inventorySymbol)
                    showInventory(CONTROL_TAKEOFF);
                else if (inventory[intch].getItem().inventorySymbol == heroWeapon->getItem().inventorySymbol)
                    showInventory(CONTROL_UNEQUIP);

				throwAnimated(inventory[intch], getDirectionByControl(secondChoise));
			}
			break;
		}
		case CONTROL_DRINK: {
			char hv[200] = "What do you want to drink?";

			for (int i = 0; i < MaxInvVol; i++) {
				if (inventory[i].type == ItemPotion) {
					list[len] = inventory[i];
					len++;
				}
			}

			printList(list, len, hv, 1);
			len = 0;

			char choise = termRead.readChar();
			if (choise == '\033')
                return;
			int intch = choise - 'a';

			if (inventory[intch].type == ItemPotion) {
				switch (inventory[intch].item.invPotion.effect) {
					case 1: {
                        health = std::min(health + 3, DEFAULT_HERO_HEALTH);
						message += "Now you feeling better. ";
						break;
					}
					case 2: {
						INVISIBILITY = 150;
						message += "Am I invisible? Oh, lol! ";
						break;
					}
					case 3: {
						for (int i = 0; i < 1; i++) {
							int l = rand() % Length;
							int h = rand() % Height;
							if (map[h][l] != 2 && UnitsMap[h][l].type == UnitEmpty) {
								UnitsMap[h][l] = UnitsMap[posH][posL];
								UnitsMap[posH][posL].type = UnitEmpty;
								posH = h;
								posL = l;
								findVisibleArray();
							} else {
                                i--;
                            }
						}
						message += "Teleportation is so straaange thing! ";
						break;
					}
					case 4: {
						message += "Well.. You didn't die. Nice. ";
						break;
					}
					case 5: {
						VISION = 1;
						BLINDNESS = 50;
						message += "My eyes!! ";
						break;
					}
				}
				discoveredPotions[inventory[intch].item.invPotion.symbol - 600] = true;
				if (inventory[intch].getItem().count == 1) {
					inventory[intch].type = ItemEmpty;
				} else {
					--inventory[intch].getItem().count;
				}
			}
			break;
		}
		case CONTROL_READ: {
			char hv[200] = "What do you want to read?";

			for (int i = 0; i < MaxInvVol; i++) {
				if (inventory[i].type == ItemScroll) {
					list[len] = inventory[i];
					len++;
				}
			}

			printList(list, len, hv, 1);
			len = 0;

			char choise = termRead.readChar();
			if (choise == '\033')
                return;
			int intch = choise - 'a';

			if (inventory[intch].type == ItemScroll) {
				switch (inventory[intch].item.invPotion.effect) {
					case 1: {
						message += "You wrote this map. Why you read it, I don't know. ";
						break;
					}
					case 2: {
						clearRightPane();
						//move(0, Length + 10);
						//printw("What do you want to identify?");
                        termRend
                            .setCursorPosition(Vec2i{ Length + 10 })
                            .put("What do you want to identify?");

						char in = termRead.readChar();
						int intin = in - 'a';
						if (inventory[intin].type != ItemEmpty) {
							if (inventory[intin].type != ItemPotion) {
								inventory[intin].getItem().showMdf = true;
							} else if (inventory[intin].type == ItemPotion) {
								discoveredPotions[inventory[intin].getItem().symbol - 600] = true;
							}	
						
							if (inventory[intch].getItem().count == 1) {
								inventory[intch].type = ItemEmpty;
							} else {
								--inventory[intch].getItem().count;
							}
						}
						break;
					}
				}
			}
			break;
		}
		case CONTROL_OPENBANDOLIER: {
			clearRightPane();
			//move(0, Length + 10);
			//printw("Here is your ammo.");
            termRend
                .setCursorPosition(Vec2i{ Length + 10 })
                .put("Here is your ammo.");
			int choise = 0;
			int num = 0;
			PossibleItem buffer;
			int pos;
			while (true) {
				num = 0;
				for (int i = 0; i < BANDOLIER; i++) {
					//move(1, Length + 12 + num);
					num += 2;

                    TextStyle style = TextStyle{ TerminalColor{} };
                    char symbol = '-';

					if (inventory[AMMO_SLOT + i].type == ItemAmmo) {
						switch (inventory[AMMO_SLOT + i].getItem().symbol) {
							case 450:
                                style = TextStyle{ TextStyle::Bold, TerminalColor{ Color::Black } };
                                symbol = ',';
								//if (choise == i) //addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT | UL);
								//else addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								break;
							case 451:
                                style = TextStyle{ TextStyle::Bold, TerminalColor{ Color::Red } };
                                symbol = ',';
								//if (choise == i)// addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT | UL);
								//else addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
								break;
							default:
								//if (choise == i) addch('-' | COLOR_PAIR(WHITE_BLACK) | UL);
								//else addch('-' | COLOR_PAIR(WHITE_BLACK));
								break;
						}
					//} else {
						//if (choise == i) addch('-' | COLOR_PAIR(WHITE_BLACK) | UL);
						//else addch('-' | COLOR_PAIR(WHITE_BLACK));
					}
                    if (choise == i)
                        style += TextStyle::Underlined;

                    termRend
                        .setCursorPosition(Vec2i{ Length + num + 12, 1 })
                        .put(symbol, style);
				}
				//switch (getch()) {
                char input = termRead.readChar();
                switch (input) {
					case CONTROL_LEFT: {
						if (choise > 0)
                            choise--;
						break;
					}
					case CONTROL_RIGHT: {
						if (choise < BANDOLIER - 1)
                            choise++;
						break;
					}
					case CONTROL_EXCHANGE: {
						if (buffer.type != ItemEmpty) {
							inventory[pos] = inventory[AMMO_SLOT + choise];
							inventory[AMMO_SLOT + choise] = buffer;
							buffer.type = ItemEmpty;
						} else {
							buffer = inventory[AMMO_SLOT + choise];
							inventory[AMMO_SLOT + choise].type = ItemEmpty;
							pos = AMMO_SLOT + choise;
						}
						break;
					}
					case '\033': {
						if (buffer.type != ItemEmpty) {
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
		case CONTROL_RELOAD: {
			clearRightPane();
			//move(0, Length + 10);
			//printw("Now you can load your weapon");
            termRend
                .setCursorPosition(Vec2i{ Length + 10 })
                .put("Now you can load your weapon");
			while (true) {
				for (int i = 0; i < heroWeapon->item.invWeapon.cartridgeSize; i++) {
                    TextStyle style{ TerminalColor{} };
                    char symbol = 'i';
					if (heroWeapon->item.invWeapon.cartridge[i].count == 1) {
						switch (heroWeapon->item.invWeapon.cartridge[i].symbol) {
							case 450:
								//addch('i' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                                style = TextStyle{ TextStyle::Bold, Color::Black };
								break;
							case 451:
                                style = TextStyle{ TextStyle::Bold, Color::Red };
								//addch('i' | COLOR_PAIR(RED_BLACK) | LIGHT);
								break;
							default:
								//addch('?');
                                symbol = '?';
						}
					} else {
						//addch('_');
                        symbol = '_';
					}
					//move(1, 10 + Length + i);
                    termRend
                        .setCursorPosition(Vec2i{ Length + i + 10, 1 })
                        .put(symbol, style);
				}
				
				string loadString = "";
				
				for (int i = 0; i < BANDOLIER; i++) {
					int ac = inventory[AMMO_SLOT + i].item.invArmor.count;
					//sprintf(tmp, "[%i|", i + 1);
					//loadString += tmp;
                    loadString += "[{}|"_format(i + 1);
					if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
						switch (inventory[AMMO_SLOT + i].getItem().symbol) {
							case 450:
								loadString += " steel bullets ";
								break;
							case 451:
								loadString += " shotgun shells ";
								break;
							default:
								loadString += " omgwth? ";
						}
						loadString += "]";
					}
					else loadString += " nothing ]";
				}
				
				loadString += "   [u] - unload ";
				
                //move(2, 10 + Length);
				//printw("%s", loadString.c_str());
                termRend
                    .setCursorPosition(Vec2i{ Length + 10, 2 })
                    .put(loadString);
				
				char in = termRead.readChar();
				if (in == '\033')
                    return;

				if (in == 'u') {
					if (heroWeapon->item.invWeapon.currentCS == 0) {
						continue;
					} else {
						bool found = false;
						for (int j = 0; j < BANDOLIER; j++) {
							if (inventory[AMMO_SLOT + j].type == ItemAmmo && 
									inventory[AMMO_SLOT + j].getItem().symbol == 
									heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].symbol) {
								heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
								inventory[AMMO_SLOT + j].item.invAmmo.count++;
								heroWeapon->item.invWeapon.currentCS--;
								found = true;
								break;
							}
						}
						if (!found) {
							for (int j = 0; j < BANDOLIER; j++) {
								if (inventory[AMMO_SLOT + j].type == ItemEmpty) {
									inventory[AMMO_SLOT + j] = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1];
									inventory[AMMO_SLOT + j].type = ItemAmmo;
									heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
									heroWeapon->item.invWeapon.currentCS--;
									found = true;
									break;
								}
							}
						}
						if (!found) {
							bool can_stack = false;
							for (int j = 0; j < Depth; j++) {
								if (ItemsMap[posH][posL][j].type == 
										ItemAmmo && 
										ItemsMap[posH][posL][j].getItem().symbol == 
										heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].symbol) {
									ItemsMap[posH][posL][j].item.invAmmo.count++;
									heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
									heroWeapon->item.invWeapon.currentCS--;
									can_stack = true;
									found = true;
								}
							}
							if (!can_stack) {
								int empty = findEmptyItemUnderThisCell(posH, posL);
								if (empty != 101010) {
                                    ItemsMap[posH][posL][empty].item.invAmmo = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1];
                                    ItemsMap[posH][posL][empty].type = ItemAmmo;
                                    heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count--;
                                    heroWeapon->item.invWeapon.currentCS--;
                                    found = true;
								}
							}
						}
						if (!found) {
							message += "You can`t unload your weapon. Idk, why. ";
						}
					}
				} else {
					int intin = in - '1';
					if (inventory[AMMO_SLOT + intin].type != ItemEmpty) {
						if (heroWeapon->item.invWeapon.currentCS >= heroWeapon->item.invWeapon.cartridgeSize) {
							message += "Weapon is loaded ";
							return;
						}
						heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS] = inventory[AMMO_SLOT + intin].item.invAmmo;
						heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS].count = 1;
						heroWeapon->item.invWeapon.currentCS++;
						if (inventory[AMMO_SLOT + intin].item.invAmmo.count > 1) {
                            inventory[AMMO_SLOT + intin].item.invAmmo.count --;
                        } else {
                            inventory[AMMO_SLOT + intin].type = ItemEmpty;
                        }
					}
				}
			}
			break;	
		}
	}
}

void dropInventory(PossibleUnit& unit) {
	int h = unit.getUnit().posH;
	int l =	unit.getUnit().posL;
	for (int i = 0; i < UNITINVENTORY; i++) {										/* Here are some changes, that we need to test */
		if (unit.getUnit().unitInventory[i].type != ItemEmpty) {
			if (unit.getUnit().unitInventory[i].getItem().isStackable) {
				int dep = findStackable(unit.getUnit().posH, unit.getUnit().posL, unit.getUnit().unitInventory[i].getItem().symbol);
				if (dep == 101010) {
					int empty = hero.findEmptyItemUnderThisCell(h, l);
					if (empty != 101010) {
						ItemsMap[h][l][empty] = unit.getUnit().unitInventory[i];
					}
				} else {
					ItemsMap[unit.getUnit().posH][unit.getUnit().posL][dep].getItem().count += unit.getUnit().unitInventory[i].getItem().count;
				}
			} else {
				int empty = hero.findEmptyItemUnderThisCell(h, l);
				if (empty != 101010) {
					ItemsMap[h][l][empty] = unit.getUnit().unitInventory[i];
				}
			}
		}
	}
}

void Hero::attackEnemy(int& a1, int& a2) {
	if (heroWeapon->type == ItemWeapon) {
		UnitsMap[posH + a1][posL + a2].getUnit().health -= heroWeapon->item.invWeapon.damage;
	} else if (heroWeapon->type == ItemTools) {
		UnitsMap[posH + a1][posL + a2].getUnit().health -= heroWeapon->item.invTools.damage;
	}
	if (UnitsMap[posH + a1][posL + a2].getUnit().health <= 0) {
		dropInventory(UnitsMap[posH + a1][posL + a2]);
		UnitsMap[posH + a1][posL + a2].type = UnitEmpty;
		xp += UnitsMap[posH + a1][posL + a2].unit.uEnemy.xpIncreasing;
	}
}

void getProjectileDirectionsAndSymbol(Direction direction, int & dx, int & dy, char & sym) {
	switch (direction) {
    case Direction::Up:
		dy = -1;
		dx = 0;
		sym = '|';
		break;
    case Direction::UpRight:
		dy = -1;
		dx = 1;
		sym = '/';
		break;
    case Direction::Right:
		dx = 1;
		dy = 0;
		sym = '-';
		break;
    case Direction::DownRight:
		dx = 1;
		dy = 1;
		sym = '\\';
		break;
    case Direction::Down:
		dy = 1;
		dx = 0;
		sym = '|';
		break;
    case Direction::DownLeft:
		dy = 1;
		dx = -1;
		sym = '/';
		break;
    case Direction::Left:
		dx = -1;
		dy = 0;
		sym = '-';
		break;
    case Direction::UpLeft:
		dx = -1;
		dy = -1;
		sym = '\\';
	}
}

void Hero::throwAnimated(PossibleItem& item, Direction direction) {
	int ThrowLength = 0;
	int dx = 0;
	int dy = 0;
	char sym;
	getProjectileDirectionsAndSymbol(direction, dx, dy, sym);
	for (int i = 0; i < 12 - item.getItem().weight / 3; i++) {						// 12 is "strength"
		int row = posH + dy * (i + 1);
		int col = posL + dx * (i + 1);

		if (map[row][col] == 2)
            break;

		if (UnitsMap[row][col].type != UnitEmpty) {
			UnitsMap[row][col].getUnit().health -= item.getItem().weight / 2;
			if (UnitsMap[row][col].getUnit().health <= 0) {
				dropInventory(UnitsMap[row][col]);
				UnitsMap[row][col].type = UnitEmpty;
				xp += UnitsMap[row][col].unit.uEnemy.xpIncreasing;
			}
			break;
		}
		//move(row, col);
		//addch(sym);
		//refresh();
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
		ThrowLength++;
		delay(DELAY);
	}
	int empty = findEmptyItemUnderThisCell(posH + dy * ThrowLength, posL + dx * ThrowLength);
	if (empty == 101010) {
		int empty2 = findEmptyItemUnderThisCell(posH + dy * (ThrowLength - 1), posL + dx * (ThrowLength - 1));
		ItemsMap[posH + dy * (ThrowLength - 1)][posL + dx * (ThrowLength - 1)][empty2] = item;
		item.type = ItemEmpty;
	} else {
		ItemsMap[posH + dy * ThrowLength][posL + dx * ThrowLength][empty] = item;
		item.type = ItemEmpty;
	}
}

void Hero::shoot() {
	if (heroWeapon->item.invWeapon.Ranged == false) {
		message += "You have no ranged weapon in hands. ";
		return;
	}
	if (heroWeapon->item.invWeapon.currentCS == 0) {
		message += "You have no bullets. ";
		Stop = true;
		return;
	}
	//move(0, Length + 10);
	//printw("In what direction? ");
    termRend
        .setCursorPosition(Vec2i{ Length + 10, 0 })
        .put("In what direction? ");

	char choise = termRead.readChar();
	if (choise != CONTROL_UP 
			&& choise != CONTROL_DOWN 
			&& choise != CONTROL_LEFT 
			&& choise != CONTROL_RIGHT 
			&& choise != CONTROL_UPLEFT 
			&& choise != CONTROL_UPRIGHT 
			&& choise != CONTROL_DOWNLEFT
			&& choise != CONTROL_DOWNRIGHT) {
		Stop = true;
		return;
	}
	int dx = 0;
	int dy = 0;
	char sym;
	getProjectileDirectionsAndSymbol(getDirectionByControl(choise), dx, dy, sym);
	int bullet_power = heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].damage + hero.heroWeapon->item.invWeapon.damageBonus;

	for (int i = 1; i < heroWeapon->item.invWeapon.range + heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].range; i++) {
		int row = posH + dy * i;
		int col = posL + dx * i; 

		if (map[row][col] == 2)
            break;

		if (UnitsMap[row][col].type != UnitEmpty) {
			UnitsMap[row][col].getUnit().health -= bullet_power - i / 3;
			if (UnitsMap[row][col].getUnit().health <= 0) {
				dropInventory(UnitsMap[row][col]);
				UnitsMap[row][col].type = UnitEmpty;
				xp += UnitsMap[row][col].unit.uEnemy.xpIncreasing;
			}
		}
		//move(row, col);
		//addch(sym);
		//refresh();
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
		delay(DELAY / 3);
	}
	heroWeapon->item.invWeapon.cartridge[heroWeapon->item.invWeapon.currentCS - 1].count = 0;
	heroWeapon->item.invWeapon.currentCS--;
}

void Hero::mHLogic(int& a1, int& a2) {
	if (map[posH + a1][posL + a2] != 2 || 
			(map[posH + a1][posL + a2] == 2 && CanHeroMoveThroughWalls) && 
			(posH + a1 > 0 && posH + a1 < Height - 1 && posL + a2 > 0 && posL + a2 < Length - 1)) {
		if (UnitsMap[posH + a1][posL + a2].type == UnitEmpty) {
			UnitsMap[posH + a1][posL + a2] = UnitsMap[posH][posL];
			UnitsMap[posH][posL].type = UnitEmpty;
			posH += a1;
			posL += a2;
		} else if (UnitsMap[posH + a1][posL + a2].type == UnitEnemy) {
			attackEnemy(a1, a2);
		}
	} else if (map[posH + a1][posL + a2] == 2) {
		if (heroWeapon->type == ItemTools) {
			if (heroWeapon->item.invTools.possibility == 1) {
				//move(0, Length + 10);
				//printw("Do you want to dig this wall (y or n)? ");
                termRend
                    .setCursorPosition(Vec2i{ Length + 10, 0 })
                    .put("Do you want to dig this wall? [yn]");

				char inpChar = termRead.readChar();
				if (inpChar == 'y' || inpChar == 'Y') {
					map[posH + a1][posL + a2] = 1;
					heroWeapon->item.invTools.uses--;
					if (heroWeapon->item.invTools.uses <= 0) {
						//sprintf(tmp, "Your %s is broken. ", heroWeapon->getItem().getName());
						//message += tmp;
                        message += "Your {} is broken. "_format(heroWeapon->getItem().getName());
						heroWeapon->type = ItemEmpty;
						findVisibleArray();
					}
					return;
				}
			}
		}
		message += "The wall is the way. ";
		Stop = true;
	}
	findVisibleArray();
}

void Enemy::shoot() {
	if (posH == hero.posH && posL < hero.posL)
        dir = Direction::Right;
	else if (posH == hero.posH && posL > hero.posL)
        dir = Direction::Left;
	else if (posL == hero.posL && posH > hero.posH)
        dir = Direction::Up;
	else if (posL == hero.posL && posH < hero.posH)
        dir = Direction::Down;
	else if (posL > hero.posL && posH > hero.posH)
        dir = Direction::UpLeft;
	else if (posL > hero.posL && posH < hero.posH)
        dir = Direction::DownLeft;
	else if (posL < hero.posL && posH < hero.posH)
        dir = Direction::DownRight;
	else if (posL < hero.posL && posH > hero.posH)
        dir = Direction::UpRight;
	int dx;
	int dy;
	char sym;
	getProjectileDirectionsAndSymbol(dir, dx, dy, sym);
	for (int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++) {
		int row = posH + dy * i;
		int col = posL + dx * i;

		if (map[row][col] == 2)
            break;

		if (UnitsMap[row][col].type == UnitHero) {
			hero.health -= (unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus) * (( 100 - hero.heroArmor->item.invArmor.defence) / 100.0);
			break;
		}
		//move(row, col);
		//addch(sym);
		//refresh();
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
		delay(DELAY / 3);
	}

	unitAmmo->item.invAmmo.count--;
	if (unitAmmo->item.invAmmo.count <= 0) {
		unitAmmo->type = ItemEmpty;
	}
}

int bfs(int targetH, int targetL, int h, int l, int &posH, int &posL) {
	int depth = 2 + ABS(targetH - h) + ABS(targetL - l);						// <- smth a little bit strange
	queue<int> x, y;
	x.push(l);
	y.push(h);
	int used[Height][Length] = {};
	used[h][l] = true;
	while (!x.empty() && !y.empty()) {
		int v_x = x.front();
		int v_y = y.front();
		if (v_y == targetH && v_x == targetL)
            break;
		if (used[v_y][v_x] > depth) {
			return -1;
		}
		x.pop();
		y.pop();
	
		if (v_y < Height - 1 && !used[v_y + 1][v_x] && (UnitsMap[v_y + 1][v_x].type == UnitEmpty 
			|| UnitsMap[v_y + 1][v_x].type == UnitHero) && map[v_y + 1][v_x] != 2) {
			y.push(v_y + 1);
			x.push(v_x);
			used[v_y + 1][v_x] = 1 + used[v_y][v_x];
		}
		if (v_y > 0 && !used[v_y - 1][v_x] && (UnitsMap[v_y - 1][v_x].type == UnitEmpty 
			|| UnitsMap[v_y - 1][v_x].type == UnitHero) && map[v_y - 1][v_x] != 2) {
			y.push(v_y - 1);
			x.push(v_x);	
			used[v_y - 1][v_x] = 1 + used[v_y][v_x];
		}
		if (v_x < Length - 1 && !used[v_y][v_x + 1] && (UnitsMap[v_y][v_x + 1].type == UnitEmpty 
			|| UnitsMap[v_y][v_x + 1].type == UnitHero) && map[v_y][v_x + 1] != 2) {
			y.push(v_y);
			x.push(v_x + 1);
			used[v_y][v_x + 1] = 1 + used[v_y][v_x];
		}
		if (v_x > 0 && !used[v_y][v_x - 1] && (UnitsMap[v_y][v_x - 1].type == UnitEmpty 
			|| UnitsMap[v_y][v_x - 1].type == UnitHero) && map[v_y][v_x - 1] != 2) {
			y.push(v_y);
			x.push(v_x - 1);
			used[v_y][v_x - 1] = 1 + used[v_y][v_x];	
		}
		if (MODE == 2) {
			if (v_y < Height - 1)
			{
				if (v_x > 0 && !used[v_y + 1][v_x - 1] && (UnitsMap[v_y + 1][v_x - 1].type == UnitEmpty 
					|| UnitsMap[v_y + 1][v_x - 1].type == UnitHero) && map[v_y + 1][v_x - 1] != 2) {
					y.push(v_y + 1);
					x.push(v_x - 1);
					used[v_y + 1][v_x - 1] = 1 + used[v_y][v_x];
				}
				if (v_x < Length - 1 && !used[v_y + 1][v_x + 1] && (UnitsMap[v_y + 1][v_x + 1].type == UnitEmpty 
					|| UnitsMap[v_y + 1][v_x + 1].type == UnitHero) && map[v_y + 1][v_x + 1] != 2) { 
					y.push(v_y + 1);
					x.push(v_x + 1);
					used[v_y + 1][v_x + 1] = 1 + used[v_y][v_x];
				}
			}
			if (v_y > 0) {
				if (v_x > 0 && !used[v_y - 1][v_x - 1] && (UnitsMap[v_y - 1][v_x - 1].type == UnitEmpty 
					|| UnitsMap[v_y - 1][v_x - 1].type == UnitHero) && map[v_y - 1][v_x - 1] != 2) {
					y.push(v_y - 1);
					x.push(v_x - 1);
					used[v_y - 1][v_x - 1] = 1 + used[v_y][v_x];
				}
				if (v_x < Length - 1 && !used[v_y - 1][v_x + 1] && (UnitsMap[v_y - 1][v_x + 1].type == UnitEmpty 
					|| UnitsMap[v_y - 1][v_x + 1].type == UnitHero) && map[v_y - 1][v_x + 1] != 2) {
					y.push(v_y - 1);
					x.push(v_x + 1);
					used[v_y - 1][v_x + 1] = 1 + used[v_y][v_x];
				}
			}
		}
	}

	if (!used[targetH][targetL]) {
		return -1;
	}
	int v_y = targetH, v_x = targetL;
	while (used[v_y][v_x] != 2 ) {
		if (MODE == 2) {
			if (v_y && v_x && used[ v_y - 1 ][ v_x - 1 ] + 1 == used[ v_y ][ v_x ]) {
				--v_y;
				--v_x;
				continue;
			}
			if (v_y && v_x < Length - 1 && used[ v_y - 1 ][ v_x + 1 ] + 1 == used[ v_y ][ v_x ]) {
				--v_y;
				++v_x;
				continue;
			}
			if (v_y < Height - 1 && v_x && used[ v_y + 1 ][ v_x - 1 ] + 1 == used[ v_y ][ v_x ]) {
				++v_y;
				--v_x;
				continue;
			}
			if (v_y < Height - 1 && v_x < Length - 1 && used[ v_y + 1 ][ v_x + 1 ] + 1 == used[ v_y ][ v_x ]) {
				++v_y;
				++v_x;
				continue;
			}
		}
		if (v_y && used[ v_y - 1 ][ v_x ] + 1 == used[ v_y ][ v_x ]) {
			--v_y;
			continue;
		}
		if (v_x && used[ v_y ][ v_x - 1 ] + 1 == used[ v_y ][ v_x ]) {
			--v_x;
			continue;
		}
		if (v_y < Height - 1 && used[ v_y + 1 ][ v_x ] + 1 == used[ v_y ][ v_x ]) {
			++v_y;
			continue;
		}
		if (v_x < Length - 1 && used[ v_y ][ v_x + 1 ] + 1 == used[ v_y ][ v_x ]) {
			++v_x;
			continue;
		}
	}

	posH = v_y;
	posL = v_x;
}

void updatePosition(PossibleUnit& unit) {
	bool HeroVisible = false;

	if (INVISIBILITY > 0) {
		HeroVisible = false;
	} else if ((SQR(unit.getUnit().posH - hero.posH) + SQR(unit.getUnit().posL - hero.posL) < SQR(unit.getUnit().vision)) 
		&& unit.getUnit().canSeeCell(hero.posH, hero.posL)) {
		HeroVisible = true;
	}
	
	int pH = 1, pL = 1;

	if (HeroVisible) {
		if ((unit.getUnit().posH == hero.posH || 
				unit.getUnit().posL == hero.posL || 
				ABS(hero.posH - unit.getUnit().posH) == ABS(hero.posL - unit.getUnit().posL)) && 
				unit.getUnit().unitWeapon->item.invWeapon.Ranged == true &&
				unit.getUnit().unitWeapon->item.invWeapon.range + unit.getUnit().unitAmmo->item.invAmmo.range >= 
				ABS(hero.posH - unit.getUnit().posH) + ABS(hero.posL - unit.getUnit().posL)) {
			unit.unit.uEnemy.shoot();
		} else {
			unit.unit.uEnemy.targetH = hero.posH;
			unit.unit.uEnemy.targetL = hero.posL;

			if (bfs(hero.posH, hero.posL, unit.getUnit().posH, unit.getUnit().posL, pH, pL) == -1) {
				HeroVisible = false;
			} else {
				if (UnitsMap[pH][pL].type == UnitEnemy) {
					return;
				} else if (UnitsMap[pH][pL].type == UnitHero) {
					if (unit.getUnit().unitWeapon->type == ItemWeapon) {
						if (hero.heroArmor->item.invArmor.mdf != 2) {
							hero.health -= unit.getUnit().unitWeapon->item.invWeapon.damage * ((100 - hero.heroArmor->item.invArmor.defence) / 100.0);
						} else {
							unit.getUnit().health -= unit.getUnit().unitWeapon->item.invWeapon.damage;
						}
					} else if (unit.getUnit().unitWeapon->type == ItemTools) {
						if (hero.heroArmor->item.invArmor.mdf != 2) {
							hero.health -= unit.getUnit().unitWeapon->item.invTools.damage * ((100 - hero.heroArmor->item.invArmor.defence) / 100.0);
						} else {
							unit.getUnit().health -= unit.getUnit().unitWeapon->item.invTools.damage;
						}
					}
					if (unit.getUnit().health <= 0) {
						unit.type = UnitEmpty;
					}
				} else {
					unit.getUnit().posH = pH;
					unit.getUnit().posL = pL;
					UnitsMap[pH][pL] = unit;
					unit.type = UnitEmpty;
				}
			}
		}
	}
	if (!HeroVisible) {
		bool needRandDir = 0;
		if (unit.unit.uEnemy.targetH != -1 && (unit.unit.uEnemy.targetH != unit.getUnit().posH || unit.unit.uEnemy.targetL != unit.getUnit().posL)) {

			if (bfs(unit.unit.uEnemy.targetH, unit.unit.uEnemy.targetL, unit.unit.uEnemy.posH, unit.unit.uEnemy.posL, pH, pL) == -1) {
				needRandDir = 1;
			} else {
				if (pH < Height && pH > 0 && pL < Length && pL > 0) {
					if (UnitsMap[pH][pL].type == UnitHero) {
						if (unit.getUnit().unitWeapon->type == ItemWeapon) {
							if (hero.heroArmor->item.invArmor.mdf != 2) {
								hero.health -= unit.getUnit().unitWeapon->item.invWeapon.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
							} else {
								unit.getUnit().health -= unit.getUnit().unitWeapon->item.invWeapon.damage;
							}
						} else if (unit.getUnit().unitWeapon->type == ItemTools) {
							if (hero.heroArmor->item.invArmor.mdf != 2) {
								hero.health -= unit.getUnit().unitWeapon->item.invTools.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
							} else {
								unit.getUnit().health -= unit.getUnit().unitWeapon->item.invTools.damage;
							}
						}
						if (unit.getUnit().health <= 0) {
							unit.type = UnitEmpty;
						}
					} else {
						unit.getUnit().posH = pH;
						unit.getUnit().posL = pL;
						UnitsMap[pH][pL] = unit;
						unit.type = UnitEmpty;
					}
				}
			}
		} else {
			needRandDir = 1;
		}
		if (needRandDir) {
			vector<int> visionArrayH;
			vector<int> visionArrayL;

			int psH = unit.getUnit().posH, psL = unit.getUnit().posL, vis = unit.getUnit().vision;

			for (int i = MAX(psH - vis, 0); i < MIN(Height, psH + vis); i++) {
				for (int j = MAX(psL - vis, 0); j < MIN(psL + vis, Length); j++) {
					if ((i != psH || j != psL)
					&& SQR(psH - i) + SQR(psL - j) < SQR(vis) && map[i][j] != 2 
					&& UnitsMap[i][j].type == UnitEmpty && unit.getUnit().canSeeCell(i, j)) {
						visionArrayH.push_back(i);
						visionArrayL.push_back(j);
					}
				}	
			}
			while (true) {
				int r; 
				int rposH = visionArrayH[r = (rand() % visionArrayH.size())];
				int rposL = visionArrayL[r];
				
				unit.unit.uEnemy.targetH = rposH;
				unit.unit.uEnemy.targetL = rposL;

				if (bfs(unit.unit.uEnemy.targetH, unit.unit.uEnemy.targetL, unit.unit.uEnemy.posH, unit.unit.uEnemy.posL, pH, pL) == -1) {
					continue;
				}
				if (pH < Height && pH > 0 && pL < Length && pL > 0) {
					if (UnitsMap[pH][pL].type == UnitHero) {
						if (unit.getUnit().unitWeapon->type == ItemWeapon) {
							if (hero.heroArmor->item.invArmor.mdf != 2) {
								hero.health -= unit.getUnit().unitWeapon->item.invWeapon.damage * ((100 - hero.heroArmor->item.invArmor.defence) / 100.0);
							} else {
								unit.getUnit().health -= unit.getUnit().unitWeapon->item.invWeapon.damage;
							}
						} else if (unit.getUnit().unitWeapon->type == ItemTools) {
							if (hero.heroArmor->item.invArmor.mdf != 2) {
								hero.health -= unit.getUnit().unitWeapon->item.invTools.damage * ((100 - hero.heroArmor->item.invArmor.defence) / 100.0);
							} else {
								unit.getUnit().health -= unit.getUnit().unitWeapon->item.invTools.damage;
							}
						}
						if (unit.getUnit().health <= 0) {
							unit.type = UnitEmpty;
						}
					} else {
						unit.getUnit().posH = pH;
						unit.getUnit().posL = pL;
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

void updateAI() {
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Length; j++) {
			if (UnitsMap[i][j].type == UnitEnemy && UnitsMap[i][j].unit.uEnemy.movedOnTurn != turns) {
#				ifdef DEBUG
				//sprintf(tmp, "{%i|%i|%i|%i}", i, j, UnitsMap[i][j].unit.uEnemy.symbol, UnitsMap[i][j].getUnit().health);
				//message += tmp;
                message += "{{{}|{}|{}|{}}}"_format(i, j, UnitsMap[i][j].unit.uEnemy.symbol, UnitsMap[i][j].getUnit().health);
#				endif
				if (MODE == 2 && turns % 200 == 0) {
					UnitsMap[i][j].getUnit().health++;
				}
				UnitsMap[i][j].unit.uEnemy.movedOnTurn = turns;
				updatePosition(UnitsMap[i][j]);
			}
		}
	}
}

void setItems() {
	for (int i = 0; i < FOODCOUNT; i++) {
		int h = rand() % Height;
		int l = rand() % Length;

		if (map[h][l] == 1) {
			int p = rand() % TypesOfFood;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentFood[p];
			ItemsMap[h][l][d].getItem().posH = h;
			ItemsMap[h][l][d].getItem().posL = l;
		} else {
            i--;
        }
	}

	for (int i = 0; i < ARMORCOUNT; i++) {
		int h = rand() % Height;
		int l = rand() % Length;

		if (map[h][l] == 1) {
			int p = rand() % TypesOfArmor;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentArmor[p];
			ItemsMap[h][l][d].getItem().posH = h;
			ItemsMap[h][l][d].getItem().posL = l;
			if (rand() % 500 / Luck == 0) {
                ItemsMap[h][l][d].item.invArmor.mdf = 2;
            }
		} else {
            i--;
        }
	}
	
	for (int i = 0; i < WEAPONCOUNT; i++) {
		int h = rand() % Height;
		int l = rand() % Length;
		
		if (map[h][l] == 1) {
			int p = rand() % TypesOfWeapon;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentWeapon[p];
			ItemsMap[h][l][d].getItem().posH = h;
			ItemsMap[h][l][d].getItem().posL = l;
		} else {
            i--;
        }
	}

	for (int i = 0; i < AMMOCOUNT; i++) {
		int h = rand() % Height;
		int l = rand() % Length;
		
		if (map[h][l] == 1) {
			int p = rand() % TypesOfAmmo;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentAmmo[p];
			ItemsMap[h][l][d].getItem().posH = h;
			ItemsMap[h][l][d].getItem().posL = l;
			ItemsMap[h][l][d].item.invAmmo.count = (rand() % Luck) / 2 + 1;
		} else {
            i--;
        }
	}

	for (int i = 0; i < SCROLLCOUNT; i++) {
		int h = rand() % Height;
		int l = rand() % Length;

		if (map[h][l] == 1) {
			int p = rand() % TypesOfScroll;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentScroll[p];
			ItemsMap[h][l][d].getItem().posH = h;
			ItemsMap[h][l][d].getItem().posL = l;
		}
	}
	for (int i = 0; i < POTIONCOUNT; i++) {
		int h = rand() % Height;
		int l = rand() % Length;

		if (map[h][l] == 1) {
			int p = rand() % TypesOfPotion;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentPotion[p];
			ItemsMap[h][l][d].getItem().posH = h;
			ItemsMap[h][l][d].getItem().posL = l;
		}
	}
	for (int i = 0; i < TOOLSCOUNT; i++) {
		int h = rand() % Height;
		int l = rand() % Length;

		if (map[h][l] == 1) {
			int p = rand() % TypesOfTools;
			int d = rand() % Depth;
			ItemsMap[h][l][d] = differentTools[p];
			ItemsMap[h][l][d].getItem().posH = h;
			ItemsMap[h][l][d].getItem().posL = l;
		}
	}
}

void spawnUnits() {
	for (int i = 0; i < 1; i++) {
		int h = rand() % Height;
		int l = rand() % Length;
		if (map[h][l] == 1 && UnitsMap[h][l].type == UnitEmpty) {
			UnitsMap[h][l] = hero;
			hero.posH = h;
			hero.posL = l;
			break;
		} else {
            i--;
        }
	}
	for (int i = 0; i < ENEMIESCOUNT; i++) {
		int h = rand() % Height;
		int l = rand() % Length;
		if (map[h][l] == 1 && UnitsMap[h][l].type == UnitEmpty) {
			int p = rand() % TypesOfEnemies;
			UnitsMap[h][l] = differentEnemies[p];
			UnitsMap[h][l].getUnit().posH = h;
			UnitsMap[h][l].getUnit().posL = l;
		} else {
            i--;
        }
	}
}

#ifdef DEBUG

void draw() {
	
	//move(0, 0);
    termRend.setCursorPosition(Vec2i{});

	static int mapSaved[FIELD_ROWS][FIELD_COLS] = {};

	for (int i = 0; i < FIELD_ROWS; i++) {
		for (int j = 0; j < FIELD_COLS; j++) {
			mapSaved[i][j] = map[i][j];
		}
	}
	
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Length; j++) {
            char symbol = ' ';
            TextStyle style{ TerminalColor{} };
			if (mapSaved[i][j] != 0) {
				bool near = abs(i - hero.posH) <= 1 && abs(j - hero.posL) <= 1;
/* Here */			
                if (hero.findItemsCountUnderThisCell(i, j) == 0 && UnitsMap[i][j].type == UnitEmpty) {
					switch (mapSaved[i][j]) {
						case 1:
							if (seenUpdated[i][j]) {
								//addch(ACS_BULLET);
                                symbol = '.';
							} else {
								//addch(' ');
							}
							break;
						case 2: {
								//bool u = (i && mapSaved[i - 1][j] == 2);
								//bool r = (j < Length - 1 && mapSaved[i][j + 1] == 2);
								//bool d = (i < Height - 1 && mapSaved[i + 1][j] == 2);
								//bool l = (j && mapSaved[i][j - 1] == 2);
								//int count = u + r + d + l;
								//int attrib = COLOR_PAIR(WHITE_BLACK) | (LIGHT * seenUpdated[i][j]);
                                if (seenUpdated[i][j])
                                    style += TextStyle::Bold;
                                symbol = '#';
                                /*
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
								}*/
								break;
							}
					}
				} else if (hero.findItemsCountUnderThisCell(i, j) == 1 && UnitsMap[i][j].type == UnitEmpty) { /* HERE */
					int MeetedItem = hero.findNotEmptyItemUnderThisCell(i, j);
					switch (ItemsMap[i][j][MeetedItem].getItem().symbol) {
						case 100:
							//addch('%');
                            symbol = '%';
							break;
						case 101:
							//addch('%' | COLOR_PAIR(RED_BLACK));
                            symbol = '%';
                            style += Color::Red;
							break;
						case 300:
							//addch('&' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                            symbol = '&';
                            style += Color::Black;
                            style += TextStyle::Bold;
							break;
						case 301:
							//addch('&' | COLOR_PAIR(YELLOW_BLACK));
                            symbol = '&';
                            style += Color::Yellow;
							break;
						case 400:
							//addch('/' | COLOR_PAIR(RED_BLACK) | LIGHT);
                            symbol = '/';
                            style += Color::Red;
                            style += TextStyle::Bold;
							break;
						case 401:
							//addch('/' | COLOR_PAIR(YELLOW_BLACK));
                            symbol = '/';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
							break;
						case 402:
							//addch('/' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
                            symbol = '/';
                            style += TextStyle::Bold;
							break;
						case 403:
							//addch('/' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
                            symbol = '/';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
							break;
						case 404:
							//addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                            symbol = '/';
                            style += Color::Black;
                            style += TextStyle::Bold;
							break;
						case 405:
							//addch('/' | COLOR_PAIR(WHITE_BLACK));
                            symbol = '/';
							break;
						case 450:
							//addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
                            symbol = ',';
                            style += Color::Black;
                            style += TextStyle::Bold;
							break;
						case 451:
							//addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
                            symbol = ',';
                            style += Color::Red;
                            style += TextStyle::Bold;
							break;
						case 500:
							//addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
                            symbol = '~';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
							break;
						case 501:
							//addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
                            symbol = '~';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
							break;
						case 600:
							//addch('!' | COLOR_PAIR(BLUE_BLACK) | LIGHT);
                            symbol = '!';
                            style += Color::Blue;
                            style += TextStyle::Bold;
							break;
						case 601:
							//addch('!' | COLOR_PAIR(GREEN_BLACK));
                            symbol = '!';
                            style += Color::Green;
							break;
						case 602:
							//addch('!' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                            symbol = '!';
                            style += Color::Black;
                            style += TextStyle::Bold;
							break;
						case 603:
							//addch('!' | COLOR_PAIR(MAGENTA_BLACK) | LIGHT);
                            symbol = '!';
                            style += Color::Magenta;
                            style += TextStyle::Bold;
							break;
						case 604:
							//addch('!' | COLOR_PAIR(YELLOW_BLACK));
                            symbol = '!';
                            style += Color::Yellow;
							break;
						case 700:
							//addch('\\' | COLOR_PAIR(YELLOW_BLACK));
                            symbol = '\\';
                            style += Color::Yellow;
							break;
					}
				} else if (hero.findItemsCountUnderThisCell(i, j) > 1 && UnitsMap[i][j].type == UnitEmpty) { /* Here */
					//addch('^' | COLOR_PAIR(BLACK_WHITE) | LIGHT);
                    symbol = '^';
                    style = TextStyle{ TextStyle::Bold, TerminalColor{ Color::Black, Color::White } };
				}
                if (UnitsMap[i][j].type == UnitHero) { /* Here */
					//addch('@' | COLOR_PAIR(GREEN_BLACK));
                    symbol = '@';
                    style += Color::Green;
				} else if (UnitsMap[i][j].type == UnitEnemy) { /* Here */
                    symbol = '@';
                    switch (UnitsMap[i][j].getUnit().symbol) { /* Here */
						case 201:
							//addch('@' | COLOR_PAIR(YELLOW_BLACK));
                            style += Color::Yellow;
							break;
						case 202:
							//addch('@' | COLOR_PAIR(GREEN_BLACK) | LIGHT);
                            style = TextStyle{ TextStyle::Bold, Color::Green };
							break;
						case 203:
							//addch('@' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                            style = TextStyle{ TextStyle::Bold, Color::Black };
							break;
					}
				}
			} else {
				//addch(' ');	
                symbol = ' ';
			}
            termRend.put(symbol, style);
		}
		
		//printw("\n");
        termRend.put('\n');
		
	}

}

#else

void draw(){
	
	//move(0, 0);
    termRend.setCursorPosition(Vec2i{});

	static int mapSaved[FIELD_ROWS][FIELD_COLS] = {};

	if (MODE == 2 && !hero.isMapInInventory()) {
		for (int i = 0; i < FIELD_ROWS; i++) {
			for (int j = 0; j < FIELD_COLS; j++) {
				mapSaved[i][j] = 0;
			}
		}
	}

	for (int i = 0; i < FIELD_ROWS; i++) {
		for (int j = 0; j < FIELD_COLS; j++) {
			if (seenUpdated[i][j]) {
				int itemsOnCell = hero.findItemsCountUnderThisCell(i, j);
				if (itemsOnCell == 0) {
					mapSaved[i][j] = map[i][j];
				} else if (itemsOnCell == 1) {
					mapSaved[i][j] = ItemsMap[i][j][hero.findNotEmptyItemUnderThisCell(i, j)].getItem().symbol;
				} else {
					mapSaved[i][j] = 100500; // Magic constant that means 'pile'
				}
			}
		}
	}
	
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Length; j++) {
            char symbol = ' ';
            TextStyle style{ TerminalColor{} };
			if (mapSaved[i][j] != 0) {
				bool near = abs(i - hero.posH) <= 1 && abs(j - hero.posL) <= 1;
				if (seenUpdated[i][j]) {
					if (UnitsMap[i][j].type == UnitEmpty) {
						switch (mapSaved[i][j]) {
						case 1:
								//addch(ACS_BULLET);
                            symbol = '.';
							break;
						case 2: {
                                style += TextStyle::Bold;
                                symbol = '#';
								/*bool u = (i && mapSaved[i - 1][j] == 2);
								bool r = (j < Length - 1 && mapSaved[i][j + 1] == 2);
								bool d = (i < Height - 1 && mapSaved[i + 1][j] == 2);
								bool l = (j && mapSaved[i][j - 1] == 2);
								int count = u + r + d + l;
								//int attrib = COLOR_PAIR(WHITE_BLACK) | (LIGHT);
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
								}*/
								break;
							}
                            case 100:
                                //addch('%');
                                symbol = '%';
                                break;
                            case 101:
                                //addch('%' | COLOR_PAIR(RED_BLACK));
                                symbol = '%';
                                style += Color::Red;
                                break;
                            case 300:
                                //addch('&' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                                symbol = '&';
                                style += Color::Black;
                                style += TextStyle::Bold;
                                break;
                            case 301:
                                //addch('&' | COLOR_PAIR(YELLOW_BLACK));
                                symbol = '&';
                                style += Color::Yellow;
                                break;
                            case 400:
                                //addch('/' | COLOR_PAIR(RED_BLACK) | LIGHT);
                                symbol = '/';
                                style += Color::Red;
                                style += TextStyle::Bold;
                                break;
                            case 401:
                                //addch('/' | COLOR_PAIR(YELLOW_BLACK));
                                symbol = '/';
                                style += Color::Yellow;
                                style += TextStyle::Bold;
                                break;
                            case 402:
                                //addch('/' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
                                symbol = '/';
                                style += TextStyle::Bold;
                                break;
                            case 403:
                                //addch('/' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
                                symbol = '/';
                                style += Color::Yellow;
                                style += TextStyle::Bold;
                                break;
                            case 404:
                                //addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                                symbol = '/';
                                style += Color::Black;
                                style += TextStyle::Bold;
                                break;
                            case 405:
                                //addch('/' | COLOR_PAIR(WHITE_BLACK));
                                symbol = '/';
                                break;
                            case 450:
                                //addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
                                symbol = ',';
                                style += Color::Black;
                                style += TextStyle::Bold;
                                break;
                            case 451:
                                //addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
                                symbol = ',';
                                style += Color::Red;
                                style += TextStyle::Bold;
                                break;
                            case 500:
                                //addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
                                symbol = '~';
                                style += Color::Yellow;
                                style += TextStyle::Bold;
                                break;
                            case 501:
                                //addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
                                symbol = '~';
                                style += Color::Yellow;
                                style += TextStyle::Bold;
                                break;
                            case 600:
                                //addch('!' | COLOR_PAIR(BLUE_BLACK) | LIGHT);
                                symbol = '!';
                                style += Color::Blue;
                                style += TextStyle::Bold;
                                break;
                            case 601:
                                //addch('!' | COLOR_PAIR(GREEN_BLACK));
                                symbol = '!';
                                style += Color::Green;
                                break;
                            case 602:
                                //addch('!' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                                symbol = '!';
                                style += Color::Black;
                                style += TextStyle::Bold;
                                break;
                            case 603:
                                //addch('!' | COLOR_PAIR(MAGENTA_BLACK) | LIGHT);
                                symbol = '!';
                                style += Color::Magenta;
                                style += TextStyle::Bold;
                                break;
                            case 604:
                                //addch('!' | COLOR_PAIR(YELLOW_BLACK));
                                symbol = '!';
                                style += Color::Yellow;
                                break;
                            case 700:
                                //addch('\\' | COLOR_PAIR(YELLOW_BLACK));
                                symbol = '\\';
                                style += Color::Yellow;
                                break;
						}
					} else {
						if (UnitsMap[i][j].type == UnitHero) {
							//addch('@' | COLOR_PAIR(GREEN_BLACK));
                            symbol = '@';
                            style += Color::Green;
						} else if (UnitsMap[i][j].type == UnitEnemy && seenUpdated[i][j]) {
                            symbol = '@';
							switch (UnitsMap[i][j].getUnit().symbol) {
								case 201:
									//addch('@' | COLOR_PAIR(YELLOW_BLACK));
                                    style += Color::Yellow;
									break;
								case 202:
									//addch('@' | COLOR_PAIR(GREEN_BLACK) | LIGHT);
                                    style = TextStyle{ TextStyle::Bold, Color::Green };
									break;
								case 203:
									//addch('@' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                                    style = TextStyle{ TextStyle::Bold, Color::Black };
									break;							
							}
						}
					}
				} else {
					switch (mapSaved[i][j]) {
						case 1:
								//addch(' ');
                                symbol = ' ';
							break;
						case 2: {
                                symbol = '#';
                                /*
								bool u = (i && mapSaved[i - 1][j] == 2);
								bool r = (j < Length - 1 && mapSaved[i][j + 1] == 2);
								bool d = (i < Height - 1 && mapSaved[i + 1][j] == 2);
								bool l = (j && mapSaved[i][j - 1] == 2);
								int count = u + r + d + l;
								int attrib = COLOR_PAIR(WHITE_BLACK)/ * | (LIGHT * near)* /;
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
								}*/
								break;
							}
						case 100:
							//addch('%');
                            symbol = '%';
							break;
						case 101:
							//addch('%' | COLOR_PAIR(RED_BLACK));
                            symbol = '%';
                            style += Color::Red;
							break;
						case 300:
							//addch('&' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                            symbol = '&';
                            style += Color::Black;
                            style += TextStyle::Bold;
							break;
						case 301:
							//addch('&' | COLOR_PAIR(YELLOW_BLACK));
                            symbol = '&';
                            style += Color::Yellow;
							break;
						case 400:
							//addch('/' | COLOR_PAIR(RED_BLACK) | LIGHT);
                            symbol = '/';
                            style += Color::Red;
                            style += TextStyle::Bold;
							break;
						case 401:
							//addch('/' | COLOR_PAIR(YELLOW_BLACK));
                            symbol = '/';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
							break;
						case 402:
							//addch('/' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
                            symbol = '/';
                            style += TextStyle::Bold;
							break;
						case 403:
							//addch('/' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
                            symbol = '/';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
							break;
						case 404:
							//addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                            symbol = '/';
                            style += Color::Black;
                            style += TextStyle::Bold;
							break;
						case 405:
							//addch('/' | COLOR_PAIR(WHITE_BLACK));
                            symbol = '/';
							break;
						case 450:
							//addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
                            symbol = ',';
                            style += Color::Black;
                            style += TextStyle::Bold;
							break;
						case 451:
							//addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
                            symbol = ',';
                            style += Color::Red;
                            style += TextStyle::Bold;
							break;
						case 500:
							//addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
                            symbol = '~';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
							break;
						case 501:
							//addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
                            symbol = '~';
                            style += Color::Yellow;
                            style += TextStyle::Bold;
							break;
						case 600:
							//addch('!' | COLOR_PAIR(BLUE_BLACK) | LIGHT);
                            symbol = '!';
                            style += Color::Blue;
                            style += TextStyle::Bold;
							break;
						case 601:
							//addch('!' | COLOR_PAIR(GREEN_BLACK));
                            symbol = '!';
                            style += Color::Green;
							break;
						case 602:
							//addch('!' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
                            symbol = '!';
                            style += Color::Black;
                            style += TextStyle::Bold;
							break;
						case 603:
							//addch('!' | COLOR_PAIR(MAGENTA_BLACK) | LIGHT);
                            symbol = '!';
                            style += Color::Magenta;
                            style += TextStyle::Bold;
							break;
						case 604:
							//addch('!' | COLOR_PAIR(YELLOW_BLACK));
                            symbol = '!';
                            style += Color::Yellow;
							break;
						case 700:
							//addch('\\' | COLOR_PAIR(YELLOW_BLACK));
                            symbol = '\\';
                            style += Color::Yellow;
							break;
					}
				}
			} else {
				//addch(' ');	
                symbol = ' ';
			}
            termRend.put(symbol, style);
		}
		//printw("\n");
        termRend.put('\n');
	}
}

#endif

/*void clearScreen() {
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 180; j++) {
			move(i, j);
			addch(' ');
		}
	}
}*/

void printMenu(const std::vector<std::string> & items, int active) {
    TextStyle activeItemStyle{ TextStyle::Bold, Color::Red };
    std::vector<TextStyle> itemStyles(items.size());
    itemStyles[active - 1] = activeItemStyle;
    for (int i = 1; i <= items.size(); ++i) {
        termRend
            .setCursorPosition(Vec2i{ 0, i })
            .put("{} {}"_format(i, items[i - 1]), itemStyles[i - 1]);
    }
}

void mSettingsMode() {
	int SwitchMode = 1;
	while (true) {
        /*
		move(0, 0);
		printw("Choose mode");

		move(1, 0);
		if (SwitchMode == 1)
		{
			addch('1' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('1');
		printw(" Normal");
		
		move(2, 0);
		if (SwitchMode == 2)
		{
			addch('2' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('2');
		printw(" Hard");
        */
        termRend
            .setCursorPosition(Vec2i{})
            .put("Choose mode");

        printMenu({"Normal", "Hard"}, SwitchMode);

        char input = termRead.readChar();
		switch (input) {
			case CONTROL_DOWN:
				if (SwitchMode < 2)
                    SwitchMode++;
				break;
			case CONTROL_UP:
				if (SwitchMode > 1)
                    SwitchMode--;
				break;
			case '\033':
				//clearScreen();
                termRend.clear();
				return;
				break;
			case CONTROL_CONFIRM:
				switch (SwitchMode) {
					case 1:
						MODE = 1;
						break;
					case 2:
						MODE = 2;
						break;
				}
				break;
		}
		//clearScreen();
        termRend.clear();
	}
}

void mSettingsMap() {
	//clearScreen();
	//move(0, 0);
	//printw("Do you want to load map from file?");
    termRend
        .clear()
        .setCursorPosition(Vec2i{})
        .put("Do you want to load map from file?");
	char inpChar = termRead.readChar();
	if (inpChar == 'y' || inpChar == 'Y') {
		GenerateMap = false;
	}
	//clearScreen();
    termRend.clear();
}

void mSettings() {
	int SwitchSettings = 1;
	//clearScreen();
    termRend.clear();
	while (true) {
		//move(0, 0);
		//printw("Settings");
        termRend
            .setCursorPosition(Vec2i{})
            .put("Settings");

        /*
		move(1, 0);
		if (SwitchSettings == 1)
		{
			addch('1' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('1');
		printw(" Mode");

		move(2, 0);
		if (SwitchSettings == 2)
		{
			addch('2' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('2');
		printw(" Maps");
        */

        printMenu({"Mode", "Maps"}, SwitchSettings);

        char input = termRead.readChar();
		switch (input) {
			case CONTROL_DOWN:
				if (SwitchSettings < 2)
                    SwitchSettings ++;
				break;
			case CONTROL_UP:
				if (SwitchSettings > 1)
                    SwitchSettings --;
				break;
			case CONTROL_CONFIRM: {
				switch (SwitchSettings) {
					case 1: {
						mSettingsMode();
						break;
					}
					case 2: {
						mSettingsMap();
						break;
					}
				}
				break;
			}
			case '\033': {
				MenuCondition = 0;
				//clearScreen();
                termRend.clear();
				return;
				break;
			}
		}
	}
}

void mainMenu() {
    std::vector<string> tips = {
		"lol",
		"kek",
		"azaza",
		"I fukd bugs",
		"seriously? Again?",
		"it's all about the.. Bugs",
		"I used to fuck bugs. Now I fukd them too, but it doesn't matter",
		"bugs, bugs, bugs...",
		"Tip of the day",
		"nice to meet you, lol",
		"now with fixed AI!",
		"guns suck. except one of them, but i won't tell which",
		"do not touch the walls",
		"maybe, some coffee?",
		"by TheyDidItForLulz && Yuri12358!",
		"and what do you hope to find there?",
		"Without GMO. Probably",
		"Cake is a lie",
		"- Hey, Kira. - What? - ... - Fuuuck",
	};


	int Switch = 1;
	int tip = rand() % tips.size();
	while (1)
	{
		if (MenuCondition == 0) {
			//clearScreen();
			//move(0, 0);
			//printw("Welcome to RLRPG /*Tip of the day: %s*/", tips[tip].c_str());
            termRend
                .clear()
                .setCursorPosition(Vec2i{})
                .put("Welcome to RLRPG /* Tip of the day: {} */"_format(tips[tip]));

            printMenu({ "Start game", "Settings", "About", "Help", "Exit" },
                    Switch);

            /*TextStyle activeItemStyle{ TextStyle::Bold, Color::Red };
            TextStyle itemStyles[5];
            itemStyles[Switch] = activeItemStyle;
            std::string items[] = {
                "Start game",
                "Settings",
                "About",
                "Help",
                "Exit"
            };
            for (int i = 1; i <= 5; ++i) {
                termRend
                    .setCursorPosition(Vec2i{ 0, i })
                    .put("{} {}"_format(i, items[i - 1]), itemStyles[i - 1])
            }
            */
            /*
			move(1, 0);
			if (Switch == 1)
			{
				addch('1' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('1');
			printw(" Start game");

			move(2, 0);
			if (Switch == 2)
			{
				addch('2' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('2');
			printw(" Settings");

			move(3, 0);
			if (Switch == 3)
			{
				addch('3' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('3');
			printw(" About");

			//move(4, 0);
            termRend.setCursorPosition(Vec2i{ 0, 4 });
			if (Switch == 4)
				//addch('4' | COLOR_PAIR(RED_BLACK) | LIGHT);
                termRend.put('4', TextStyle{ TextStyle::Bold, Color::Red });
			else
                //addch('4');
                termRend.put('5');
			printw(" Help");

			//move(5, 0);
            termRend.setCursorPosition(Vec2i{ 0, 5 });
			if (Switch == 5)
				//addch('5' | COLOR_PAIR(RED_BLACK) | LIGHT);
                termRend.put('5', TextStyle{ TextStyle::Bold, Color::Red });
			else
                //addch('5');
                termRend.put('5');
			//printw(" Exit");
            termRend.put(" Exit");*/

            char input = termRead.readChar();
			switch (input) {
				case CONTROL_DOWN:
					Switch ++;
					if (Switch > 5)
                        Switch = 5;
					break;
				case CONTROL_UP:
					Switch --;
					if (Switch < 1)
                        Switch = 1;
					break;
				case CONTROL_CONFIRM:
					if (Switch == 1) {
						MenuCondition = 1;
					}
					if (Switch == 2) {
						MenuCondition = 2;
					}
					if (Switch == 5) {
						EXIT = true;
						return;
					}
					break;
			}
		} else if (MenuCondition == 2) {
			mSettings();
		} else if (MenuCondition == 1) {
			return;
		}
	}
}

void readMap() {
	//FILE* file = fopen("map.me", "r");
    std::ifstream file{ "map.me" };
	for (int i = 0; i < FIELD_ROWS; i++) {
		for (int j = 0; j < FIELD_COLS; j++) {
			//fscanf(file, "%d", &map[i][j]);
            file >> map[i][j];
		}
	}
	//fclose(file);
}

int LEVELUP = hero.level * hero.level + 4/* * hero.level + 8*/;

void getXP() {
	if (hero.xp > LEVELUP) {
		hero.level++;
		//sprintf(tmp, "Now you are level %i. ", hero.level);
		//message += tmp;
        message += fmt::format("Now you are level {}. ", hero.level);
		MaxInvItemsWeight += MaxInvItemsWeight / 4;
		DEFAULT_HERO_HEALTH += DEFAULT_HERO_HEALTH / 4;
		hero.health = DEFAULT_HERO_HEALTH;
		LEVELUP = hero.level * hero.level + 4/* * hero.level + 8*/;
	}
}

void setRandomPotionEffects() {
	for (int i = 0; i < TypesOfPotion; i++) {
		int rv = rand() % TypesOfPotion;
		if (differentPotion[rv].effect == 0) {
			differentPotion[rv].effect = i + 1;
		} else {
            i--;
        }
	}
}

int main() {
	//initscr();
	
	//noecho();
	
	//start_color();
    
    termRead.setEchoing(false);
	
    /*
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
    */

	initialize();
	
	mainMenu();
	if (EXIT) { 	
		//endwin();
		return 0;
	}

	if (GenerateMap) {
		generate_maze();
	} else {
		readMap();
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

	//inventory[0].item = LeatherChestplate;
	//inventory[0].type = ItemArmor;
	inventory[0] = LeatherChestplate;
	inventory[0].getItem().inventorySymbol = 'a';
	inventoryVol++;
	hero.heroArmor = &inventory[0];
	hero.heroArmor->getItem().attribute = 201;
	if (rand() % (500 / Luck) == 0)
        hero.heroArmor->getItem().mdf = 2;
	
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
	
	setRandomPotionEffects();

	Enemy Barbarian(0);
	Enemy Zombie(1);
	Enemy Guardian(2);
	differentEnemies[0] = Barbarian;
	differentEnemies[1] = Zombie;
	differentEnemies[2] = Guardian;

	hero.heroWeapon = &inventory[EMPTY_SLOT];

	setItems();

	spawnUnits();

	hero.findVisibleArray();

	int TurnsCounter = 0;
	
	draw();
			
    //move(Height, 0);
    termRend.setCursorPosition(Vec2i{ 0, Height} );
    bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
            hero.health,
            hero.hunger,
            hero.heroArmor->item.invArmor.defence,
            hero.heroWeapon->item.invWeapon.damage,
            hero.level, hero.xp,
            Luck);

    /*
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
	sprintf(tmp, "Lu: %i ", Luck);			
	bar += tmp;*/								
	bar += "Bul: |";
	for (int i = 0; i < BANDOLIER; i++) {
		if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
			//sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invAmmo.count);
			//bar += tmp;
            bar += fmt::format("{}|", inventory[AMMO_SLOT + i].item.invAmmo.count);
		} else {
			bar += "0|";
		}
	}
	bar += " ";
	if (hero.isBurdened)
        bar += "Burdened. ";
	//printw("%- 190s", bar.c_str());
    termRend.put(fmt::sprintf("%- 190s", bar));
	
	if (hero.heroWeapon->type != ItemEmpty) {
		weapon_bar = "";
		weapon_bar += hero.heroWeapon->getItem().getName();
		if (hero.heroWeapon->item.invWeapon.Ranged) {
			weapon_bar += "[";
			for (int i = 0; i < hero.heroWeapon->item.invWeapon.cartridgeSize; i++) {
				if (i < hero.heroWeapon->item.invWeapon.currentCS && (hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 450 ||
					hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 451)) {
					weapon_bar += "i";
				} else {
					weapon_bar += "_";
				}
			}
			weapon_bar += "]";
		}		
		//move(Height + 1, 0);
		//printw("%- 190s", weapon_bar.c_str());
        termRend
            .setCursorPosition(Vec2i{ 0, Height + 1 })
            .put(fmt::sprintf("%- 190s", weapon_bar));
	}

	//move(hero.posH, hero.posL);
    termRend.setCursorPosition(Vec2i{ hero.posL, hero.posH });
	
	while (true) {
		if (EXIT) { 	
			//refresh();
            termRend.display();
			//endwin();
			return 0;
		}

		message = "";
		bar = "";
	
//		sprintf(tmp, "w: %i; Imap: %i; Umap: %i ! ! ", sizeof(Weapon), sizeof(ItemsMap), sizeof(UnitsMap));
//		message += tmp;

		if (hero.hunger < 1) {
			message += "You died from starvation. Press any key to exit. ";
			//move(Height + 2, 0);
			//printw("%- 190s", message.c_str());
			//refresh();
            termRend
                .setCursorPosition(Vec2i{ 0, Height + 2 })
                .put(fmt::sprintf("%- 190s", message))
                .display();
            termRead.readChar();
			//getch();
			//endwin();
			return 0;
		}

		if (hero.health < 1) {
			hero.health = 0;
			message += "You died. Press any key to exit. ";
			//move(Height + 2, 0);
			//printw("% -190s", message.c_str());
			//getch();		
			//refresh();
			//endwin();
            termRend
                .setCursorPosition(Vec2i{ 0, Height + 2 })
                .put(fmt::sprintf("%- 190s", message))
                .display();
            termRead.readChar();
			return 0;
		}

		//move(hero.posH, hero.posL);
        termRend.setCursorPosition(Vec2i{ hero.posL, hero.posH });

		//char inp = getch();
        char inp = termRead.readChar();
	
		hero.moveHero(inp);

		if (!Stop) {
			TurnsCounter++;

			if (TurnsCounter % 25 == 0 && TurnsCounter != 0 && MODE == 1) {
				if (hero.health < DEFAULT_HERO_HEALTH) {
					hero.health ++;
				}
			}

			hero.hunger--;
			
			if (INVISIBILITY > 0)
                INVISIBILITY--;

			if (BLINDNESS > 1) {
                BLINDNESS --;
            } else if(BLINDNESS == 1) {
				BLINDNESS--;
				VISION = DEFAULT_VISION;
			}
		
			if (hero.isBurdened)
                hero.hunger--;

			updateAI();
			
			++turns;

			draw();
			
			move(Height, 0);									
            /*
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
			sprintf(tmp, "Lu: %i ", Luck);								
			bar += tmp;*/
            bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
                    hero.health,
                    hero.hunger,
                    hero.heroArmor->item.invArmor.defence,
                    hero.heroWeapon->item.invWeapon.damage,
                    hero.level, hero.xp,
                    Luck);
            
			bar += "Bul: |";
			for (int i = 0; i < BANDOLIER; i++) {
				if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
					//sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invAmmo.count);
					//bar += tmp;
                    bar += fmt::format("{}|", inventory[AMMO_SLOT + i].item.invAmmo.count);
				} else {
					bar += "0|";
				}
			}
			bar += " ";
			if (hero.isBurdened)
                bar += "Burdened. ";
	
			if (hero.hunger < 75) {	
				bar += "Hungry. ";
			}

			//printw("%- 190s", bar.c_str());
            termRend.put(fmt::sprintf("%- 190s", bar));
		
			if (hero.heroWeapon->type != ItemEmpty) {
				weapon_bar = "";
				weapon_bar += hero.heroWeapon->getItem().getName();
				if (hero.heroWeapon->item.invWeapon.Ranged) {
					weapon_bar += "[";
					for (int i = 0; i < hero.heroWeapon->item.invWeapon.cartridgeSize; i++) {
						if (i < hero.heroWeapon->item.invWeapon.currentCS && (hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 450 ||
							hero.heroWeapon->item.invWeapon.cartridge[i].symbol == 451)) {
							weapon_bar += "i";
						} else {
							weapon_bar += "_";
						}
					}
					weapon_bar += "]";
				}
				//move(Height + 1, 0);
				//printw("%- 190s", weapon_bar.c_str());
                termRend
                    .setCursorPosition(Vec2i{ 0, Height + 1 })
                    .put(fmt::sprintf("%- 190s", weapon_bar));
			}

			//move(Height + 2, 0);
			
			//printw("%- 190s", message.c_str());
            termRend
                .setCursorPosition(Vec2i{ 0, Height + 1 })
                .put(fmt::sprintf("%- 190s", message));
			
			if (inp == '\033') {	
				//move(Height, 0);
				//printw("Are you sure want to exit?\n");
                termRend
                    .setCursorPosition(Vec2i{ 0, Height })
                    .put("Are you sure you want to exit?\n")
                    .display();
				//char inp = getch();
                char inp = termRead.readChar();
				if (inp == 'y' || inp == 'Y' || inp == CONTROL_CONFIRM) {
					//refresh();
					//endwin();
					return 0;
				}
				Stop = true;
			}	
	
			getXP();

			//move(hero.posH, hero.posL);
            termRend.setCursorPosition(Vec2i{ hero.posL, hero.posH });
		} else {
			draw();
            
			/*move(Height, 0);
            //
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
			bar += tmp;											// !!*/
            termRend.setCursorPosition(Vec2i{ 0, Height} );
            bar += fmt::format("HP: {} Sat: {} Def: {} Dmg: {} L/XP: {}/{} Lu: {} ",
                    hero.health,
                    hero.hunger,
                    hero.heroArmor->item.invArmor.defence,
                    hero.heroWeapon->item.invWeapon.damage,
                    hero.level, hero.xp,
                    Luck);

			bar += "Bul: |";
			for (int i = 0; i < BANDOLIER; i++) {
				if (inventory[AMMO_SLOT + i].type != ItemEmpty) {
					//sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invAmmo.count);
					//bar += tmp;
                    bar += fmt::format("{}|", inventory[AMMO_SLOT + i].item.invAmmo.count);
				} else {
					bar += "0|";
				}
			}
			bar += " ";
			if (hero.isBurdened)
                bar += "Burdened. ";							//
			//printw("%- 190s", bar.c_str());									//
		
			if (hero.hunger < 75) {	
				bar += "Hungry. ";
			}
            termRend
                .put(fmt::sprintf("%- 190s", bar))
                .setCursorPosition(Vec2i{ 0, Height + 2 })
                .put(fmt::sprintf("%- 190s", message));

			//move(Height + 2, 0);
			
			//printw("%- 190s", message.c_str());

			Stop = false;
		}
	}
		
	//refresh();
    termRend.display();
	stopLog();
	//endwin();

	return 0;
}
