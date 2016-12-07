#ifndef __GEOOBJENT_H
#define __GEOOBJENT_H

#include <cmath>
#include <iostream>

#include "Light.h"
#include "Material.h"
#include "Vector.h"
#include "Transformation.h"

class GeoObject {
 public:
 	GeoObject() = default;
 	GeoObject(Material mtrl_) : mtrl(mtrl_) {}
    ~GeoObject() = default;
 	virtual bool intersect(const Vector &ray_pos, const Vector &ray_dir, double *t, Vector *normal) = 0;
    virtual Vector get_color(const Light &light, const Vector &view, const Vector &pos, const Vector &normal) {
        return light.get_color(view, pos, normal, mtrl);
    }
 	Material mtrl;
};

class Sphere : public GeoObject {
 public:
 	Sphere() = default;
 	Sphere(double x_, double y_, double z_, double radius_, Material mtrl_) :
 		GeoObject(mtrl_), center(x_, y_, z_), radius(radius_) {}
 	~Sphere() = default;
    bool aabb_intersect(const Vector &center, double radius,
                        const Vector &ray_pos, const Vector &ray_dir) {
        Vector min = center - radius;
        Vector max = center + radius;
        float tmin = (min.x - ray_pos.x) / ray_dir.x;
        float tmax = (max.x - ray_pos.x) / ray_dir.x;

        if (tmin > tmax)
            swap(tmin, tmax);

        float tymin = (min.y - ray_pos.y) / ray_dir.y;
        float tymax = (max.y - ray_pos.y) / ray_dir.y;

        if (tymin > tymax)
            swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax))
            return false;

        if (tymin > tmin)
            tmin = tymin;
        if (tymax < tmax)
            tmax = tymax;

        float tzmin = (min.z - ray_pos.z) / ray_dir.z;
        float tzmax = (max.z - ray_pos.z) / ray_dir.z;

        if (tzmin > tzmax)
            swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax))
            return false;

        return true;
    }
 	bool intersect(const Vector &ray_pos, const Vector &ray_dir, double *t, Vector *normal) {
        if (!aabb_intersect(center, radius, ray_pos, ray_dir))
            return false;
 		Vector temp = ray_pos - center;
 		double A = ray_dir.dot(ray_dir);
 		double B = 2 * ray_dir.dot(temp);
 		double N = temp.dot(temp) - sqr(radius);
 		double discriminant = B * B - 4 * A * N;
 		if (discriminant < 0.0)
 			return false;
 		double sqrt_discriminant = sqrt(discriminant);
 		double t0 = (-B + sqrt_discriminant) / (2.0 * A);
 		double t1 = (-B - sqrt_discriminant) / (2.0 * A);
 		if (t0 > t1)
 			t0 = t1;
 		// Ensure that t0 is greater than zero and less than best t.
 		if (t0 > EPS && (t0 < *t)) {
            Vector intersect_pt = ray_pos + ray_dir * t0;
            *normal = intersect_pt - center;
            normal->normalize();
            *t = t0;
 			return true;
 		} else {
 			return false;
 		}
 	}
 	friend ostream& operator<< (ostream &out, Sphere &sph) {
 		out << "S(" << sph.center << ", " << sph.radius << ")" << endl;
 		return out;
 	}
	Vector center;
	double radius;
};

class Ellipsoid : public Sphere {
 public:
    Ellipsoid() = default;
    Ellipsoid(double x_, double y_, double z_, double radius_,
              Material mtrl_, Transformation trans_) :
        Sphere(x_, y_, z_, radius_, mtrl_) {
            // scale, rotate, then translate
            trans_.pre_chain(Scaling(radius_, radius_, radius_), 2);
            trans_.pre_chain(Translation(x_, y_, z_), 0);

            trans = trans_;
            trans_inv = trans_.inverse();
            trans_inv_t = trans_inv.transpose();
        }
    ~Ellipsoid() = default;
    bool intersect(const Vector &ray_pos_t, const Vector &ray_dir_t, double *t, Vector *normal) {
        // transform ray from world space to object space
        Vector ray_pos = trans_inv.apply(ray_pos_t);
        Vector ray_dir = trans_inv.apply_dir(ray_dir_t);

        if (!aabb_intersect(Vector(0, 0, 0), 1.0, ray_pos, ray_dir))
            return false;

        Vector temp = ray_pos + EPS;
        double A = ray_dir.dot(ray_dir);
        double B = 2 * ray_dir.dot(temp);
        double C = temp.dot(temp) - (1.0 + EPS);
        double discriminant = sqr(B) - 4 * A * C;
        if (discriminant < 0.0)
            return false;
        double sqrt_discriminant = sqrt(discriminant);
        double t0 = (-B + sqrt_discriminant) / (2.0 * A);
        double t1 = (-B - sqrt_discriminant) / (2.0 * A);
        if (t0 - t1 > EPS)
            t0 = t1;
        // Transform t0 to world space
        Vector intersection_obj_space = ray_pos + ray_dir * t0;
        Vector intersection_world_space = trans.apply(intersection_obj_space);
        if (fabs(ray_dir_t.x) > EPS)
            t0 = (intersection_world_space.x - ray_pos_t.x) / ray_dir_t.x;
        else if (fabs(ray_dir_t.y) > EPS)
            t0 = (intersection_world_space.y - ray_pos_t.y) / ray_dir_t.y;
        else if (fabs(ray_dir_t.z) > EPS)
            t0 = (intersection_world_space.z - ray_pos_t.z) / ray_dir_t.z;
        else
            t0 = INF + 1.0;
        // Ensure that t0 is greater than zero and less than best t.
        if (t0 > EPS && (t0 < *t)) {
            *t = t0;
            *normal = trans_inv_t.apply_norm(intersection_obj_space);
            normal->normalize();
            return true;
        } else {
            return false;
        }
    }
    Transformation trans, trans_inv, trans_inv_t; // inverse transformation matrix from world space to
                                                  // unit sphere space
};

