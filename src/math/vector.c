#include "vector.h"

void vec_multNum3f(const Vector3f *const v, const float n, Vector3f *const res) {
    res->x = v->x * n;
    res->y = v->y * n;
    res->z = v->z * n;
}
