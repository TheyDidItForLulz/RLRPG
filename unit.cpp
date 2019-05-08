#include<ctime>
#include<thread>

#include"include/unit.hpp"
#include"include/utils.hpp"
#include"include/level.hpp"
#include"include/colors.hpp"
#include"include/controls.hpp"
#include"include/globals.hpp"
#include"include/log.hpp"
#include<termlib/termlib.hpp>

#include<fmt/core.h>
#include<fmt/printf.h>

using namespace fmt::literals;

int g_vision = 16;
int g_maxBurden = 25;								
int DEFAULT_HERO_HEALTH = 15;

extern TerminalRenderer termRend;
extern TerminalReader termRead;

std::string Unit::getName() {
	switch (symbol) {
		case 200:
			return "Hero";
		case 201:
			return "Barbarian";
		case 202:
			return "Zombie";
	}
}

bool Unit::linearVisibilityCheck(double fromX, double fromY, double toX, double toY) {
	double dx = toX - fromX;
	double dy = toY - fromY;
	if (std::abs(dx) > std::abs(dy)) {
		double k = dy / dx;
		int s = sgn(dx);
		for (int i = 0; i * s < dx * s; i += s) {
			int x = fromX + i;
			int y = fromY + i * k;
			if (map[y][x] == 2) {
				return false;
			}
		}
	} else {
		double k = dx / dy;
		int s = sgn(dy);
		for (int i = 0; i * s < dy * s; i += s) {
			int x = fromX + i * k;
			int y = fromY + i;
			if (map[y][x] == 2) {
				return false;
			}
		}
	}
	return true;
}

bool Unit::canSeeCell(int h, int l) {
	double offset = 1. / VISION_PRECISION;
	return
        linearVisibilityCheck(posL + .5, posH + .5, l + offset, h + offset) ||
        linearVisibilityCheck(posL + .5, posH + .5, l + offset, h + 1 - offset) ||
        linearVisibilityCheck(posL + .5, posH + .5, l + 1 - offset, h + offset) ||
        linearVisibilityCheck(posL + .5, posH + .5, l + 1 - offset, h + 1 - offset);
}

Enemy::Enemy(int eType) {
	switch (eType) {
		case 0: {
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
		case 1: {
			health = 10;
			unitInventory[0] = differentWeapon[3];
			unitWeapon = &unitInventory[0];
			inventoryVol = 1;
			symbol = 202;
			vision = 10;
			xpIncreasing = 2;
			break;
		}
		case 2: {
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

void Hero::checkVisibleCells() {
	for (int i = 0; i < FIELD_ROWS; i++) {
		for (int j = 0; j < FIELD_COLS; j++) {
			seenUpdated[i][j] = 0;
			if (sqr(posH - i) + sqr(posL - j) < sqr(g_vision)) {
				seenUpdated[i][j] = canSeeCell(i, j);
			}
		}
	}
}

bool Hero::isInventoryEmpty() {
	for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
		if (inventory[i].type != ItemEmpty)
            return false;
	}
	return true;
}

int Hero::findEmptyInventoryCell() {
	for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
		if (inventory[i].type == ItemEmpty)
            return i;
	}
	return 101010;											// Magic constant, means "Inventory is full".
}	

int Hero::getInventoryItemsWeight() {
	int toReturn = 0;
	for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
		if (inventory[i].type != ItemEmpty) {
			toReturn += inventory[i].getItem().weight;
		}
	}
	return toReturn;
}

void Hero::printList(PossibleItem items[], int len, std::string_view msg, int mode) {
	int num = 0;

    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, num })
        .put(msg);

	num ++;
	switch (mode) {
		case 1: {
			for (int i = 0; i < len; i++) {
                termRend.setCursorPosition(Vec2i{ FIELD_COLS + 10, num });
				if (items[i].getItem().showMdf == true && items[i].getItem().count == 1) {
					if (items[i].getItem().attribute == 100) {
                        termRend.put("[{}] {} {{{}}}. "_format(
                                items[i].getItem().inventorySymbol,
                                items[i].getItem().getName(),
                                items[i].getItem().getMdf()));
					} else {
                        termRend.put("[{}] {} ({}) {{{}}}. "_format(
                                items[i].getItem().inventorySymbol,
                                items[i].getItem().getName(),
                                items[i].getItem().getAttribute(),
                                items[i].getItem().getMdf()));
					}
				} else if (items[i].getItem().count > 1) {
					if (items[i].getItem().attribute == 100) {
                        termRend.put("[{}] {} {{{}}}. "_format(
                                items[i].getItem().inventorySymbol,
                                items[i].getItem().getName(),
                                items[i].getItem().count));
					} else {
                        termRend.put("[{}] {} ({}) {{{}}}. "_format(
                                items[i].getItem().inventorySymbol,
                                items[i].getItem().getName(),
                                items[i].getItem().getAttribute(),
                                items[i].getItem().count));
					}
				} else if (items[i].getItem().attribute == 100) {
                    termRend.put("[{}] {}. "_format(
                            items[i].getItem().inventorySymbol,
                            items[i].getItem().getName()));
				} else {
                    termRend.put("[{}] {} ({}). "_format(
                            items[i].getItem().inventorySymbol,
                            items[i].getItem().getName(),
                            items[i].getItem().getAttribute()));
				}
				num ++;
			}
			break;
		}
		case 2: {
			for (int i = 0; i < len; i++)
			{
                termRend.setCursorPosition(Vec2i{ FIELD_COLS + 10, num });
				if (items[i].getItem().showMdf == true) {
                    termRend.put("[{}] {} ({}) {{{}}}. "_format(
                            i + 'a',
                            items[i].getItem().getName(),
                            items[i].getItem().getAttribute(),
                            items[i].getItem().getMdf()));
				} else {
                    termRend.put("[{}] {} ({}). "_format(
                            i + 'a',
                            items[i].getItem().getName(),
                            items[i].getItem().getAttribute()));
                }
				num ++;
			}
			break;

		}
	}
}

