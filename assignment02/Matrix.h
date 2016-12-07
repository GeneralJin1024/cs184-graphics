#ifndef __MATRIX_H
#define __MATRIX_H

#include <cassert>

#include "Vector.h"

class Matrix {
 public:
 	Matrix() = default;
 	Matrix(int n) {
 		vals.resize(n, vector<double>(n, 0.0));
        sz = n;
 	}
    Matrix(const Matrix &other) {
        vals = other.vals;
        sz = other.sz;
    }
    static Matrix identity(int n) {
        Matrix mat(n);
        for (int i = 0; i < n; i++) {
            mat.vals[i][i] = 1.0;
        }
        return mat;
    }
    Matrix multiply(const Matrix &other) const {
        Matrix mat(4);
        assert(mat.sz == other.sz);
        for (int i = 0; i < sz; i++) {
            for (int j = 0; j < sz; j++) {
                for (int k = 0; k < sz; k++) {
                    mat.vals[i][j] += vals[i][k] * other.vals[k][j];
                }
            }
        }
        return mat;
    }
    Matrix inverse() const {
        auto vals_copy = vals;
        Matrix inv = Matrix::identity(sz);

        // iterate through columns
        for (int i = 0; i < sz; i++) {
            // iterate through rows
            int maxj = i;
            for (int j = i + 1; j < sz; j++) {
                if (fabs(vals_copy[j][i]) > fabs(vals_copy[maxj][i]))
                    maxj = j;
            }

            swap(vals_copy[i], vals_copy[maxj]);
            swap(inv[i], inv[maxj]);

            double val = vals_copy[i][i];
            for (int j = 0; j < sz; j++) {
                vals_copy[i][j] /= val;
                inv[i][j] /= val;
            }

            for (int j = i + 1; j < sz; j++) {
                double coeff = vals_copy[j][i];
                for (int k = 0; k < sz; k++) {
                    vals_copy[j][k] -= coeff * vals_copy[i][k];
                    inv[j][k] -= coeff * inv[i][k];
                }
            }
        }

        // iterate through columns
        for (int i = sz - 1; i >= 0; i--) {
            double coeff = vals_copy[i][i];
            for (int j = 0; j < sz; j++) {
                inv[i][j] /= coeff;
            }
            vals_copy[i][i] = 1.0;

            // iterate through rows
            for (int j = i - 1; j >= 0; j--) {
                coeff = vals_copy[j][i];
                vals_copy[j][i] = 0.0;
                for (int k = 0; k < sz; k++) {
                    inv[j][k] -= coeff * inv[i][k];
                }
            }
        }

        return inv;
    }
    Matrix transpose() {
        Matrix ret(*this);
        for (int i = 0; i < sz; i++) {
            for (int j = 0; j < i; j++) {
                swap(ret.vals[i][j], ret.vals[j][i]);
            }
        }
        return ret;
    }
    vector<double>& operator[](int idx) {
        return vals[idx];
    }
 	vector<vector<double>> vals;
    int sz;
};

#endif
