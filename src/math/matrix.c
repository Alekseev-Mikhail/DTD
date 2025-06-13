#include "matrix.h"

#include <math.h>

void mat_multVec4f(const Matrix4f *const m, const Vector4f *const v, Vector4f *const res) {
    res->x = m->t[0][0] * v->x + m->t[1][0] * v->y + m->t[2][0] * v->z + m->t[3][0] * v->w;
    res->y = m->t[0][1] * v->x + m->t[1][1] * v->y + m->t[2][1] * v->z + m->t[3][1] * v->w;
    res->z = m->t[0][2] * v->x + m->t[1][2] * v->y + m->t[2][2] * v->z + m->t[3][2] * v->w;
    res->w = m->t[0][3] * v->x + m->t[1][3] * v->y + m->t[2][3] * v->z + m->t[3][3] * v->w;
}

void mat_multMat4f(const Matrix4f *const l, const Matrix4f *const r, Matrix4f *const res) {
    for (int column = 0; column < 4; column++) {
        for (int row = 0; row < 4; row++) {
            float *const curr = res->t[column] + row;
            for (int i = 0; i < 4; i++) {
                *curr += l->t[i][row] * r->t[column][i];
            }
        }
    }
}

void mat_identity(Matrix4f *const res) {
    res->t[0][0] = 1.0f;
    res->t[1][1] = 1.0f;
    res->t[2][2] = 1.0f;
    res->t[3][3] = 1.0f;
}

void mat_translation(Matrix4f *const res, const Vector3f *const pos) {
    mat_identity(res);
    res->t[3][0] = pos->x;
    res->t[3][1] = pos->y;
    res->t[3][2] = pos->z;
}

void mat_rotation(Matrix4f *const res, const Vector3f *const rot) {
    const float xSin = sinf(rot->x);
    const float xCos = cosf(rot->x);
    const float ySin = sinf(rot->y);
    const float yCos = cosf(rot->y);
    const float zSin = sinf(rot->z);
    const float zCos = cosf(rot->z);
    res->t[0][0] = yCos * zCos;
    res->t[0][1] = xSin * ySin * zCos - xCos * zSin;
    res->t[0][2] = xCos * ySin * zCos + xSin * zSin;
    res->t[1][0] = yCos * zSin;
    res->t[1][1] = xSin * ySin * zSin + xCos * zCos;
    res->t[1][2] = xCos * ySin * zSin - xSin * zCos;
    res->t[2][0] = -ySin;
    res->t[2][1] = xSin * yCos;
    res->t[2][2] = xCos * yCos;
    res->t[3][3] = 1.0f;
}

void mat_perspective(Matrix4f *const res, const float aspect, const float fov, const float near,
                         const float far) {
    const float fovFactor = 1.0f / tanf(fov / 2.0f);
    const float zFactor = far / (far - near);
    res->t[0][0] = aspect * fovFactor;
    res->t[1][1] = fovFactor;
    res->t[2][2] = zFactor;
    res->t[2][3] = 1.0f;
    res->t[3][2] = -zFactor * near;
}
