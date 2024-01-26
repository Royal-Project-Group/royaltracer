//
// Created by m on 20.11.2023.
//

#ifndef ROYAL_TRACER_BVH_H
#define ROYAL_TRACER_BVH_H


#include "../SceneArchitecture/SceneStructs.h"

// bin count
#define BINS 8


float fminf1(float a, float b);
float fmaxf1(float a, float b);
void swap(unsigned int &a, unsigned int &b);
cl_float3 fminf3(cl_float3 a, cl_float3 b);
cl_float3 fmaxf3(cl_float3 a, cl_float3 b);
cl_float3 subtract_cl_float3(cl_float3 a, cl_float3 b);
float& getElement(cl_float3& vec, int idx);


struct BVHNode
{
    cl_float3 aabbMin;
    cl_uint leftFirst;
    cl_float3 aabbMax;
    cl_uint triCount;
    bool isLeaf() { return triCount > 0; }
};



struct aabb
{
    cl_float3 bmin = {1e30f, 1e30f, 1e30f}, bmax = {-1e30f, -1e30f, -1e30f};
    void grow( cl_float3 p ) { bmin = fminf3( bmin, p ); bmax = fmaxf3( bmax, p ); }
    void grow( aabb& b ) { if (b.bmin.x != 1e30f) { grow( b.bmin ); grow( b.bmax ); } }
    float area()
    {
        cl_float3 e = subtract_cl_float3(bmax , bmin); // box extent
        return e.x * e.y + e.y * e.z + e.z * e.x;
    }
};
struct Bin { aabb bounds; int triCount = 0; };


class BVH{
public:
    std::vector<Triangle> tri;
    cl_uint rootNodeIdx = 0, nodesUsed = 1;
    BVHNode* bvhNode;
    cl_uint* triIdx;


    void BuildBVH(Scene scene);
    void UpdateNodeBounds( cl_uint nodeIdx );
    void Subdivide( cl_uint nodeIdx );
    void PrintBVHTree(cl_uint nodeIdx, int depth);

    float CalculateNodeCost(BVHNode &node);

    float FindBestSplitPlane(BVHNode &node, int &axis, float &splitPos);

    std::vector<BVHNode> GetBVHNodes();
    std::vector<cl_uint> GetTriIdx();
};


#endif //ROYAL_TRACER_BVH_H
