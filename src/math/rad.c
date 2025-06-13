#include "rad.h"

#include <math.h>

float toRad(const float degrees) {
    static const float oneDegInRads = (float) M_PI / 180.0f;
    return degrees * oneDegInRads;
}

float toDeg(const float radians) {
    static const float oneRadInDegs = 180.0f / (float) M_PI;
    return radians * oneRadInDegs;
}
