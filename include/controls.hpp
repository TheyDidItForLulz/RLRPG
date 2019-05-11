#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include<stdexcept>
#include<termlib/vec2.hpp>
#include<optional>

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

inline std::optional<Direction> directionFrom(Vec2i vec) {
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

#define CONTROL_UP 'k'
#define CONTROL_DOWN 'j'
#define CONTROL_LEFT 'h'
#define CONTROL_RIGHT 'l'
#define CONTROL_UPLEFT 'y'
#define CONTROL_UPRIGHT 'u'
#define CONTROL_DOWNLEFT 'b'
#define CONTROL_DOWNRIGHT 'n'
#define CONTROL_PICKUP ','
#define CONTROL_EAT 'e'
#define CONTROL_SHOWINVENTORY 'i'
#define CONTROL_WEAR 'W'
#define CONTROL_WIELD 'w'
#define CONTROL_TAKEOFF 'T'
#define CONTROL_UNEQUIP '-'
#define CONTROL_DROP 'd'
#define CONTROL_THROW 't'
#define CONTROL_SHOOT 's'
#define CONTROL_DRINK 'q'
#define CONTROL_EXCHANGE 'x'
#define CONTROL_CONFIRM ' '
#define CONTROL_READ 'r'
#define CONTROL_OPENBANDOLIER 'a'
#define CONTROL_RELOAD 'R'

inline Direction getDirectionByControl(char control) {
    switch (control) {
        case CONTROL_UP: return Direction::Up;
        case CONTROL_UPRIGHT: return Direction::UpRight;
        case CONTROL_RIGHT: return Direction::Right;
        case CONTROL_DOWNRIGHT: return Direction::DownRight;
        case CONTROL_DOWN: return Direction::Down;
        case CONTROL_DOWNLEFT: return Direction::DownLeft;
        case CONTROL_LEFT: return Direction::DownLeft;
        case CONTROL_UPLEFT: return Direction::UpLeft;
        default: throw std::logic_error("No direction for the provided control");
    }
};

#endif // CONTROLS_HPP
