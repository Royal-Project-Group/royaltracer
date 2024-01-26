//
// Created by pfaff on 10/24/2023.
//

#ifndef ROYAL_TRACER_MATHUTILS_H
#define ROYAL_TRACER_MATHUTILS_H
#include <CL/cl2.hpp>


class MathUtils {

public:
    // multiply
    static cl_float16 multiply(cl_float16 a, cl_float16 b);

    // indentity matrix
    static cl_float16 identiyMatrix();

    // rotate x
    static cl_float16 rotateX(cl_float16 xRotation, float angle);

    // rotate y
    static cl_float16 rotateY(cl_float16 yRotation, float angle);

    // rotate z
    static cl_float16 rotateZ(cl_float16 zRotation, float angle);

    // scale
    static cl_float16 scale(cl_float16 m, float x, float y, float z);

    // translate
    static cl_float16 translate(cl_float16 m, float x, float y, float z);
};


#endif //ROYAL_TRACER_MATHUTILS_H
