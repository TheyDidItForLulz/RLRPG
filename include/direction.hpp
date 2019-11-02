#ifndef DIRECTION_HPP
#define DIRECTION_HPP

#include<termlib/vec2.hpp>
#include<controls.hpp>

#include<tl/optional.hpp>

enum class Direction {
    Left,
    Down,
    Up,
    Right,
    UpLeft,
    UpRight,
    DownLeft,
    DownRight
};

inline char toChar(Direction dir) {
    switch (dir) {
        case Direction::Up: 
        case Direction::Down: return '|';
        case Direction::UpRight:
        case Direction::DownLeft: return '/';
        case Direction::Left:
        case Direction::Right: return '-';
        case Direction::UpLeft:
        case Direction::DownRight: return '\\';
        default: return '?';
    }
}

inline Vec2i toVec2i(Direction dir) {
    switch (dir) {
        case Direction::Up: return Vec2i{ 0, -1 };
        case Direction::UpRight: return Vec2i{ 1, -1 };
        case Direction::Right: return Vec2i{ 1, 0 };
        case Direction::DownRight: return Vec2i{ 1, 1 };
        case Direction::Down: return Vec2i{ 0, 1 };
        case Direction::DownLeft: return Vec2i{ -1, 1 };
        case Direction::Left: return Vec2i{ -1, 0 };
        case Direction::UpLeft: return Vec2i{ -1, -1 };
    }
    return Vec2i{};
}

inline tl::optional<Direction> directionFrom(Vec2i vec) {
    if (vec.x < 0)
        if (vec.y < 0)
            return Direction::UpLeft;
        else if (vec.y > 0)
            return Direction::DownLeft;
        else
            return Direction::Left;
    else if (vec.x > 0)
        if (vec.y < 0)
            return Direction::UpRight;
        else if (vec.y > 0)
            return Direction::DownRight;
        else
            return Direction::Right;
    else
        if (vec.y < 0)
            return Direction::Up;
        else if (vec.y > 0)
            return Direction::Down;
        else
            return {};
}

inline tl::optional<Direction> getDirectionByControl(char control) {
    switch (control) {
        case CONTROL_UP: return Direction::Up;
        case CONTROL_UPRIGHT: return Direction::UpRight;
        case CONTROL_RIGHT: return Direction::Right;
        case CONTROL_DOWNRIGHT: return Direction::DownRight;
        case CONTROL_DOWN: return Direction::Down;
        case CONTROL_DOWNLEFT: return Direction::DownLeft;
        case CONTROL_LEFT: return Direction::Left;
        case CONTROL_UPLEFT: return Direction::UpLeft;
        default: return {};
    }
};

#endif // DIRECTION_HPP
