#include<stdlib.h>
#include"include/level.hpp"

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
															
void generate_maze( void )												
{															
	Pair point = { 0, 0 };												
	maze_next( &point, &point, &point );										
	for( int i = 1; i < FIELD_ROWS - 1; ++i )
	{
		for( int j = 1; j < FIELD_COLS - 1; ++j )
		{
			if( ( i + j ) % 2 && rand() % 100 < 5 )
			{
				map[i][j] = 1;
			}
		}
	}
}															

