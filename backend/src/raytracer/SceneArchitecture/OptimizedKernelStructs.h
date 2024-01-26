//
// Created by m on 24.11.2023.
//

#ifndef ROYAL_TRACER_OPTIMIZEDKERNELSTRUCTS_H
#define ROYAL_TRACER_OPTIMIZEDKERNELSTRUCTS_H

#include <CL/cl.h>

struct TriBASE{
    cl_float3 v1,v2,v3;
    cl_float3 centroid, vn;
};

struct TriEX{
    cl_int materialID;
    cl_int objectID;
    cl_float2 uv1, uv2, uv3;

    // paramaters that can be precalulated for each triangle for map coordinates
    cl_float3 vv0, vv1;
    cl_float d00, d01, d11, invDenom;
};


#endif //ROYAL_TRACER_OPTIMIZEDKERNELSTRUCTS_H
