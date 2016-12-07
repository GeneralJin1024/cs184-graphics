#ifndef __BALLJOINT_H
#define __BALLJOINT_H

#include "include.h"

const double PI = acos(-1.0);

class BallJoint {
 public:
 	BallJoint() {
 		trans = trans.Identity();
 		color[0] = rand()*1.0 / RAND_MAX;
 		color[1] = rand()*1.0 / RAND_MAX;
 		color[2] = rand()*1.0 / RAND_MAX;
 	}
 	BallJoint(T length) : length(length) {
 		trans = trans.Identity();
 		color[0] = rand()*1.0 / RAND_MAX;
 		color[1] = rand()*1.0 / RAND_MAX;
 		color[2] = rand()*1.0 / RAND_MAX;
 	}

 	Vector3f get_end() {
 		return trans * Vector3f(0.0, 0.0, length);
 	}

 	Vector3f draw(Vector3f start) {
 		int num_polygons = 20;

 		Vector3f end(0.0f, 0.0f, length);
 		end = trans * end;
 		Vector3f norm_end = end.normalized();
 		end += start;
 		T rad = 0.5;

 		for (int i = 0; i < num_polygons; i++) {
 			Vector3f p0 = Vector3f(rad * cos((double)i * 2.0 * PI / num_polygons),
 								   rad * sin((double)i * 2.0 * PI / num_polygons),
 								   0.0);
 			Vector3f p1 = Vector3f(rad * cos((double)(i + 1) * 2.0 * PI / num_polygons),
 								   rad * sin((double)(i + 1) * 2.0 * PI / num_polygons),
 								   0.0);
 			p0 = trans * p0;
 			p1 = trans * p1;

 			Vector3f n0 = p0.normalized();
 			Vector3f n1 = p1.normalized();

 			p0 += start;
 			p1 += start;

 			glBegin(GL_TRIANGLES);
 				glColor3f(color[0], color[1], color[2]);
 				glNormal3f(norm_end[0], norm_end[1], norm_end[2]);
 				glVertex3f(end[0], end[1], end[2]);

 				glNormal3f(n0[0], n0[1], n0[2]);
 				glVertex3f(p0[0], p0[1], p0[2]);
 				glNormal3f(n1[0], n1[1], n1[2]);
 				glVertex3f(p1[0], p1[1], p1[2]);
 			glEnd();
 		}
 		glBegin(GL_POLYGON);
	 		glColor3f(color[0], color[1], color[2]);
	 		for (int i = 0; i < num_polygons; i++) {
	 			Vector3f p0 = Vector3f(rad * cos((double)i * 2.0 * PI / num_polygons),
	 								   rad * sin((double)i * 2.0 * PI / num_polygons),
	 								   0.0);
	 			p0 = trans * p0;
	 			Vector3f n0 = p0.normalized();
	 			p0 += start;
	 			glNormal3f(n0[0], n0[1], n0[2]);
	 			glVertex3f(p0[0], p0[1], p0[2]);
	 		}
	 	glEnd();
 		return end;
 	}

 	Vector3f get_x() {
 		return trans * Vector3f(1.0, 0.0, 0.0);
 	}

 	Vector3f get_y() {
 		return trans * Vector3f(0.0, 1.0, 0.0);
 	}

 	Vector3f get_z() {
 		return trans * Vector3f(0.0, 0.0, 1.0);
 	}

 	void transform(AngleAxis<T> transformation) {
 		trans = transformation * trans;
 	}

 	void apply_delta(T rad, Vector3f angle) {
 		trans = AngleAxis<T>(rad, angle) * trans;
 	}

 	void save_transformation() {
 		saved_trans = trans;
 	}

 	void load_transformation() {
 		trans = saved_trans;
 	}

 	T length;
 	AngleAxis<T> trans, saved_trans;
 	Vector3f color;
};

#endif