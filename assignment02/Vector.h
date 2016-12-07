#ifndef __VECTOR_H
#define __VECTOR_H

#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

const double EPS = 1e-6;
const double INF = 1e20;
const double PI = acos(-1.0);

inline double sqr(double x) { return x * x; }

class Vector {
 public:
 	Vector() : x(0.0), y(0.0), z(0.0) {}
 	Vector(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
 	Vector(const Vector &other) {
 		x = other.x;
 		y = other.y;
 		z = other.z;
 	}
 	~Vector() = default;
 	friend Vector operator+ (Vector v, const Vector &other) {
 		v.x += other.x;
 		v.y += other.y;
 		v.z += other.z;
 		return v;
 	}
 	friend Vector operator- (Vector v, const Vector &other) {
 		v.x -= other.x;
 		v.y -= other.y;
 		v.z -= other.z;
 		return v;
 	}
 	friend Vector operator+ (Vector v, double k) {
 		v.x += k;
 		v.y += k;
 		v.z += k;
 		return v;
 	}
 	friend Vector operator- (Vector v, double k) {
 		v.x -= k;
 		v.y -= k;
 		v.z -= k;
 		return v;
 	}
 	friend Vector operator* (double k, Vector v) {
 		v.x *= k;
 		v.y *= k;
 		v.z *= k;
 		return v;
 	}
 	friend Vector operator* (Vector v, double k) {
 		v.x *= k;
 		v.y *= k;
 		v.z *= k;
 		return v;
 	}
 	friend Vector operator/ (Vector v, double k) {
 		v.x /= k;
 		v.y /= k;
 		v.z /= k;
 		return v;
 	}
    friend Vector operator* (Vector v, const Vector &other) {
        v.x *= other.x;
        v.y *= other.y;
        v.z *= other.z;
        return v;
    }
    friend Vector operator/ (Vector v, const Vector &other) {
        v.x /= other.x;
        v.y /= other.y;
        v.z /= other.z;
        return v;
    }
 	void set(double x_, double y_, double z_) {
 		x = x_;
 		y = y_;
 		z = z_;
 	}
    Vector normalized() const {
        Vector vec(x, y, z);
        vec.normalize();
        return vec;
    }
 	void normalize() {
 		double d = sqrt(x * x + y * y + z * z);
	 	if (d > EPS) {
	 		x /= d;
	 		y /= d;
	 		z /= d;
	 	}
 	}
    Vector cross(const Vector &other) {
        return Vector (y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }
 	double dot(const Vector &other) const {
 		return x * other.x + y * other.y + z * other.z;
 	}
    Vector clip() {
        Vector v(x, y, z);
        v.x = max(min(v.x, 1.0), 0.0);
        v.y = max(min(v.y, 1.0), 0.0);
        v.z = max(min(v.z, 1.0), 0.0);
        return v;
    }
    friend Vector operator- (Vector v) {
        v.x = -v.x;
        v.y = -v.y;
        v.z = -v.z;
        return v;
    }
 	friend istream& operator>> (istream &in, Vector &v) {
 		in >> v.x >> v.y >> v.z;
 		return in;
 	}
 	friend ostream& operator<< (ostream &out, const Vector &v) {
 		out << v.x << ", " << v.y << ", " << v.z;
 		return out;
 	}
    double norm() const {
        return sqrt(sqr(x) + sqr(y) + sqr(z));
    }
 	double x, y, z;
};

#endif
