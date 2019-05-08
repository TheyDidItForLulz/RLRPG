#ifndef LEVEL_HPP
#define LEVEL_HPP

#define COLS 40
#define ROWS 10
#define FIELD_COLS ( COLS * 2 ) + 1											//
#define FIELD_ROWS ( ROWS * 2 ) + 1											//
															//
extern int map[FIELD_ROWS][FIELD_COLS];											//
extern bool seenUpdated[FIELD_ROWS][FIELD_COLS];

#define FLOOR 1

#endif // LEVEL_HPP
