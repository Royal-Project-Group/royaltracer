//
// Created by stemp on 12.10.2023.
//

#ifndef TEST_IMAGETOOLS_H
#define TEST_IMAGETOOLS_H

#include <CL/cl.h>
#include <vector>
#include <string>

//Class to manage the Post Processing and Image saving
class ImageTools {
public:
    std::vector<std::vector<cl_float3>> antiAliasImage(const std::vector<std::vector<cl_float3>>& pixels, int factor);

    //void previewRender(int time, std::vector<std::vector<cl_float3>> pixels);
};


#endif //TEST_IMAGETOOLS_H
