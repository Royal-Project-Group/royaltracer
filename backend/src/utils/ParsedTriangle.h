//
// Created by nikla on 18.10.2023.
//

#ifndef ROYAL_TRACER_PARSEDTRIANGLE_H
#define ROYAL_TRACER_PARSEDTRIANGLE_H

#include <array>

using float3 = std::array<float, 3>;

struct ParsedTriangle {
    const size_t MAX_NUM_VERTICES = 3;
    int obj_id;
    int materialID;
    float3 v1;
    float3 v2;
    float3 v3;
    // normal vectors
    float3 vn1;
    float3 vn2;
    float3 vn3;
    // texture coordinates
    float3 vt1;
    float3 vt2;
    float3 vt3;
};

#endif //ROYAL_TRACER_PARSEDTRIANGLE_H
