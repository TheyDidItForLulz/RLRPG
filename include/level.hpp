#ifndef LEVEL_HPP
#define LEVEL_HPP

// Should be odd numbers
const int FIELD_COLS = 81;
const int FIELD_ROWS = 21;
															//
extern int map[FIELD_ROWS][FIELD_COLS];											//
extern bool seenUpdated[FIELD_ROWS][FIELD_COLS];

void initField();
void readMap();

#endif // LEVEL_HPP
