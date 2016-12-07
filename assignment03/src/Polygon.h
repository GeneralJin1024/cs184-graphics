#ifndef __POLYGON_H
#define __POLYGON_H

#include "include.h"
#include "TransformableObject.h"

class Polygon : public TransformableObject {
 public:
 	Polygon(int idx) : TransformableObject(idx) {}
 	~Polygon() = default;

 	vector<unique_ptr<Vector3f>> vertices;
 	vector<vector<int>> faces;
};

#endif