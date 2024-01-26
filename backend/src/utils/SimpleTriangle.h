//
// Created by nikla on 24.10.2023.
//

#ifndef ROYAL_TRACER_SIMPLETRIANGLE_H
#define ROYAL_TRACER_SIMPLETRIANGLE_H

#include <array>

using float3 = std::array<float, 3>;

struct SimpleTriangle {
    const std::size_t MAX_NUM_VERTICES = 3;
    int obj_id;
    int materialID;
    float3 v1;
    float3 v2;
    float3 v3;
    // normal vector for the whole triangle
    float3 vn;
    // texture coordinates
    float3 vt1;
    float3 vt2;
    float3 vt3;
};

#endif //ROYAL_TRACER_SIMPLETRIANGLE_H
