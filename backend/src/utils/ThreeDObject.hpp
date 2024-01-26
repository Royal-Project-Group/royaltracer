#ifndef OBJ_ECT
#define OBJ_ECT

#include <atomic>
#include <vector>
#include <array>
#include <memory>

#include "SimpleTriangle.h"
#include "Material.h"

// class representation of a Wavefront OBJ
struct ThreeDObject {
    int id;
    std::vector<SimpleTriangle> triangles;
    std::array<float, 3> position, rotation, scale;

    std::vector<Material> mat;
};

#endif