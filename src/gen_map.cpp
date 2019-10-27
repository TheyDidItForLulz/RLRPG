#include<algorithm>
#include<vector>
#include<effolkronium/random.hpp>

#include<game.hpp>
#include<utils.hpp>

using Random = effolkronium::random_static;

const int ROOMS_COUNT = 3;
const int COLS = (LEVEL_COLS - 1) / 2;
const int ROWS = (LEVEL_ROWS - 1) / 2;

Array2D<bool, ROWS, COLS> used;

void mazeNext(Coord2i start, Coord2i prev, Coord2i curr) {
    if (start == curr and used[start]) {
        return;
    }

    auto & level = g_game.level();

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
        auto next = *Random::get(neighbors);
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
            g_game.level().at(r, c) = 1;
        }
    }
}

void generateRooms() {
    for (int i = 0; i < ROOMS_COUNT; ++i) {
        Size2i roomSize{ Random::get(5, 6), Random::get(2, 3) };
        Coord2i upLeftCorner{ Random::get(0, COLS - roomSize.x), Random::get(0, ROWS - roomSize.y) };
        Coord2i downRightCorner = upLeftCorner + roomSize - 1;
        clearRoom(upLeftCorner * 2 + 1, downRightCorner * 2 + 1);
    }
}

#include<iostream>

void generateMaze() {
    Coord2i start;
    mazeNext(start, start, start);                                        
    generateRooms();
}

