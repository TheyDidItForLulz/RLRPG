#include<level.hpp>

#include<fstream>

Array2D<int, LEVEL_ROWS, LEVEL_COLS> level;
Array2D<bool, LEVEL_ROWS, LEVEL_COLS> seenUpdated;

void initField() {
    level.forEach([] (int & cell) {
        cell = 1;
    });
}

void readMap() {
    std::ifstream file{ "map.me" };
    level.forEach([&] (int & cell) {
        file >> cell;
    });
}

