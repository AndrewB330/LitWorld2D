#pragma once
// We are using glm library, but we also need some specific 2d functions that are not available in glm
#include <glm/glm.hpp>

using namespace glm;

inline vec2 rotate2d(vec2 v, float angle) {
    float cs = cosf(angle);
    float sn = sinf(angle);
    return vec2(v.x * cs - v.y * sn, v.x * sn + v.y * cs);
}

inline vec2 tangent2d(vec2 v) {
    return vec2(-v.y, v.x);
}