bool Hero::isMapInInventory() {
	for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
		if (inventory[i].type != ItemEmpty && inventory[i].getItem().symbol == 500)
            return true;
	}
	return false;
}

int Hero::findItemsCountUnderThisCell(int h, int l) {
	int result = 0;
	for (int i = 0; i < FIELD_DEPTH; i++) {
		if (itemsMap[h][l][i].type != ItemEmpty) {
			result++;
		}
	}
	return result;
}

int Hero::findEmptyItemUnderThisCell(int h, int l) {
	for (int i = 0; i < FIELD_DEPTH; i++) {
		if (itemsMap[h][l][i].type == ItemEmpty) {
			return i;
		}
	}
	return 101010;											// Magic constant. Means, that something went wrong.
}

int Hero::findNotEmptyItemUnderThisCell(int h, int l) {
	for (int i = 0; i < FIELD_DEPTH; i++) {
		if (itemsMap[h][l][i].type != ItemEmpty) {
			return i;
		}
	}
	return 101010;
}

int Hero::findAmmoInInventory() {
	for (int i = 0; i < BANDOLIER; i++) {
		if (inventory[AMMO_SLOT + i].type == ItemAmmo) {
            return i;
        }
	}
	return 101010;
}

int Hero::findScrollInInventory() {
	for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
		if (inventory[i].type == ItemScroll) {
			return i;
		}
	}
	return 101010;
}

