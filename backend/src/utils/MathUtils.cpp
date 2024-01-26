//
// Created by pfaff on 10/24/2023.
//

#include "MathUtils.h"
#include <cmath>

//multiply two matrices
cl_float16 MathUtils::multiply(cl_float16 m1, cl_float16 m2){
    cl_float16 result;
    result.s0 = m1.s0 * m2.s0 + m1.s1 * m2.s4 + m1.s2 * m2.s8 + m1.s3 * m2.sc;
    result.s1 = m1.s0 * m2.s1 + m1.s1 * m2.s5 + m1.s2 * m2.s9 + m1.s3 * m2.sd;
    result.s2 = m1.s0 * m2.s2 + m1.s1 * m2.s6 + m1.s2 * m2.sa + m1.s3 * m2.se;
    result.s3 = m1.s0 * m2.s3 + m1.s1 * m2.s7 + m1.s2 * m2.sb + m1.s3 * m2.sf;
    result.s4 = m1.s4 * m2.s0 + m1.s5 * m2.s4 + m1.s6 * m2.s8 + m1.s7 * m2.sc;
    result.s5 = m1.s4 * m2.s1 + m1.s5 * m2.s5 + m1.s6 * m2.s9 + m1.s7 * m2.sd;
    result.s6 = m1.s4 * m2.s2 + m1.s5 * m2.s6 + m1.s6 * m2.sa + m1.s7 * m2.se;
    result.s7 = m1.s4 * m2.s3 + m1.s5 * m2.s7 + m1.s6 * m2.sb + m1.s7 * m2.sf;
    result.s8 = m1.s8 * m2.s0 + m1.s9 * m2.s4 + m1.sa * m2.s8 + m1.sb * m2.sc;
    result.s9 = m1.s8 * m2.s1 + m1.s9 * m2.s5 + m1.sa * m2.s9 + m1.sb * m2.sd;
    result.sa = m1.s8 * m2.s2 + m1.s9 * m2.s6 + m1.sa * m2.sa + m1.sb * m2.se;
    result.sb = m1.s8 * m2.s3 + m1.s9 * m2.s7 + m1.sa * m2.sb + m1.sb * m2.sf;
    result.sc = m1.sc * m2.s0 + m1.sd * m2.s4 + m1.se * m2.s8 + m1.sf * m2.sc;
    result.sd = m1.sc * m2.s1 + m1.sd * m2.s5 + m1.se * m2.s9 + m1.sf * m2.sd;
    result.se = m1.sc * m2.s2 + m1.sd * m2.s6 + m1.se * m2.sa + m1.sf * m2.se;
    result.sf = m1.sc * m2.s3 + m1.sd * m2.s7 + m1.se * m2.sb + m1.sf * m2.sf;
    return result;
}

//identity matrix
cl_float16 MathUtils::identiyMatrix(){
    cl_float16 matrix;
    matrix.s0 = 1;
    matrix.s1 = 0;
    matrix.s2 = 0;
    matrix.s3 = 0;

    matrix.s4 = 0;
    matrix.s5 = 1;
    matrix.s6 = 0;
    matrix.s7 = 0;

    matrix.s8 = 0;
    matrix.s9 = 0;
    matrix.sa = 1;
    matrix.sb = 0;

    matrix.sc = 0;
    matrix.sd = 0;
    matrix.se = 0;
    matrix.sf = 1;
    return matrix;
}

//rotate x
cl_float16 MathUtils::rotateX(cl_float16 xRotation, float angle){
    cl_float16 rotation = MathUtils::identiyMatrix();
    rotation.s5 = cos(angle);
    rotation.s6 = -sin(angle);
    rotation.s9 = sin(angle);
    rotation.sa = cos(angle);
    return multiply(xRotation, rotation);
}

//rotate y
cl_float16 MathUtils::rotateY(cl_float16 yRotation, float angle){
    cl_float16 rotation = MathUtils::identiyMatrix();
    rotation.s0 = cos(angle);
    rotation.s2 = sin(angle);
    rotation.s8 = -sin(angle);
    rotation.sa = cos(angle);
    return multiply(yRotation, rotation);
}

//rotate z
cl_float16 MathUtils::rotateZ(cl_float16 zRotation, float angle){
    cl_float16 rotation = MathUtils::identiyMatrix();
    rotation.s0 = cos(angle);
    rotation.s1 = -sin(angle);
    rotation.s4 = sin(angle);
    rotation.s5 = cos(angle);
    return multiply(zRotation, rotation);
}

//scale
cl_float16 MathUtils::scale(cl_float16 m, float x, float y, float z){
    cl_float16 result = MathUtils::identiyMatrix();
    result.s0 = x;
    result.s5 = y;
    result.sa = z;
    return multiply(m, result);
}

//translate
cl_float16 MathUtils::translate(cl_float16 m, float x, float y, float z){
    cl_float16 result = MathUtils::identiyMatrix();
    result.s3 = x;
    result.s7 = y;
    result.sb = z;
    return multiply(m, result);
}




