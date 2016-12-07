#ifndef __BEZIER_PATCH_H
#define __BEZIER_PATCH_H

#include "include.h"
#include "TransformableObject.h"

Matrix4f right_mat;
Matrix4f left_mat;

class BezierPatch {
 public:
    BezierPatch(int idx) {}
    BezierPatch(vector<vector<shared_ptr<Vector3f>>>&& points_) :
        points(move(points_)) {}
    ~BezierPatch() = default;

    void output_points() {
        for (int i = 0; i < (int)points.size(); i++) {
            for (int j = 0; j < (int)points[i].size(); j++) {
                cout << "(" << (*points[i][j])[0] << ", " << (*points[i][j])[1] << ", " << (*points[i][j])[2] << ") ";
            }
            cout << endl;
        }
    }

    shared_ptr<Vector3f> computeUVPoint(T u, T v) {
        RowVector4f u_vec(pow(u, 3), pow(u, 2), u, 1);
        RowVector4f v_vec(pow(v, 3), pow(v, 2), v, 1);
        Vector4f v_vec_trans = v_vec.transpose();

        Matrix4f x_coords = getDimensionMatrix(0);
        Matrix4f y_coords = getDimensionMatrix(1);
        Matrix4f z_coords = getDimensionMatrix(2);

        T x = u_vec * left_mat * x_coords * right_mat * v_vec_trans;
        T y = u_vec * left_mat * y_coords * right_mat * v_vec_trans;
        T z = u_vec * left_mat * z_coords * right_mat * v_vec_trans;

        return shared_ptr<Vector3f>(new Vector3f(x, y, z));
    }
    shared_ptr<Vector3f> computeUVNormal(T u, T v) {
        // Use cross-product to compute normal
        auto a = computeUVPoint(u + eps, v);
        auto b = computeUVPoint(u, v + eps);
        auto pt = computeUVPoint(u, v);
        auto c = (*a - *pt);
        auto d = (*b - *pt);
        shared_ptr<Vector3f> norm(new Vector3f(c.cross(d).normalized()));
        if (*norm == Vector3f(0.0f, 0.0f, 0.0f)) {
            return computeUVNormal(u + eps, v + eps);
        }
        return norm;
    }
    Matrix4f getDimensionMatrix(int dim) {
        if (dimension_matrix_cache.count(dim)) {
            return dimension_matrix_cache[dim];
        }
        Matrix4f &mat = dimension_matrix_cache[dim];
        mat << (*points[0][0])[dim], (*points[0][1])[dim], (*points[0][2])[dim], (*points[0][3])[dim],
               (*points[1][0])[dim], (*points[1][1])[dim], (*points[1][2])[dim], (*points[1][3])[dim],
               (*points[2][0])[dim], (*points[2][1])[dim], (*points[2][2])[dim], (*points[2][3])[dim],
               (*points[3][0])[dim], (*points[3][1])[dim], (*points[3][2])[dim], (*points[3][3])[dim];
        return mat;
    }