void Hero::printAmmoList(PossibleItem& pAmmo) {										// Picked ammo
	clearRightPane();
    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
        .put("In what slot do vou want to pull your ammo?");
	int choice = 0;
	int num = 0;
	while (1) {
		num = 0;
		for (int i = 0; i < BANDOLIER; i++) {
            termRend.setCursorPosition(Vec2i{ FIELD_COLS + num + 12, 1 });
			num += 2;
            char symbol = '-';
            TextStyle style{ TerminalColor{} };
			if (inventory[AMMO_SLOT + i].type == ItemAmmo) {
				switch (inventory[AMMO_SLOT + i].getItem().symbol) {
					case 450:
                        symbol = ',';
                        style = TextStyle{ TextStyle::Bold, Color::Black };
						break;
					case 451:
                        symbol = ',';
                        style = TextStyle{ TextStyle::Bold, Color::Red };
						break;
					default:
						break;
				}
			}
            if (choice == i) {
                style += TextStyle::Underlined;
            }
            termRend.put(symbol, style);
		}
        char input = termRead.readChar();
		switch (input) {
			case CONTROL_LEFT:
				if (choice > 0)
                    choice--;
				break;
			case CONTROL_RIGHT:
				if (choice < BANDOLIER - 1)
                    choice++;
				break;
			case CONTROL_CONFIRM:
				if (inventory[AMMO_SLOT + choice].item.invAmmo.symbol != pAmmo.item.invAmmo.symbol && inventory[AMMO_SLOT + choice].type == ItemAmmo) {
					PossibleItem buffer;
					buffer = pAmmo;
					pAmmo = inventory[AMMO_SLOT + choice];
					inventory[AMMO_SLOT + choice] = buffer;
				} else if (inventory[AMMO_SLOT + choice].item.invAmmo.symbol == pAmmo.item.invAmmo.symbol && inventory[AMMO_SLOT + choice].type == ItemAmmo) {
					inventory[AMMO_SLOT + choice].item.invAmmo.count += pAmmo.item.invAmmo.count;
					pAmmo.type = ItemEmpty;
				} else if (inventory[AMMO_SLOT + choice].type == ItemEmpty) {
					inventory[AMMO_SLOT + choice] = pAmmo;
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

void Hero::pickUp() {
	if (findItemsCountUnderThisCell(posH, posL) == 0) {
		message += "There is nothing here to pick up. ";
		g_stop = true;
		return;
	}
	else if (findItemsCountUnderThisCell(posH, posL) == 1)
	{
		int num = findNotEmptyItemUnderThisCell(posH, posL);

        message += "You picked up {}. "_format(itemsMap[posH][posL][num].getItem().getName());

		if (itemsMap[posH][posL][num].type == ItemAmmo)
		{
			printAmmoList(itemsMap[posH][posL][num]);
			return;
		}

		bool couldStack = false;

		if (itemsMap[posH][posL][num].getItem().isStackable)
		{
			for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
			{
				if (inventory[i].type != ItemEmpty && inventory[i].getItem().symbol == itemsMap[posH][posL][num].getItem().symbol)
				{
					couldStack = true;
					inventory[i].getItem().count += itemsMap[posH][posL][num].getItem().count;
					itemsMap[posH][posL][num].type = ItemEmpty;
				}
			}
		}

		if (!couldStack)
		{
			int eic = findEmptyInventoryCell();
			if (eic != 101010)
			{
				inventory[eic] = itemsMap[posH][posL][num];
				inventory[eic].getItem().inventorySymbol = eic + 'a';
				log("Item index: '%c'\n", inventory[eic].getItem().inventorySymbol);
				itemsMap[posH][posL][num].type = ItemEmpty;
				inventoryVol++;
			}
			else
			{
				message += "Your inventory is full, motherfuck'a! ";
			}
		}

		if (getInventoryItemsWeight() > g_maxBurden && !isBurdened)
		{
			message += "You're burdened. ";
			isBurdened = true;
		}

		return;
	}
	
	PossibleItem list[FIELD_DEPTH];
	int len = 0;

	for (int i = 0; i < FIELD_DEPTH; i++) {	
		if (itemsMap[posH][posL][i].type != ItemEmpty) {
			list[len] = itemsMap[posH][posL][i];
			len++;
		}
	}

	printList(list, len, "What do you want to pick up? ", 2);
	len = 0;

	char choice = termRead.readChar();

	if (choice == '\033')
        return;

	int intch = choice - 'a';
	
	int helpfulArray[FIELD_DEPTH], hACounter = 0;

	for (int i = 0; i < FIELD_DEPTH; i++)
	{
		if (itemsMap[posH][posL][i].type != ItemEmpty)
		{
			helpfulArray[hACounter] = i;
			hACounter++;
		}
	}

	if (itemsMap[posH][posL][helpfulArray[intch]].type != ItemEmpty)
	{
        message += "You picked up %s. "_format(itemsMap[posH][posL][helpfulArray[intch]].getItem().getName());
		
		if (itemsMap[posH][posL][helpfulArray[intch]].type == ItemAmmo)
		{
			printAmmoList(itemsMap[posH][posL][helpfulArray[intch]]);
			return;
		}

		bool couldStack = false;

		if (itemsMap[posH][posL][helpfulArray[intch]].getItem().isStackable)
		{
			for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
			{
				if (inventory[i].type != ItemEmpty && inventory[i].getItem().symbol == itemsMap[posH][posL][helpfulArray[intch]].getItem().symbol)
				{
					couldStack = true;
					inventory[i].getItem().count += itemsMap[posH][posL][helpfulArray[intch]].getItem().count;
					itemsMap[posH][posL][helpfulArray[intch]].type = ItemEmpty;
				}
			}
		}

		if (!couldStack)
		{
			int eic = findEmptyInventoryCell();
			if (eic != 101010)
			{
				inventory[eic] = itemsMap[posH][posL][helpfulArray[intch]];
				inventory[eic].getItem().inventorySymbol = eic + 'a';
				itemsMap[posH][posL][helpfulArray[intch]].type = ItemEmpty;
				inventoryVol++;
			}
			else
			{
				message += "Your inventory is full, motherfuck'a! ";
			}
		}
	}

	if (getInventoryItemsWeight() > g_maxBurden && !isBurdened)
	{
		message += "You're burdened. ";
		isBurdened = true;
	}
}

bool Hero::isFoodInInventory()
{
	for (int i = 0; i < MAX_USABLE_INV_SIZE; i++)
	{
		if (inventory[i].type == ItemFood) return true;
	}
	return false;
}

bool Hero::isArmorInInventory()
{
	for (int i = 0; i < MAX_USABLE_INV_SIZE; i++)
	{
		if (inventory[i].type == ItemArmor) return true;
	}
	return false;
}

bool Hero::isWeaponOrToolsInInventory()
{
	for (int i = 0; i < MAX_USABLE_INV_SIZE; i++)
	{
		if (inventory[i].type == ItemWeapon || inventory[i].type == ItemTools) return true;
	}
	return false;
}

bool Hero::isPotionInInventory()
{
	for (int i = 0; i < MAX_USABLE_INV_SIZE; i++)
	{
		if (inventory[i].type == ItemPotion) return true;
	}
	return false;
}

void Hero::clearRightPane()
{
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 50; j++)
		{
            termRend
                .setCursorPosition(Vec2i{ FIELD_COLS + j + 10, i })
                .put(' ');
		}
	}
}

void Hero::eat() {
	if (isFoodInInventory()) {
		showInventory(CONTROL_EAT);
	} else {
        message += "You don't have anything to eat. ";
    }
}

void Hero::moveHero(char inp)
{
	int a1 = 0, a2 = 0;
	
	switch (inp) {
		
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
			if (isInventoryEmpty() == false)
			{
				showInventory(CONTROL_SHOWINVENTORY);
			}
			else
			{
				message += "Your inventory is empty. ";
			}
/*->*/				g_stop = true;
			break;				
		}
		case CONTROL_WEAR:
		{

			if (isArmorInInventory() == true)
			{
				showInventory(CONTROL_WEAR);
			}
			else message += "You don't have anything to wear. ";
/*->*/				g_stop = true;
			break;

		}
		case CONTROL_WIELD:
		{
			if (isWeaponOrToolsInInventory() == true)
			{
				showInventory(CONTROL_WIELD);
			}
			else message += "You don't have anything to wield. ";
/*->*/				g_stop = true;
			break;
		}
		case CONTROL_TAKEOFF:
		{
			showInventory(CONTROL_TAKEOFF);
/*->*/				g_stop = true;
			break;
		}
		case CONTROL_UNEQUIP:
		{
			showInventory(CONTROL_UNEQUIP);
/*->*/				g_stop = true;
			break;
		}
		case CONTROL_DROP:
		{
			if (isInventoryEmpty() == false)
			{
				showInventory(CONTROL_DROP);
			}
/*->*/				g_stop = true;
			break;		
		}
		case CONTROL_THROW:
		{
			if (isInventoryEmpty() == false)
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
			if (isPotionInInventory() == true)
			{
				showInventory(CONTROL_DRINK);
			}
/*->*/				g_stop = true;
			break;
		}
		case CONTROL_OPENBANDOLIER:
		{
			if (findAmmoInInventory() != 101010)
			{
				showInventory(CONTROL_OPENBANDOLIER);
			}
			else message += "Your bandolier is empty. ";
/*->*/				g_stop = true;
			break;
		}
		case CONTROL_RELOAD:
		{
			if (!heroWeapon->item.invWeapon.Ranged)
			{
				message += "You have no ranged weapon in hands. ";
				g_stop = true;
			}
			else if (findAmmoInInventory() != 101010)
			{
				showInventory(CONTROL_RELOAD);
			}
			else
			{
				message += "You have no bullets to reload. ";
				g_stop = true;
			}
			break;
		}
		case CONTROL_READ:
		{
			if (findScrollInInventory() != 101010)
			{
				showInventory(CONTROL_READ);
			}
			else message += "You don't have anything to read. ";
/*->*/				g_stop = true;
			break;
		}
		case '\\':
		{
			char hv = termRead.readChar();
			
			if (hv == 'h')
			{
				if (termRead.readChar() == 'e')
				{
					if (termRead.readChar() == 'a')
					{
						if (termRead.readChar() == 'l')
						{
							hunger = 3000;
							health = DEFAULT_HERO_HEALTH * 100;
						}
					}
				}
			}
		
			if (hv == 'w')
			{
				if (termRead.readChar() == 'a')
				{
					if (termRead.readChar() == 'l')
					{
						if (termRead.readChar() == 'l')
						{
							if (termRead.readChar() == 's')
							{
								canMoveThroughWalls = true;
							}
						}
					}
				}
			}
			else if (hv == 'd')
			{
				if (termRead.readChar() == 's')
				{
					if (termRead.readChar() == 'c')
					{
						canMoveThroughWalls = false;
					}
				}
				else
				{
					itemsMap[1][1][0] = differentFood[0];
				}

			}
			else if (hv == 'k')
			{
				if (termRead.readChar() == 'i')
				{
					if (termRead.readChar() == 'l')
					{
						if (termRead.readChar() == 'l')
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

PossibleUnit::PossibleUnit(UnitedUnits u, UnitType t): type(t) {
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

PossibleUnit::PossibleUnit(const PossibleUnit& p) {
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

PossibleUnit& PossibleUnit::operator=(const PossibleUnit& p) {
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

Unit& PossibleUnit::getUnit() {
    switch (type) {
        case UnitEmpty:
            return unit.uEmpty;
        case UnitHero:
            return unit.uHero;
        case UnitEnemy:
            return unit.uEnemy;
    }        
}