class Triangle : public GeoObject {
 public:
 	Triangle() = default;
    Triangle(Vector a, Vector b, Vector c, Material mtrl_) :
        Triangle(a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z, mtrl_) {}
 	Triangle(double ax_, double ay_, double az_,
 			 double bx_, double by_, double bz_,
 			 double cx_, double cy_, double cz_,
 			 Material mtrl_) :
 		GeoObject(mtrl_), a(ax_, ay_, az_), b(bx_, by_, bz_), c(cx_, cy_, cz_) {
        double minx, maxx, miny, maxy, minz, maxz;
        minx = min(ax_, min(bx_, cx_));
        maxx = max(ax_, max(bx_, cx_));

        miny = min(ay_, min(by_, cy_));
        maxy = max(ay_, max(by_, cy_));

        minz = min(az_, min(bz_, cz_));
        maxz = max(az_, max(bz_, cz_));
        minvert = Vector(minx, miny, minz);
        maxvert = Vector(maxx, maxy, maxz);
    }
    ~Triangle() = default;
    bool aabb_intersect(const Vector &ray_pos, const Vector &ray_dir) {
        float tmin = (minvert.x - ray_pos.x) / ray_dir.x;
        float tmax = (maxvert.x - ray_pos.x) / ray_dir.x;

        if (tmin > tmax)
            swap(tmin, tmax);

        float tymin = (minvert.y - ray_pos.y) / ray_dir.y;
        float tymax = (maxvert.y - ray_pos.y) / ray_dir.y;

        if (tymin > tymax)
            swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax))
            return false;

        if (tymin > tmin)
            tmin = tymin;
        if (tymax < tmax)
            tmax = tymax;

        float tzmin = (minvert.z - ray_pos.z) / ray_dir.z;
        float tzmax = (maxvert.z - ray_pos.z) / ray_dir.z;

        if (tzmin > tzmax)
            swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax))
            return false;

        return true;
    }
 	bool intersect(const Vector &ray_pos, const Vector &ray_dir, double *t, Vector *normal) {
        if (!aabb_intersect(ray_pos, ray_dir))
            return false;
        // two vectors of plane and normal
        Vector A = b - a;
        Vector B = c - a;
        Vector N = A.cross(B);

        double denominator = N.dot(ray_dir);
        if (fabs(denominator) < EPS) {
            return false; // ray and triangle are parallel
        }

        double D = -N.dot(a);
        double numerator = N.dot(ray_pos) + D;
        double intersect_t = -numerator / denominator;
        if (intersect_t < 0.0)
            return false; // triangle is behind
        Vector intersection = ray_pos + ray_dir * intersect_t;
        Vector edge0 = b - a;
        Vector edge1 = c - b;
        Vector edge2 = a - c;
        Vector c0 = intersection - a;
        Vector c1 = intersection - b;
        Vector c2 = intersection - c;
        bool intersect = false;
        double res0 = N.dot(edge0.cross(c0));
        double res1 = N.dot(edge1.cross(c1));
        double res2 = N.dot(edge2.cross(c2));
        if (res0 > 0 && res1 > 0 && res2 > 0) {
            intersect = true;
        }

        if (intersect && *t > intersect_t) {
            *t = intersect_t;
            *normal = get_normal();
            return true;
        }
        return false;
 	}
    Vector get_normal() {
        Vector A = b - a;
        Vector B = c - a;
        return A.cross(B).normalized();
    }
 	Vector a, b, c;
    Vector minvert, maxvert;
};

#endif
