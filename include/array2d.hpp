#ifndef ARRAY2D_HPP
#define ARRAY2D_HPP

#include<cstddef>
#include<cassert>
#include<functional>
#include<termlib/vec2.hpp>

template<class T, std::size_t Rows, std::size_t Cols>
class Array2D {
public:
    Array2D(): data() {}

    Array2D(T const & val) {
        forEach([&] (T & elem) {
            elem = val;
        });
    }

    T       & operator [](Coord2i c)       { return at(c.y, c.x); }
    T const & operator [](Coord2i c) const { return at(c.y, c.x); }

    T       & at(Coord2i c)       { return at(c.y, c.x); }
    T const & at(Coord2i c) const { return at(c.y, c.x); }

    T & at(std::size_t row, std::size_t col) {
        assert(row >= 0 and col >= 0 and row < Rows and col < Cols);
        return data[row][col];
    }

    T const & at(std::size_t row, std::size_t col) const {
        assert(row >= 0 and col >= 0 and row < Rows and col < Cols);
        return data[row][col];
    }

    Size2i size() const {
        return Size2i{ Cols, Rows };
    }

    bool isIndex(Coord2i cell) const {
        return (cell.x >= 0 and cell.y >= 0 and cell.y < Rows and cell.x < Cols);
    }

    using FnCoord           = std::function<void(Coord2i)>;
    using FnData            = std::function<void(T       &)>;
    using FnConstData       = std::function<void(T const &)>;
    using FnCoordData       = std::function<void(Coord2i, T       &)>;
    using FnCoordConstData  = std::function<void(Coord2i, T const &)>;

    void forEach(FnCoord const & fn) const {
        for (Coord2i c{}; c.y < Rows; ++c.y)
            for (c.x = 0; c.x < Cols; ++c.x)
                fn(c);
    }

    void forEach(FnData const & fn) {
        for (Coord2i c{}; c.y < Rows; ++c.y)
            for (c.x = 0; c.x < Cols; ++c.x)
                fn(at(c));
    }

    void forEach(FnConstData const & fn) const {
        for (Coord2i c{}; c.y < Rows; ++c.y)
            for (c.x = 0; c.x < Cols; ++c.x)
                fn(at(c));
    }

    void forEach(FnCoordData const & fn) {
        for (Coord2i c{}; c.y < Rows; ++c.y)
            for (c.x = 0; c.x < Cols; ++c.x)
                fn(c, at(c));
    }

    void forEach(FnCoordConstData const & fn) const {
        for (Coord2i c{}; c.y < Rows; ++c.y)
            for (c.x = 0; c.x < Cols; ++c.x)
                fn(c, at(c));
    }

private:
    T data[Rows][Cols];
};

#endif // ARRAY2D_HPP

