#include<level.hpp>

#include<fstream>

int map[FIELD_ROWS][FIELD_COLS];                                            
bool seenUpdated[FIELD_ROWS][FIELD_COLS];                                        // <- visible array

void initField() {
    for (int i = 0; i < FIELD_ROWS; ++i) { 
        for (int j = 0; j < FIELD_COLS; ++j) { 
            map[i][j] = 1; 
        }
    }
}

void readMap() {
    std::ifstream file{ "map.me" };
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            file >> map[i][j];
        }
    }
}

