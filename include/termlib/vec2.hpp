#ifndef VEC_2_HPP
#define VEC_2_HPP

template<class T>
class Vec2 {
public:
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

	Vec2 operator+(Vec2 & v) {
		return Vec2(x + v.x, y + v.y);
	}

	Vec2 operator-(Vec2 & v) {
		return Vec2(x - v.x, y - v.y);
	}

	Vec2 & operator+=(Vec2 & v) {
		x += v.x;
		y += v.y;
		return *this;
	}

	Vec2 & operator-=(Vec2 & v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}

	Vec2 operator+(T s) {
		return Vec2(x + s, y + s);
	}

	Vec2 operator-(T s) {
		return Vec2(x - s, y - s);
	}

	Vec2 operator*(T s) {
		return Vec2(x * s, y * s);
	}

	Vec2 operator/(T s) {
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

typedef Vec2<int> Vec2i;

#endif // VEC_2_HPP

