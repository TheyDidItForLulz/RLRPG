#ifndef UTILS_HPP
#define UTILS_HPP

#include<termlib/vec2.hpp>

template<class T>
inline int sgn(T x) {
    if (x > 0)
        return 1;
    if (x < 0)
        return -1;
    return 0;
}

template<class T>
inline auto sqr(T x) {
    return x * x;
}

template<class T, class U>
inline auto avg(T x, U y) {
    return (x + y) / 2;
}

template<class T, class U, class V>
inline auto clamp(T min, U val, V max) {
    if (max < val)
        return max;
    if (val < min)
        return min;
    return val;
}

void sleep(double sec);

template<class T>
T distSquared(Vec2<T> const & point1, Vec2<T> const & point2) {
    return sqr(point1 - point2);
}

#endif // UTILS_HPP

