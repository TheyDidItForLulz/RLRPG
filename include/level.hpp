#ifndef LEVEL_HPP
#define LEVEL_HPP

#include<array2d.hpp>

// Should be odd numbers
const int LEVEL_COLS = 81;
const int LEVEL_ROWS = 21;

extern Array2D<int, LEVEL_ROWS, LEVEL_COLS> level;
extern Array2D<bool, LEVEL_ROWS, LEVEL_COLS> seenUpdated;

void initField();
void readMap();

#endif // LEVEL_HPP
