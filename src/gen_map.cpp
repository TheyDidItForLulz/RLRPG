#include<algorithm>
#include<vector>

#include<level.hpp>
#include<utils.hpp>

const int ROOMS_COUNT = 3;
const int COLS = (LEVEL_COLS - 1) / 2;
const int ROWS = (LEVEL_ROWS - 1) / 2;


//extern int map[ FIELD_ROWS ][ FIELD_COLS ];
//int used[ ROWS ][ COLS ];
Array2D<bool, ROWS, COLS> used;

/*struct Pair {                                                            
    int x;
    int y; 
};*/

void mazeNext(Coord2i start, Coord2i prev, Coord2i curr) {
    if (start == curr and used[start]) {
        return;
    }

    used[curr] = true;
    Coord2i cell = curr * 2 + 1;
    for (int i = cell.y - 1; i <= cell.y + 1; ++i) { 
        for (int j = cell.x - 1; j <= cell.x + 1; ++j) {
            level.at(i, j) = 2;
        }                                                    
    }                                                        
    level[cell] = 1;
    level[cell + (prev - curr)] = 1;

    Vec2i dirs[4] = {
        Vec2i{  0, -1 },
        Vec2i{  0,  1 },
        Vec2i{ -1,  0 },
        Vec2i{  1,  0 }
    };

    while (true) {
        std::vector<Coord2i> neighbors;
        for (auto dir : dirs) {
            auto next = curr + dir;
            if (used.isIndex(next) and not used[next]) {
                neighbors.push_back(next);
            }
        }
        if (neighbors.empty())
            return;
        auto next = neighbors[std::rand() % neighbors.size()];
        mazeNext(start, curr, next);
    }
}

void clearRoom(Coord2i a, Coord2i b) {
    if (a.x > b.x)
        std::swap(a.x, b.x);
    if (a.y > b.y)
        std::swap(a.y, b.y);
    for (int r = a.y; r <= b.y; ++r) {
        for (int c = a.x; c <= b.x; ++c) {
            level.at(r, c) = 1;
        }
    }
}

int random(int a, int b) {
    if (a > b)
        std::swap(a, b);
    return a + std::rand() % (b - a + 1);
}

void generateRooms() {
    for (int i = 0; i < ROOMS_COUNT; ++i) {
        Size2i roomSize{ random(5, 6), random(2, 3) };
        Coord2i upLeftCorner{ random(0, COLS - roomSize.x), random(0, ROWS - roomSize.y) };
        /*int r1, r2, c1, c2;
        
        {
            r1 = ( rand() % ROWS ) * 2 + 1;
            r2 = ( rand() % ROWS ) * 2 + 1;
            c1 = ( rand() % COLS ) * 2 + 1;
            c2 = ( rand() % COLS ) * 2 + 1;
        }
        while( std::abs( r1 - r2 ) < 3 or std::abs( r1 - r2 ) > 4 or std::abs( c1 - c2 ) < 11 or std::abs( c1 - c2 ) > 12 );
        clear_room( r1, c1, r2, c2 );*/
        Coord2i downRightCorner = upLeftCorner + roomSize - 1;
        clearRoom(upLeftCorner * 2 + 1, downRightCorner * 2 + 1);
    }
}

void generateMaze() {
    Coord2i start;
    mazeNext(start, start, start);                                        
    generateRooms();
}

