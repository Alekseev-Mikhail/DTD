#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

/**
 * Represents a 4x4 matrix of float values used for 3D transformations.
 * The matrix is stored in column-major order, where t[column][row].
 */
typedef struct {
    float t[4][4];
} Matrix4f;

void mat_multVec4f(const Matrix4f *m, const Vector4f *v, Vector4f *res);

void mat_multMat4f(const Matrix4f *l, const Matrix4f *r, Matrix4f *res);

void mat_identity(Matrix4f *res);

void mat_translation(Matrix4f *res, const Vector3f *pos);

void mat_rotation(Matrix4f *res, const Vector3f *rot);

void mat_perspective(Matrix4f *res, float aspect, float fov, float near, float far);

#endif //MATRIX_H
