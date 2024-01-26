//
// Created by m on 20.10.2023.
//

#ifndef TEST_IMAGEDENOISER_H
#define TEST_IMAGEDENOISER_H


#include <vector>
#include <CL/cl2.hpp>

class ImageDenoiser {
public:
    std::vector<std::vector<cl_float3>> denoise(std::vector<std::vector<cl_float3>> pixels, std::vector<std::vector<cl_float3>> albedos, std::vector<std::vector<cl_float3>> normals);
};


#endif //TEST_IMAGEDENOISER_H
