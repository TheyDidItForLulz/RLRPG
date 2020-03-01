#ifndef VEC_2_HPP
#define VEC_2_HPP

template<class T>
struct Vec2 {
    T x, y;

    Vec2(): x(0), y(0) {}
    Vec2(T val): x(val), y(val) {}
    Vec2(T x, T y): x(x), y(y) {}

    template<class U>
    Vec2(const Vec2<U> & v) : x(v.x), y(v.y) {}

    template<class U>
    Vec2& operator=(const Vec2<U> & v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    template<class U>
    bool operator==(const Vec2<U> & v) const {
        return x == v.x and y == v.y;
    }

    template<class U>
    bool operator!=(const Vec2<U> & v) const {
        return x != v.x or y != v.y;
    }

    Vec2 operator+(const Vec2 & v) const {
        return Vec2(x + v.x, y + v.y);
    }

    Vec2 operator-(const Vec2 & v) const {
        return Vec2(x - v.x, y - v.y);
    }

    T operator*(const Vec2 & v) const {
        return x * v.x + y * v.y;
    }

    Vec2 & operator+=(const Vec2 & v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vec2 & operator-=(Vec2 & v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vec2 operator+(T s) const {
        return Vec2(x + s, y + s);
    }

    Vec2 operator-(T s) const {
        return Vec2(x - s, y - s);
    }

    Vec2 operator*(T s) const {
        return Vec2(x * s, y * s);
    }

    Vec2 operator/(T s) const {
        return Vec2(x / s, y / s);
    }

    Vec2 & operator+=(T s) {
        x += s;
        y += s;
        return *this;
    }

    Vec2 & operator-=(T s) {
        x -= s;
        y -= s;
        return *this;
    }

    Vec2 & operator*=(T s) {
        x *= s;
        y *= s;
        return *this;
    }

    Vec2 & operator/=(T s) {
        x /= s;
        y /= s;
        return *this;
    }

    void set(T x, T y) {
        this->x = x;
        this->y = y;
    }
};

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Coord2i = Vec2i;
using Size2i = Vec2i;

#endif // VEC_2_HPP

