#include<stdlib.h>
#include"include/level.hpp"
#include"include/utils.hpp"

#define ROOMS_COUNT 3

extern int map[ FIELD_ROWS ][ FIELD_COLS ];
int used[ ROWS ][ COLS ];

struct Pair														
{															
	int x;														
	int y;														
};															

void maze_next( Pair* start, Pair* prev, Pair* curr )									
{															
	if( curr->x == start->x && curr->y == start->y && used[ start->y ][ start->x ])					
	{														
		return;													
	}														
															
															
	used[ curr->y ][ curr->x ] = 1;											
	Pair f = { curr->x * 2 + 1, curr->y * 2 + 1 };									
	for( int i = f.y - 1; i <= f.y + 1; ++i )									
	{														
		for( int j = f.x - 1; j <= f.x + 1; ++j )								
		{													
			map[ i ][ j ] = 2;										
		}													
	}														
	map[ f.y ][ f.x ] = map[ f.y + prev->y - curr->y ][ f.x + prev->x - curr->x ] = 1;				
															
	Pair stack[ 4 ];												
	int sp;														

	do														
	{														
		sp = 0;													
		if( curr->x && !used[ curr->y ][ curr->x - 1 ] )							
		{													
			stack[ sp ].x = curr->x - 1;									
			stack[ sp++ ].y = curr->y;									
		}													
		if( curr->x + 1 < COLS && !used[ curr->y ][ curr->x + 1 ] )						
		{													
			stack[ sp ].x = curr->x + 1;									
			stack[ sp++ ].y = curr->y;									
		}													
		if( curr->y && !used[ curr->y - 1 ][ curr->x ] )							
		{													
			stack[ sp ].x = curr->x;									
			stack[ sp++ ].y = curr->y - 1;									
		}													
		if( curr->y + 1 < ROWS && !used[ curr->y + 1 ][ curr->x ] )						
		{													
			stack[ sp ].x = curr->x;									
			stack[ sp++ ].y = curr->y + 1;									
		}													
		if( !sp )												
		{													
			return;												
		}													
		int rand_i = rand() % sp;										
		maze_next( start, curr, stack + rand_i );								
	}														
	while( 1 );													
}															

void clear_room( int r1, int c1, int r2, int c2 )
{
	int minr = MIN( r1, r2 );
	int maxr = MAX( r1, r2 );
	int minc = MIN( c1, c2 );
	int maxc = MAX( c1, c2 );
	for( int i = minr; i <= maxr; ++i )
	{
		for( int j = minc; j <= maxc; ++j )
		{
			map[i][j] = 1;
		}
	}
}

void generate_rooms( void )
{
	for( int i = 0; i < ROOMS_COUNT; ++i )
	{
		int r1, r2, c1, c2;
		do
		{
			r1 = ( rand() % ROWS ) * 2 + 1;
			r2 = ( rand() % ROWS ) * 2 + 1;
			c1 = ( rand() % COLS ) * 2 + 1;
			c2 = ( rand() % COLS ) * 2 + 1;
		}
		while( ABS( r1 - r2 ) < 3 || ABS( r1 - r2 ) > 4 || ABS( c1 - c2 ) < 11 || ABS( c1 - c2 ) > 12 );
		clear_room( r1, c1, r2, c2 );
	}
}

void generate_maze( void )												
{															
	Pair point = { 0, 0 };												
	maze_next( &point, &point, &point );										
	generate_rooms();
}															