    void subdivide_triangle(const vector<shared_ptr<Vector3f>> &xy_pts,
                            const vector<shared_ptr<Vector2f>> &uv_pts,
                            const T &error) {
        if ((int)xy_pts.size() != 3 || (int)uv_pts.size() != 3) {
            cerr << "Subdividing non-triangle!" << endl;
            assert(false);
            return;
        }
        int count = 0;
        bool e1, e2, e3; // flag if edges need to be divided
        e1 = e2 = e3 = false;

        Vector3f midxy_first = ((*xy_pts[0]) + (*xy_pts[1])) / 2.0;
        Vector3f midxy_second = ((*xy_pts[1]) + (*xy_pts[2])) / 2.0;
        Vector3f midxy_third = ((*xy_pts[2]) + (*xy_pts[0])) / 2.0;
        auto miduv_first = computeUVPoint(((*uv_pts[0])[0] + (*uv_pts[1])[0]) / 2.0, ((*uv_pts[0])[1] + (*uv_pts[1])[1]) / 2.0);
        auto miduv_second = computeUVPoint(((*uv_pts[1])[0] + (*uv_pts[2])[0]) / 2.0, ((*uv_pts[1])[1] + (*uv_pts[2])[1]) / 2.0);
        auto miduv_third = computeUVPoint(((*uv_pts[2])[0] + (*uv_pts[0])[0]) / 2.0, ((*uv_pts[2])[1] + (*uv_pts[0])[1]) / 2.0);

        if ((midxy_first - (*miduv_first)).norm() - error >= eps) {
            ++count;
            e1 = true;
        }

        if ((midxy_second - (*miduv_second)).norm() - error >= eps) {
            ++count;
            e2 = true;
        }

        if ((midxy_third - (*miduv_third)).norm() - error >= eps) {
            ++count;
            e3 = true;
        }

        if (count == 0) {
            vector<shared_ptr<Vector3f>> cur_triangle, cur_triangle_norm;
            cur_triangle.push_back(xy_pts[0]);
            cur_triangle.push_back(xy_pts[1]);
            cur_triangle.push_back(xy_pts[2]);
            cur_triangle_norm.push_back(move(computeUVNormal((*uv_pts[0])[0], (*uv_pts[0])[1])));
            cur_triangle_norm.push_back(move(computeUVNormal((*uv_pts[1])[0], (*uv_pts[1])[1])));
            cur_triangle_norm.push_back(move(computeUVNormal((*uv_pts[2])[0], (*uv_pts[2])[1])));
            adaptive_points.push_back(move(cur_triangle));
            adaptive_normals.push_back(move(cur_triangle_norm));
            return;
        }

        vector<shared_ptr<Vector3f>> tri_xy, mid_xy;
        vector<shared_ptr<Vector2f>> tri_uv, mid_uv;
        if (count == 1) {
            vector<pair<int, int>> aux;
            aux.push_back(make_pair(e1, 0));
            aux.push_back(make_pair(e2, 1));
            aux.push_back(make_pair(e3, 2));
            sort(aux.begin(), aux.end());
            assert(aux[2].first);
            int split_idx = (aux[2].second + 2) % 3; // index of vertex
            int i = (split_idx + 1) % 3, j = (split_idx + 2) % 3;
            shared_ptr<Vector2f> split_uv(new Vector2f(((*uv_pts[i]) + (*uv_pts[j])) / 2.0));
            shared_ptr<Vector3f> split_xy = computeUVPoint((*split_uv)[0], (*split_uv)[1]);

            tri_xy.push_back(xy_pts[split_idx]);
            tri_xy.push_back(xy_pts[i]);
            tri_xy.push_back(split_xy);
            tri_uv.push_back(uv_pts[split_idx]);
            tri_uv.push_back(uv_pts[i]);
            tri_uv.push_back(split_uv);
            subdivide_triangle(tri_xy, tri_uv, error);

            tri_xy.clear();
            tri_uv.clear();
            tri_xy.push_back(split_xy);
            tri_xy.push_back(xy_pts[j]);
            tri_xy.push_back(xy_pts[split_idx]);
            tri_uv.push_back(split_uv);
            tri_uv.push_back(uv_pts[j]);
            tri_uv.push_back(uv_pts[split_idx]);
            subdivide_triangle(tri_xy, tri_uv, error);
            return;
        }
        if (count == 2) {
            vector<pair<int, int>> aux;
            aux.push_back(make_pair(e1, 0));
            aux.push_back(make_pair(e2, 1));
            aux.push_back(make_pair(e3, 2));
            sort(aux.begin(), aux.end());
            assert(!aux[0].first);
            if (aux[1].second == 2 && aux[2].second == 0)
                swap(aux[1], aux[2]);
            int a = aux[1].second, b = aux[2].second, c = aux[0].second;

            shared_ptr<Vector2f> split_uv1(
                new Vector2f(((*uv_pts[a])[0] + (*uv_pts[b])[0]) / 2.0,
                             ((*uv_pts[a])[1] + (*uv_pts[b])[1]) / 2.0));
            shared_ptr<Vector3f> split_xy1 =
                computeUVPoint((*split_uv1)[0], (*split_uv1)[1]);

            shared_ptr<Vector2f> split_uv2(
                new Vector2f(((*uv_pts[b])[0] + (*uv_pts[c])[0]) / 2.0,
                             ((*uv_pts[b])[1] + (*uv_pts[c])[1]) / 2.0));
            shared_ptr<Vector3f> split_xy2 =
                computeUVPoint((*split_uv2)[0], (*split_uv2)[1]);

            tri_xy.push_back(xy_pts[a]);
            tri_xy.push_back(split_xy1);
            tri_xy.push_back(xy_pts[c]);
            tri_uv.push_back(uv_pts[a]);
            tri_uv.push_back(split_uv1);
            tri_uv.push_back(uv_pts[c]);
            subdivide_triangle(tri_xy, tri_uv, error);

            tri_xy.clear(); tri_uv.clear();
            tri_xy.push_back(xy_pts[b]);
            tri_xy.push_back(split_xy2);
            tri_xy.push_back(split_xy1);
            tri_uv.push_back(uv_pts[b]);
            tri_uv.push_back(split_uv2);
            tri_uv.push_back(split_uv1);
            subdivide_triangle(tri_xy, tri_uv, error);

            tri_xy.clear(); tri_uv.clear();
            tri_xy.push_back(split_xy2);
            tri_xy.push_back(xy_pts[c]);
            tri_xy.push_back(split_xy1);
            tri_uv.push_back(split_uv2);
            tri_uv.push_back(uv_pts[c]);
            tri_uv.push_back(split_uv1);
            subdivide_triangle(tri_xy, tri_uv, error);
            return;
        }
        if (count == 3) {
            for (int i = 0; i < 3; i++) {
                shared_ptr<Vector2f> split_uv(
                    new Vector2f(((*uv_pts[i]) + (*uv_pts[(i+1)%3])) / 2.0));
                shared_ptr<Vector3f> split_xy =
                    computeUVPoint((*split_uv)[0], (*split_uv)[1]);
                mid_xy.push_back(move(split_xy));
                mid_uv.push_back(move(split_uv));
            }
            for (int i = 0; i < 3; i++) {
                tri_xy.push_back(xy_pts[i]);
                tri_xy.push_back(mid_xy[i%3]);
                tri_xy.push_back(mid_xy[(i+2)%3]);
                tri_uv.push_back(uv_pts[i]);
                tri_uv.push_back(mid_uv[i%3]);
                tri_uv.push_back(mid_uv[(i+2)%3]);
                subdivide_triangle(tri_xy, tri_uv, error);

                tri_xy.clear(); tri_uv.clear();
            }
            tri_xy.push_back(mid_xy[0]);
            tri_xy.push_back(mid_xy[1]);
            tri_xy.push_back(mid_xy[2]);
            tri_uv.push_back(mid_uv[0]);
            tri_uv.push_back(mid_uv[1]);
            tri_uv.push_back(mid_uv[2]);
            subdivide_triangle(tri_xy, tri_uv, error);
            return;
        }
    }

