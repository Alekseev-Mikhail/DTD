#include "camera.h"

#include <stdlib.h>

Camera *cam_allocate() {
    Camera *c = malloc(sizeof(Camera));
    c->fov = 0;
    c->near = 0;
    c->far = 0;
    c->aspect = 0;
    c->position = calloc(2, sizeof(Vector3f));
    c->rotation = c->position + 1;
    c->vp = calloc(5, sizeof(Matrix4f));
    c->view = c->vp + 1;
    c->perspective = c->vp + 2;
    c->_posMat = c->vp + 3;
    c->_rotMat = c->vp + 4;
    c->_isPerMatUpdateNeeded = true;
    c->_isPosMatUpdateNeeded = true;
    c->_isRotMatUpdateNeeded = true;
    return c;
}

void cam_dispose(Camera *c) {
    free(c->position);
    free(c->vp);
    free(c);
}

void cam_move(Camera *const c, const float x, const float y, const float z) {
    c->_isPosMatUpdateNeeded = true;
    c->position->x += -x;
    c->position->y += -y;
    c->position->z += -z;
}

void cam_rotate(Camera *const c, const float x, const float y, const float z) {
    c->_isRotMatUpdateNeeded = true;
    c->rotation->x += -x;
    c->rotation->y += -y;
    c->rotation->z += -z;
}

void cam_setPrefs(Camera *const c, const float fov, const float near, const float far) {
    c->_isPerMatUpdateNeeded = true;
    c->fov = fov;
    c->near = near;
    c->far = far;
}

void cam_updateMatrices(Camera *const c) {
    bool isUpdateNeeded = false;
    bool isViewMatUpdateNeeded = false;

    if (c->_isPosMatUpdateNeeded) {
        mat_translation(c->_posMat, c->position);
        c->_isPosMatUpdateNeeded = false;
        isViewMatUpdateNeeded = true;
    }
    if (c->_isRotMatUpdateNeeded) {
        mat_rotation(c->_rotMat, c->rotation);
        c->_isRotMatUpdateNeeded = false;
        isViewMatUpdateNeeded = true;
    }
    if (c->_isPerMatUpdateNeeded) {
        mat_perspective(c->perspective, c->aspect, c->fov, c->near, c->far);
        c->_isPerMatUpdateNeeded = false;
        isUpdateNeeded = true;
    }

    if (isViewMatUpdateNeeded) {
        mat_multMat4f(c->_rotMat, c->_posMat, c->view);
        isUpdateNeeded = true;
    }
    if (isUpdateNeeded) mat_multMat4f(c->perspective, c->view, c->vp);
}
