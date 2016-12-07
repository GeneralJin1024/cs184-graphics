#ifndef __LIGHT_H
#define __LIGHT_H

#include <cmath>

#include "Material.h"
#include "Vector.h"

class Light {
 public:
 	Light() = default;
    Light(double r_, double g_, double b_, bool is_ambient_, int falloff_) :
        color(r_, g_, b_), is_ambient(is_ambient_), falloff(falloff_) {}
 	Light(double x_, double y_, double z_,
          double r_, double g_, double b_, bool is_ambient_, int falloff_) :
        vec(x_, y_, z_), color(r_, g_, b_), is_ambient(is_ambient_), falloff(falloff_) {}
 	~Light() = default;
    virtual double get_dist(const Vector &pos) const = 0;
 	virtual Vector direction(const Vector &pos) const = 0;
    virtual Vector get_color(const Vector &view, const Vector &pos,
                             const Vector &normal, const Material &mtrl) const {
        Vector light_dir = direction(pos);
        Vector reflected = (2 * normal.dot(light_dir) * normal) - light_dir;
        reflected.normalize();

        double LdotN = max(light_dir.dot(normal), 0.0);
        double RdotV = max(reflected.dot(view), 0.0);
        double dF = max(LdotN, 0.0);
        double sF = pow(RdotV, mtrl.sp_k);

        Vector ambient = mtrl.ambient * color;
        Vector diffuse = (mtrl.diffuse * color) * dF;
        Vector specular = (mtrl.specular * color) * sF;
        Vector intensity = ambient + diffuse + specular;
        if (falloff) {
            double intersection_dist = get_dist(pos);
            intensity = (1.0 / pow(intersection_dist, falloff)) * intensity;
        }
        return intensity.clip();
    }
    Vector vec;
	Vector color;
    bool is_ambient, is_spotlight;
    int falloff;
};

class AmbientLight : public Light {
 public:
 	AmbientLight() = default;
 	AmbientLight(double r_, double g_, double b_) :
        Light(r_, g_, b_, true, 0) {}
 	~AmbientLight() = default;
 	Vector direction(const Vector &pos) const {
        return Vector();
    };
    double get_dist(const Vector &pos) const {
        return 0.0;
    }
};

class PointLight : public Light {
 public:
 	PointLight() = default;
 	PointLight(double px_, double py_, double pz_,
 			   double r_, double g_, double b_, int falloff_) :
 		       Light(px_, py_, pz_, r_, g_, b_, false, falloff_) {}
    ~PointLight() = default;
	Vector direction(const Vector &pos) const {
		Vector dir = vec - pos;
		dir.normalize();
		return dir;
	}
    double get_dist(const Vector &pos) const {
        Vector temp = vec - pos;
        return temp.norm();
    }
};

class SpotLight : public PointLight {
 public:
    SpotLight() = default;
    SpotLight(double px_, double py_, double pz_,
              double dx_, double dy_, double dz_,
              double r_, double g_, double b_,
              double beamAngle_, double falloffAngle_) :
        PointLight(px_, py_, pz_, r_, g_, b_, 0), dir(dx_, dy_, dz_),
        beamAngle(beamAngle_ * PI / 180.0), falloffAngle(falloffAngle_ * PI / 180.0) {
        is_spotlight = true;
    }
    ~SpotLight() = default;
    Vector get_color(const Vector &view, const Vector &pos,
                     const Vector &normal, const Material &mtrl) const {
        Vector surface_dir = (pos - vec).normalized();
        double angle = acos(dir.dot(surface_dir));
        if (angle > beamAngle + falloffAngle)
            return Vector(0.0, 0.0, 0.0);

        Vector light_dir = direction(pos);
        Vector reflected = (2 * normal.dot(light_dir) * normal) - light_dir;
        reflected.normalize();

        double LdotN = max(light_dir.dot(normal), 0.0);
        double RdotV = max(reflected.dot(view), 0.0);
        double dF = max(LdotN, 0.0);
        double sF = pow(RdotV, mtrl.sp_k);

        Vector ambient = mtrl.ambient * color;
        Vector diffuse = (mtrl.diffuse * color) * dF;
        Vector specular = (mtrl.specular * color) * sF;
        Vector intensity = ambient + diffuse + specular;
        if (angle > beamAngle) {
            double portion = 1.0 - ((angle - beamAngle) / falloffAngle);
            return (intensity * portion).clip();
        }
        return intensity.clip();
    }
    Vector dir;
    double beamAngle;
    double falloffAngle;
};

class DirectionalLight : public Light {
 public:
 	DirectionalLight() = default;
 	DirectionalLight(double dx_, double dy_, double dz_,
 					 double r_, double g_, double b_) :
 		Light(dx_, dy_, dz_, r_, g_, b_, false, 0) {
        vec.normalize();
    }
 	~DirectionalLight() = default;
 	Vector direction(const Vector &pos) const {
 		return -vec;
 	}
    double get_dist(const Vector &pos) const {
        return INF;
    }
};

#endif