    void uniform_subdivision(T step_size) {
        uniform_points.clear();
        uniform_normals.clear();
        int numdiv = (1.0 + eps) / step_size;
        for (int i = 0; i <= numdiv; i++) {
          T u = i * step_size;
          vector<shared_ptr<Vector3f> > row_pts, row_norms;
          for (int j = 0; j <= numdiv; j++) {
            T v = j * step_size;
            auto point = computeUVPoint(u, v);
            auto normal = computeUVNormal(u, v);

            row_pts.push_back(move(point));
            row_norms.push_back(move(normal));
          }
          uniform_points.push_back(move(row_pts));
          uniform_normals.push_back(move(row_norms));
        }
    }
    void adaptive_tessellation(T error) {
        T step_size_i = (1.0 / (int)points.size());
        for (int i = 0; i < (int)points.size(); i++) {
            T step_size_j = (1.0 / (int)points[i].size());
            for (int j = 0; j < (int)points[i].size(); j++) {
                shared_ptr<Vector2f> pt0(new Vector2f(i * step_size_i, j * step_size_j));
                shared_ptr<Vector2f> pt1(new Vector2f((i + 1) * step_size_i, j * step_size_j));
                shared_ptr<Vector2f> pt2(new Vector2f(i * step_size_i, (j + 1) * step_size_j));
                shared_ptr<Vector2f> pt3(new Vector2f((i + 1) * step_size_i, (j + 1) * step_size_j));

                shared_ptr<Vector3f> p0(computeUVPoint((*pt0)[0], (*pt0)[1]));
                shared_ptr<Vector3f> p1(computeUVPoint((*pt1)[0], (*pt1)[1]));
                shared_ptr<Vector3f> p2(computeUVPoint((*pt2)[0], (*pt2)[1]));
                shared_ptr<Vector3f> p3(computeUVPoint((*pt3)[0], (*pt3)[1]));
                // upper left triangle (p0, p2, p3)
                vector<shared_ptr<Vector3f>> ul_xy_tri;
                vector<shared_ptr<Vector2f>> ul_uv_tri;
                ul_xy_tri.push_back(p0);
                ul_xy_tri.push_back(p2);
                ul_xy_tri.push_back(p3);
                ul_uv_tri.push_back(pt0);
                ul_uv_tri.push_back(pt2);
                ul_uv_tri.push_back(pt3);
                subdivide_triangle(ul_xy_tri, ul_uv_tri, error);

                // bottom right triangle (p0, p1, p3)
                vector<shared_ptr<Vector3f>> br_xy_tri;
                vector<shared_ptr<Vector2f>> br_uv_tri;
                br_xy_tri.push_back(move(p0));
                br_xy_tri.push_back(move(p1));
                br_xy_tri.push_back(move(p3));
                br_uv_tri.push_back(move(pt0));
                br_uv_tri.push_back(move(pt1));
                br_uv_tri.push_back(move(pt3));
                subdivide_triangle(br_xy_tri, br_uv_tri, error);
            }
        }
    }

    // stores uniformly spaced points
    vector<vector<shared_ptr<Vector3f>>> uniform_points;
    vector<vector<shared_ptr<Vector3f>>> uniform_normals;
    // stores triangles
    vector<vector<shared_ptr<Vector3f>>> adaptive_points;
    vector<vector<shared_ptr<Vector3f>>> adaptive_normals;
    vector<vector<shared_ptr<Vector3f>>> points;

    map<int, Matrix4f> dimension_matrix_cache;
};

class BezierObject : public TransformableObject {
 public:
    BezierObject(int idx) : TransformableObject(idx) {}
    ~BezierObject() = default;

    vector<unique_ptr<BezierPatch>> patches;
};

#endif
