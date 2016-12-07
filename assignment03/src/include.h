#ifndef __INCLUDE_H
#define __INCLUDE_H

#include <vector>
#include <memory>
#include <map>

#include "Eigen/Dense"
#include "Eigen/Geometry"

using namespace std;

using Eigen::Transform;
using Eigen::Translation;
using Eigen::Affine;
using Eigen::AngleAxis;
using Eigen::Matrix;
using Eigen::Scaling;

typedef float T;
typedef Matrix<T, 1, 4> RowVector4f;
typedef Matrix<T, 2, 1> Vector2f;
typedef Matrix<T, 3, 1> Vector3f;
typedef Matrix<T, 4, 1> Vector4f;
typedef Matrix<T, 4, 4> Matrix4f;

const T eps = 1e-6;

#endif