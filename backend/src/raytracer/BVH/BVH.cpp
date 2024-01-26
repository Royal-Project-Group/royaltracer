//
// Created by m on 20.11.2023.
//

#include <vector>
#include <iostream>
#include "BVH.h"


float fminf1(float a, float b) {
    return a < b ? a : b;
}

float fmaxf1(float a, float b) {
    return a > b ? a : b;
}

void swap(cl_uint &a, cl_uint &b) {
    cl_uint temp = a;
    a = b;
    b = temp;
}

cl_float3 fminf3(cl_float3 a, cl_float3 b) {
    cl_float3 result;
    result.x = fminf1(a.x, b.x);
    result.y = fminf1(a.y, b.y);
    result.z = fminf1(a.z, b.z);
    return result;
}

cl_float3 fmaxf3(cl_float3 a, cl_float3 b) {
    cl_float3 result;
    result.x = fmaxf1(a.x, b.x);
    result.y = fmaxf1(a.y, b.y);
    result.z = fmaxf1(a.z, b.z);
    return result;
}

cl_float3 subtract_cl_float3(cl_float3 a, cl_float3 b) {
    cl_float3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

float& getElement(cl_float3& vec, int index) {
    switch (index) {
        case 0: return vec.x;
        case 1: return vec.y;
        case 2: return vec.z;
        default: throw std::out_of_range("Index out of range");
    }
}


//Function for building the BVH accelerator
void BVH::BuildBVH(Scene scene)
{
    tri = scene.triangles;
    triIdx = new cl_uint[tri.size()];
    bvhNode = new BVHNode[2* tri.size()];
    for (int i = 0; i < tri.size(); i++) triIdx[i] = i;


    for (int i = 0; i < tri.size(); i++)
        tri[i].centroid = (cl_float3){ (tri[i].v1.x + tri[i].v2.x + tri[i].v3.x) * 0.33333333f,
                                       (tri[i].v1.y + tri[i].v2.y + tri[i].v3.y) * 0.33333333f,
                                       (tri[i].v1.z + tri[i].v2.z + tri[i].v3.z) * 0.33333333f };
    // assign all triangles to root node
    BVHNode& root = bvhNode[rootNodeIdx];
    root.leftFirst = 0, root.triCount = tri.size();
    UpdateNodeBounds( rootNodeIdx );
    // subdivide recursively
    Subdivide( rootNodeIdx );
}


void BVH::UpdateNodeBounds( cl_uint nodeIdx )
{
    BVHNode& node = bvhNode[nodeIdx];
    node.aabbMin = {1e30f ,1e30f ,1e30f };
    node.aabbMax = { -1e30f,-1e30f,-1e30f };
    for (cl_uint first = node.leftFirst, i = 0; i < node.triCount; i++)
    {
        cl_uint leafTriIdx = triIdx[first + i];
        Triangle& leafTri = tri[leafTriIdx];
        node.aabbMin = fminf3( node.aabbMin, leafTri.v1 ),
        node.aabbMin = fminf3( node.aabbMin, leafTri.v2 ),
        node.aabbMin = fminf3( node.aabbMin, leafTri.v3 ),
        node.aabbMax = fmaxf3( node.aabbMax, leafTri.v1 ),
        node.aabbMax = fmaxf3( node.aabbMax, leafTri.v2 ),
        node.aabbMax = fmaxf3( node.aabbMax, leafTri.v3 );
    }
}


float BVH::FindBestSplitPlane( BVHNode& node, int& axis, float& splitPos )
{
    float bestCost = 1e30f;
    for (int a = 0; a < 3; a++)
    {
        float boundsMin = 1e30f, boundsMax = -1e30f;
        for (cl_uint i = 0; i < node.triCount; i++)
        {
            Triangle& triangle = tri[triIdx[node.leftFirst + i]];
            boundsMin = fminf1(boundsMin, getElement(triangle.centroid, a));
            boundsMax = fmaxf1(boundsMax, getElement(triangle.centroid, a));
        }
        if (boundsMin == boundsMax) continue;
        // populate the bins
        Bin bin[BINS];
        float scale = BINS / (boundsMax - boundsMin);
        for (cl_uint i = 0; i < node.triCount; i++)
        {
            Triangle& triangle = tri[triIdx[node.leftFirst + i]];
            int binIdx = fminf1(BINS - 1, (int) (((getElement(triangle.centroid, a) - boundsMin) * scale)));
            bin[binIdx].triCount++;
            bin[binIdx].bounds.grow( triangle.v1 );
            bin[binIdx].bounds.grow( triangle.v2 );
            bin[binIdx].bounds.grow( triangle.v3 );
        }
        // gather data for the 7 planes between the 8 bins
        float leftArea[BINS - 1], rightArea[BINS - 1];
        int leftCount[BINS - 1], rightCount[BINS - 1];
        aabb leftBox, rightBox;
        int leftSum = 0, rightSum = 0;
        for (int i = 0; i < BINS - 1; i++)
        {
            leftSum += bin[i].triCount;
            leftCount[i] = leftSum;
            leftBox.grow( bin[i].bounds );
            leftArea[i] = leftBox.area();
            rightSum += bin[BINS - 1 - i].triCount;
            rightCount[BINS - 2 - i] = rightSum;
            rightBox.grow( bin[BINS - 1 - i].bounds );
            rightArea[BINS - 2 - i] = rightBox.area();
        }
        // calculate SAH cost for the 7 planes
        scale = (boundsMax - boundsMin) / BINS;
        for (int i = 0; i < BINS - 1; i++)
        {
            float planeCost = leftCount[i] * leftArea[i] + rightCount[i] * rightArea[i];
            if (planeCost < bestCost)
                axis = a, splitPos = boundsMin + scale * (i + 1), bestCost = planeCost;
        }
    }
    return bestCost;
}


float BVH::CalculateNodeCost( BVHNode& node )
{
    cl_float3 e = subtract_cl_float3(node.aabbMax, node.aabbMin); // extent of the node
    float surfaceArea = e.x * e.y + e.y * e.z + e.z * e.x;
    return node.triCount * surfaceArea;
}


void BVH::Subdivide( cl_uint nodeIdx )
{
    // terminate recursion
    BVHNode& node = bvhNode[nodeIdx];
    // determine split axis using SAH
    int axis;
    float splitPos;
    float splitCost = FindBestSplitPlane( node, axis, splitPos );
    float nosplitCost = CalculateNodeCost( node );
    if (splitCost >= nosplitCost) return;
// in-place partition
    int i = node.leftFirst;
    int j = i + node.triCount - 1;
    while (i <= j)
    {
        if (getElement(tri[triIdx[i]].centroid, axis) < splitPos)
            i++;
        else
            swap(triIdx[i], triIdx[j--]);
    }
    // abort split if one of the sides is empty
    int leftCount = i - node.leftFirst;
    if (leftCount == 0 || leftCount == node.triCount) return;
    // create child nodes
    int leftChildIdx = nodesUsed++;
    int rightChildIdx = nodesUsed++;
    bvhNode[leftChildIdx].leftFirst = node.leftFirst;
    bvhNode[leftChildIdx].triCount = leftCount;
    bvhNode[rightChildIdx].leftFirst = i;
    bvhNode[rightChildIdx].triCount = node.triCount - leftCount;
    node.leftFirst = leftChildIdx;
    node.triCount = 0;
    UpdateNodeBounds( leftChildIdx );
    UpdateNodeBounds( rightChildIdx );
    // recurse
    Subdivide( leftChildIdx );
    Subdivide( rightChildIdx );
}



//Test output
void BVH::PrintBVHTree(cl_uint nodeIdx, int depth = 0) {
    // Get the node
    BVHNode& node = bvhNode[nodeIdx];

    // Print indentation for the current depth level
    for (int i = 0; i < depth; i++) {
        std::cout << "\t";
    }

    // Print node information
    std::cout << "Node " << nodeIdx << ": ";
    std::cout << "Tri Count = " << node.triCount << ", ";
    std::cout << "AABB Min = (" << node.aabbMin.x << ", " << node.aabbMin.y << ", " << node.aabbMin.z << "), ";
    std::cout << "AABB Max = (" << node.aabbMax.x << ", " << node.aabbMax.y << ", " << node.aabbMax.z << ")";
    std::cout << std::endl;

    // Check if the node is not a leaf node
    if (node.triCount == 0) {
        // Recursive call for the left child
        PrintBVHTree(node.leftFirst, depth + 1);
        // Recursive call for the right child
        PrintBVHTree(node.leftFirst + 1, depth + 1);
    }
}

std::vector<BVHNode> BVH::GetBVHNodes() {
    std::vector<BVHNode> nodes;
    for (int i = 0; i < nodesUsed; i++) {
        nodes.push_back(bvhNode[i]);
    }
    return nodes;
}

std::vector<cl_uint> BVH::GetTriIdx() {
    std::vector<cl_uint> triIdxs;
    for (int i = 0; i < tri.size(); i++) {
        triIdxs.push_back(triIdx[i]);
    }
    return triIdxs;
}
