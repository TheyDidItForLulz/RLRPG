#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>
#include"include/level.hpp"
#include"include/utils.hpp"
#include<string>

#define GREEN_BLACK 1
#define SELECTING 2
#define LIGHT A_BOLD

bool EXIT = false;
bool SAVE = false;

using namespace std;

string bar = "";

int map[FIELD_ROWS][FIELD_COLS] = {};

int tool = 1;
bool filling = 0;
int start_x = -1;
int start_y = -1;

class Builder
{
public:
	Builder(int t)
	{
		switch(t)
		{
			case 1:
				posH = FIELD_ROWS / 2;
				posL = FIELD_COLS / 2;
				type = 1;
				break;
		}
	}
	int posH;
	int posL;
	int type;

	Builder(){}
	~Builder(){}
};

Builder builder(1);

void Draw()
{
	move(0, 0);
	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			if(builder.posH == i && builder.posL == j)
			{
				addch('@' | COLOR_PAIR(GREEN_BLACK) | LIGHT);
			}
			else
			{
				char c;
				switch(map[i][j])
				{
					case 0:
						c = '.';
						break;
					case 1:
						c = '_';
						break;
					case 2:
						c = '#';
						break;
				}
				if( filling && i >= MIN( start_y, builder.posH ) && i <= MAX( start_y, builder.posH )
				&& j >= MIN( start_x, builder.posL ) && j <= MAX( start_x, builder.posL ) )
				{
					addch( c | COLOR_PAIR( SELECTING ) );
				}
				else
				{
					addch( c );
				}
			}
		}
		printw("\n");
	}
}

void MoveBuilder()
{
	char inpch = getch();
	switch(inpch)
	{
		case 'h':
			if(builder.posL > 0) builder.posL --;
			break;
		case 'k':
			if(builder.posH > 0) builder.posH --;
			break;
		case 'j':
			if(builder.posH < Height - 1) builder.posH ++;
			break;
		case 'l':
			if(builder.posL < Length - 1) builder.posL ++;
			break;
		case '\033':
			move(0, FIELD_COLS + 10);
			printw("Do you want to exit without saving?");
			if(getch() == 'y') EXIT = true;
			break;
		case '#':
			tool = 2;
			break;
		case '_':
			tool = 1;
			break;
		case '\040':
			map[builder.posH][builder.posL] = tool;
			break;
		case 'f':
			if( filling )
			{
				filling = false;
				for( int i = MIN( start_y, builder.posH ); i <= MAX( start_y, builder.posH ); ++i )
				{
					for( int j = MIN( start_x, builder.posL ); j <= MAX( start_x, builder.posL ); ++j )
					{
						map[ i ][ j ] = tool;
					}
				}
			}
			else
			{
				filling = true;
				start_x = builder.posL;
				start_y = builder.posH;
			}
			break;
		case 's':
			move(0, FIELD_COLS + 10);
			printw("Do you want to save map?");
			if(getch() == 'y') SAVE = true;
			break;
	}
}

int main()
{
	initscr();
	noecho();
	start_color();

	init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
	init_pair(SELECTING, COLOR_BLACK, COLOR_CYAN);

	builder.posH = 0;
	builder.posL = 0;

	FILE* file = fopen("map.me", "w");

	move(0, 0);

	Draw();
	
	bar += "Tool: ";
	if(tool == 1)
	{
		bar += "_";
	}
	else if(tool = 2)
	{
		bar += "#";
	}
	
	move(Height, 0);
	printw("% -190s", bar.c_str());

	move(builder.posH, builder.posL);

	while(1)
	{
		bar = "";
		MoveBuilder();
		if(EXIT)
		{
			endwin();
			fclose(file);
			return 0;
		}
		if(SAVE)
		{
			break;
		}
		Draw();
	
		bar += "Tool: ";
		if(tool == 1)
		{
			bar += "_";
		}
		else if(tool = 2)
		{
			bar += "#";
		}
		
		move(Height, 0);
		printw("% -190s", bar.c_str());

		move(builder.posH, builder.posL);
	}

	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			fprintf(file, "%i ", map[i][j]);
		}
	}
	
//	fprintf(file, "", fsdf);

	fclose(file);

	endwin();
	return 0;
}
