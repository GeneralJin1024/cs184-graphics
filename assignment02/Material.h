#ifndef __MATERIAL_H
#define __MATERIAL_H

#include "Vector.h"

class Material {
 public:
 	Material() = default;
 	Material(double kar_, double kag_, double kab_,
 			 double kdr_, double kdg_, double kdb_,
 			 double ksr_, double ksg_, double ksb_, double ksp_,
 			 double krr_, double krg_, double krb_) :
 		ambient(kar_, kag_, kab_), diffuse(kdr_, kdg_, kdb_),
 		specular(ksr_, ksg_, ksb_), sp_k(ksp_),
 		reflective(krr_, krg_, krb_) {}
 	~Material() = default;
	Vector ambient; // ambient
	Vector diffuse; // diffuse
	Vector specular;
	double sp_k; // specular
	Vector reflective; // reflective
};

#endif
