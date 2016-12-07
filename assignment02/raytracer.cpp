/**
 * Raytracer - CS 184 Assignment 02
 *
 * Arguments:
 * cam ex ey ez llx lly llz lrx lry lrz ulx uly ulz urx ury urz
 * sph cx cy cz r
 * tri ax ay az bx by bz cx cy cz
 * obj "filename"
 * ltp px py pz r g b [falloff=0,1,2 for none, linear, quadratic]
 * ltd dx dy dz r g b
 * lta r g b
 * mat kar kag kab kdr kdg kdb ksr ksg ksb ksp krr krg krb
 *
 * Tranformations:
 * xft tx ty tz -> translation
 * xfr rx ry rz -> rotation
 * xfs sx sy sz -> scaling
 * xfz -> reset to identity
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <climits>
#include <map>
#include <sstream>
#include "Light.h"
#include "GeoObject.h"
#include "Camera.h"
#include "Material.h"
#include "Vector.h"
#include "Transformation.h"
#include "pngwriter/src/pngwriter.h"

using namespace std;

void load_mesh(const string &obj_filename, const Material &mtrl);

typedef pair<int, int> pii;

vector<GeoObject *> world_objects;
vector<Light *> world_lights;

const int HEIGHT = 1000;
const int WIDTH = 1000;
const int DEPTH = 3;

void parse_input(const string &filename) {
	ifstream fin(filename);
	string line, type, obj_filename;
	Material mtrl;
    Transformation trans;

	while (getline(fin, line)) {
		if (line.empty())
			continue;
		stringstream ss;
		ss << line;
		ss >> type;

        bool valid_type = true;
		if (type == "cam") {
			double ex, ey, ez, llx, lly, llz, lrx, lry, lrz, ulx, uly, ulz, urx, ury, urz;
			ss >> ex >> ey >> ez >> llx >> lly >> llz >> lrx >> lry >> lrz >> ulx >> uly >> ulz >> urx >> ury >> urz;
			Camera::instance()->init(ex, ey, ez, llx, lly, llz, lrx, lry, lrz, ulx, uly, ulz, urx, ury, urz);
		} else if (type == "sph") {
			double x, y, z, rad;
			ss >> x >> y >> z >> rad;
			GeoObject *sph = new Ellipsoid(x, y, z, rad, mtrl, trans);
			world_objects.push_back(sph);
		} else if (type == "tri") {
			double ax, ay, az, bx, by, bz, cx, cy, cz;
			ss >> ax >> ay >> az >> bx >> by >> bz >> cx >> cy >> cz;
			GeoObject *tri = new Triangle(ax, ay, az, bx, by, bz, cx, cy, cz, mtrl);
			world_objects.push_back(tri);
		} else if (type == "obj") {
			// read .obj file
			ss >> obj_filename;
			load_mesh(obj_filename, mtrl);
		} else if (type == "ltp") {
			double px, py, pz, r, g, b;
			int falloff;
			ss >> px >> py >> pz >> r >> g >> b >> falloff;
			Light *pl = new PointLight(px, py, pz, r, g, b, falloff);
            world_lights.push_back(pl);
		} else if (type == "ltd") {
			double dx, dy, dz, r, g, b;
			ss >> dx >> dy >> dz >> r >> g >> b;
			Light *dl = new DirectionalLight(dx, dy, dz, r, g, b);
            world_lights.push_back(dl);
		} else if (type == "lta") {
			double r, g, b;
			ss >> r >> g >> b;
			Light *al = new AmbientLight(r, g, b);
            world_lights.push_back(al);
        } else if (type == "lts") {
        	double px, py, pz, dx, dy, dz, r, g, b, beam, falloff;
        	ss >> px >> py >> pz >> dx >> dy >> dz >> r >> g >> b >> beam >> falloff;
        	Light *sl = new SpotLight(px, py, pz, dx, dy, dz, r, g, b, beam, falloff);
        	world_lights.push_back(sl);
		} else if (type == "mat") {
			double kar, kag, kab, kdr, kdg, kdb, ksr, ksg, ksb, ksp, krr, krg, krb;
			ss >> kar >> kag >> kab >> kdr >> kdg >> kdb >> ksr >> ksg >> ksb >> ksp >> krr >> krg >> krb;
			mtrl = Material(kar, kag, kab, kdr, kdg, kdb, ksr, ksg, ksb, ksp, krr, krg, krb);
		} else if (type.length() == 3 && type[0] == 'x' && type[1] == 'f') {
            double x, y, z;
			if (type[2] == 't') {
                // translation
                ss >> x >> y >> z;
                Translation trans_cur(x, y, z);
                trans.chain(trans_cur, 0);
			} else if (type[2] == 'r') {
                // rotation
                ss >> x >> y >> z;
                Rotation trans_cur(x, y, z);
                trans.chain(trans_cur, 1);
			} else if (type[2] == 's') {
                // scale
                ss >> x >> y >> z;
                Scaling trans_cur(x, y, z);
                trans.chain(trans_cur, 2);
			} else if (type[2] == 'z') {
				trans.reset();
			}
		} else if (type == "#") {
            return;
        } else {
			cerr << "Unknown specification type: " << type << endl;
            valid_type = false;
		}
        if (valid_type) {
            string extra;
            getline(ss, extra);
            if (!extra.empty()) {
                cerr << "Extra parameters:" << extra << endl;
            }
        }
	}
}

void load_mesh(const string &obj_filename, const Material &mtrl) {
    ifstream fin(obj_filename);
    vector<Vector> vertices;
    string line, type;
	while (getline(fin, line)) {
		if (line.empty())
			continue;
		stringstream ss;
		ss << line;
		ss >> type;

		if (type[0] == 'v') {
			double x, y, z;
			ss >> x >> y >> z;
			vertices.emplace_back(x, y, z);
		} else if (type[0] == 'f') {
			int x, y, z;
			ss >> x >> y >> z;
			GeoObject *tri = new Triangle(vertices[x - 1], vertices[y - 1], vertices[z - 1], mtrl);
			world_objects.push_back(tri);
		} else if (type[0] == '#') {
			continue;
		} else {
			cerr << "Unknown type encountered in .obj file: " << obj_filename << endl;
		}
	}
}

void write_file(string &output_filename, map<pii, Vector> &image) {
	pngwriter png(HEIGHT, WIDTH, 0, output_filename.c_str());

	for (int i = 1; i <= HEIGHT; i++) {
		for (int j = 1; j <= WIDTH; j++) {
			Vector v = image[pii(i, j)];
			png.plot(i, j, v.x, v.y, v.z);
		}
	}

	png.close();
}

Vector trace(const Vector &ray_pos, const Vector &ray_dir, int depth) {
    double min_t = INF;
    GeoObject *intersect_obj = nullptr;
    Vector intersect_norm;
    for (auto &it : world_objects) {
        if (it->intersect(ray_pos, ray_dir, &min_t, &intersect_norm)) {
            intersect_obj = it;
        }
    }

	Vector color;
    if (intersect_obj == nullptr)
    	return color;

    Vector hit_pos = ray_pos + (ray_dir * min_t);
    // Get intensity from all lights at intersection point.
    for (auto &light_it : world_lights) {
        // Check if light is blocked by any object
        Vector blocked_norm;
        bool has_intersection = false;
        min_t = INF;
        for (auto &obj_it : world_objects) {
            Vector ray_to_light = light_it->direction(hit_pos);
            has_intersection |= obj_it->intersect(hit_pos + ray_to_light * EPS, ray_to_light, &min_t, &blocked_norm);
        }
        Vector hit_vec = ray_dir * min_t;
        // Only update color if no intersection was encountered, or
        // is but is further away than the light
        if (!has_intersection || hit_vec.norm() - light_it->get_dist(hit_pos) > EPS) {
            // (light, view, hit point)
            color = color + intersect_obj->get_color(*light_it, -ray_dir, hit_pos, intersect_norm);
        }
    }

    if (depth > 1) {
        Vector reflected = ray_dir - 2.0 * intersect_norm.dot(ray_dir) * intersect_norm;
        reflected.normalize();
        // Add vector by epsilon in direction to ensure no intersection with same object
        Vector reflected_color = trace(hit_pos + reflected * EPS, reflected, depth - 1) * intersect_obj->mtrl.reflective;;
        color = color + reflected_color;
    }
    return color.clip();
}

void get_pixels(map<pii, Vector> *image) {
	Camera *cam = Camera::instance();
	for (int i = 1; i <= HEIGHT; i++) {
		for (int j = 1; j <= WIDTH; j++) {
			double u = ((HEIGHT - i + 1) - 0.5) / HEIGHT;
			double v = ((WIDTH - j + 1) - 0.5) / WIDTH;
			Vector ray_dir = \
                u * (v * cam->ll + (1.0 - v) * cam->ul) +
				(1.0 - u) * (v * cam->lr + (1.0 - v) * cam->ur) -
                cam->loc;
			ray_dir.normalize();

            Vector color = trace(cam->loc + ray_dir * EPS, ray_dir, DEPTH);
            image->insert(pair<pii, Vector>(pii(i, j), color));
		}
	}
}

void LOG(const string &msg) {
	cerr << msg << endl;
}

int main(int argc, char *argv[]) {
	string input_filename = "raytracer.in";
	string output_filename = "raytracer.png";
	if (argc >= 2)
		input_filename = string(argv[1]);
	if (argc >= 3)
		output_filename = string(argv[2]);

	map<pii, Vector> image;
	parse_input(input_filename);
	LOG("Done parsing input.");
	get_pixels(&image);
	LOG("Done generating image.");
	write_file(output_filename, image);
	LOG("Written image to file.");

	for (int i = 0; i < (int)world_objects.size(); i++) {
		delete world_objects[i];
		world_objects[i] = nullptr;
	}
	return 0;
}
