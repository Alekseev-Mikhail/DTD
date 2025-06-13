#ifndef CAMERA_H
#define CAMERA_H
#include <stdbool.h>

#include "math/matrix.h"
#include "math/vector.h"

typedef struct {
    Vector3f *position;
    Vector3f *rotation;
    float fov, near, far, aspect;
    Matrix4f *vp;
    Matrix4f *perspective;
    Matrix4f *view;
    Matrix4f *_posMat;
    Matrix4f *_rotMat;
    bool _isPerMatUpdateNeeded;
    bool _isPosMatUpdateNeeded;
    bool _isRotMatUpdateNeeded;
} Camera;

Camera *cam_allocate();

void cam_dispose(Camera *c);

void cam_move(Camera *c, float x, float y, float z);

void cam_rotate(Camera *c, float x, float y, float z);

void cam_setPrefs(Camera *c, float fov, float near, float far);

void cam_updateMatrices(Camera *c);

#endif //CAMERA_H
