#include "Vector.h"

#ifndef __CAMERA_H
#define __CAMERA_H
class Camera {
 public:
 	~Camera() = default;
 	static Camera *cam_instance;
 	static Camera* instance() {
 		if (!cam_instance)
 			cam_instance = new Camera;
 		return cam_instance;
 	}
 	void init(double ex, double ey, double ez,
			  double llx, double lly, double llz,
			  double lrx, double lry, double lrz,
			  double ulx, double uly, double ulz,
			  double urx, double ury, double urz) {
 		cam_instance = new Camera;
 		cam_instance->loc.set(ex, ey, ez);
 		cam_instance->ll.set(llx, lly, llz);
 		cam_instance->lr.set(lrx, lry, lrz);
 		cam_instance->ul.set(ulx, uly, ulz);
 		cam_instance->ur.set(urx, ury, urz);
 	}
 	friend ostream& operator<< (ostream &out, Camera &cam) {
 		out << "Loc(" << cam.loc << "), "
 			<< "ll(" << cam.ll << "), "
 			<< "lr(" << cam.lr << "), "
 			<< "ul(" << cam.ul << "), "
 			<< "ur(" << cam.ur << ")" << endl;
 		return out;
 	}
	Vector loc;
	Vector ll;
	Vector lr;
	Vector ul;
	Vector ur;

 private:
 	Camera() = default;
};
Camera* Camera::cam_instance = nullptr;
#endif