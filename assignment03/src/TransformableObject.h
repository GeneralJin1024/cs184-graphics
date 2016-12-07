#ifndef __TRANSFORMABLE_OBJECT_H
#define __TRANSFORMABLE_OBJECT_H

#include "include.h"

class TransformableObject {
 public:
 	TransformableObject(int idx) {
 		srand(time(0) * idx);
	    for (int i = 0; i < 3; i++) {
	    	mat_ambient[i] = rand() * 1.0 / (0.8 * RAND_MAX);
	    	mat_diffuse[i] = rand() * 1.0 / (0.8 * RAND_MAX);
	    	mat_specular[i] = rand() * 1.0 / (0.8 * RAND_MAX);
	    }
	    shininess = rand() * 1.0 / RAND_MAX * 20 + 15;
	    transformation = Transform<T, 3, Affine>::Identity();
 	}
 	~TransformableObject() = default;

 	Transform<T, 3, Affine> transformation;
 	float mat_ambient[3], mat_diffuse[3], mat_specular[3], shininess;
};

#endif