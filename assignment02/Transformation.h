#ifndef __TRANSFORMATION_H
#define __TRANSFORMATION_H

#include "Matrix.h"

class Transformation {
 public:
    Transformation() {
        reset();
    }
    Transformation(Matrix mat_) {
        mat = mat_;
    }
    Transformation(Matrix mat_, Matrix dir_mat_, Matrix norm_mat_) {
        mat = mat_;
        dir_mat = dir_mat_;
        norm_mat = norm_mat_;
    }
    ~Transformation() = default;
    Vector apply(Vector vec) {
        Vector ret = Vector(
            mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z + mat[0][3],
            mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z + mat[1][3],
            mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z + mat[2][3]
        );
        return ret;
    }
    Vector apply_dir(Vector vec) {
        Vector ret = Vector(
            dir_mat[0][0] * vec.x + dir_mat[0][1] * vec.y + dir_mat[0][2] * vec.z + dir_mat[0][3],
            dir_mat[1][0] * vec.x + dir_mat[1][1] * vec.y + dir_mat[1][2] * vec.z + dir_mat[1][3],
            dir_mat[2][0] * vec.x + dir_mat[2][1] * vec.y + dir_mat[2][2] * vec.z + dir_mat[2][3]
        );
        return ret;
    }
    Vector apply_norm(Vector vec) {
        Vector ret = Vector(
            norm_mat[0][0] * vec.x + norm_mat[0][1] * vec.y + norm_mat[0][2] * vec.z + norm_mat[0][3],
            norm_mat[1][0] * vec.x + norm_mat[1][1] * vec.y + norm_mat[1][2] * vec.z + norm_mat[1][3],
            norm_mat[2][0] * vec.x + norm_mat[2][1] * vec.y + norm_mat[2][2] * vec.z + norm_mat[2][3]
        );
        return ret;
    }
    void chain(const Transformation &other, int type) {
        mat = mat.multiply(other.mat);
        // 0 = translation, 1 = rotation, 2 = scaling
        if (type != 0) dir_mat = dir_mat.multiply(other.mat);
        if (type == 1) norm_mat = norm_mat.multiply(other.mat);
        else if (type == 2) norm_mat = norm_mat.multiply(other.mat.inverse());
    }
    void pre_chain(const Transformation &other, int type) {
        mat = other.mat.multiply(mat);
        // 0 = translation, 1 = rotation, 2 = scaling
        if (type != 0) dir_mat = other.mat.multiply(dir_mat);
        if (type == 1) norm_mat = other.mat.multiply(norm_mat);
        else if (type == 2) norm_mat = other.mat.inverse().multiply(norm_mat);
    }
    void reset() {
        mat = Matrix::identity(4);
        dir_mat = Matrix::identity(4);
        norm_mat = Matrix::identity(4);
    }
    Transformation inverse() {
        return Transformation(mat.inverse(), dir_mat.inverse(), norm_mat.inverse());
    }
    Transformation transpose() {
        return Transformation(mat.transpose(), dir_mat.transpose(), norm_mat.inverse());
    }
    void print_dir_mat() {
        for (int i = 0; i < dir_mat.sz; i++) {
            for (int j = 0; j < dir_mat.sz; j++) {
                cout << dir_mat[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }
    void print_norm_mat() {
        for (int i = 0; i < norm_mat.sz; i++) {
            for (int j = 0; j < norm_mat.sz; j++) {
                cout << norm_mat[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }
    friend ostream& operator<< (ostream &out, Transformation &trans) {
        for (int i = 0; i < trans.mat.sz; i++) {
            for (int j = 0; j < trans.mat.sz; j++) {
                out << trans.mat[i][j] << " ";
            }
            out << endl;
        }
        return out;
    }
    Matrix mat;
    Matrix dir_mat;
    Matrix norm_mat;
};

class Rotation : public Transformation {
 public:
    Rotation() : Transformation() {}
    Rotation(double sx_, double sy_, double sz_) {
        Vector dir(sx_, sy_, sz_);
        double angle = dir.norm();
        dir.normalize();

        double cosine = cos(angle);
        double sine = sin(angle);

        mat[0][0] = cosine + sqr(dir.x) * (1.0 - cosine);
        mat[0][1] = dir.x * dir.y * (1.0 - cosine) - dir.z * sine;
        mat[0][2] = dir.x * dir.z * (1.0 - cosine) + dir.y * sine;

        mat[1][0] = dir.y * dir.x * (1.0 - cosine) + dir.z * sine;
        mat[1][1] = cosine + sqr(dir.y) * (1.0 - cosine);
        mat[1][2] = dir.y * dir.z * (1.0 - cosine) - dir.x * sine;

        mat[2][0] = dir.z * dir.x * (1.0 - cosine) - dir.y * sine;
        mat[2][1] = dir.z * dir.y * (1.0 - cosine) + dir.x * sine;
        mat[2][2] = cosine + sqr(dir.z) * (1.0 - cosine);
    }
    ~Rotation() = default;
};

class Translation : public Transformation {
 public:
    Translation() : Transformation() {}
    Translation(double tx_, double ty_, double tz_) {
        Transformation();
        mat.vals[0][3] = tx_;
        mat.vals[1][3] = ty_;
        mat.vals[2][3] = tz_;
    }
    ~Translation() = default;
};

class Scaling : public Transformation {
 public:
    Scaling() : Transformation() {}
    Scaling(double sx_, double sy_, double sz_) {
        Transformation();
        mat.vals[0][0] = sx_;
        mat.vals[1][1] = sy_;
        mat.vals[2][2] = sz_;
    }
    ~Scaling() = default;
};

#endif
