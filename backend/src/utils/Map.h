//
// Created by nikla on 22.11.2023.
//

#ifndef ROYAL_TRACER_MAP_H
#define ROYAL_TRACER_MAP_H

#include <vector>
#include <CL/cl2.hpp>

/// struct for normal, bump and other maps
struct Map{
    int width;
    int height;
    int offset; // offset = start index
};
#endif //ROYAL_TRACER_MAP_H
