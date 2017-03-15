#include<time.h>
#include"include/unit.hpp"
#include"include/utils.hpp"
#include"include/level.hpp"

Unit::Unit(): inventoryVol(0){};

const char* Unit::GetName()
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

bool Unit::LinearVisibilityCheck( double from_x, double from_y, double to_x, double to_y )
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

bool Unit::CanSeeCell( int h, int l )
{
	double offset = 1. / VISION_PRECISION;
	return
	LinearVisibilityCheck( posL + .5, posH + .5, l + offset, h + offset ) ||
	LinearVisibilityCheck( posL + .5, posH + .5, l + offset, h + 1 - offset ) ||
	LinearVisibilityCheck( posL + .5, posH + .5, l + 1 - offset, h + offset ) ||
	LinearVisibilityCheck( posL + .5, posH + .5, l + 1 - offset, h + 1 - offset );
}

void Unit::Delay(double s)
{
	clock_t clocksNow = clock();
	while(double(clock() - clocksNow) / CLOCKS_PER_SEC < s);
}	

Unit::~Unit(){};
