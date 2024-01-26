//
// Created by m on 23.11.2023.
// Contains the structs used to transfer the data to the kernel
//

struct Material {
    float3 Ka;
    float3 Kd;
    float3 Ks;
    float3 Ke;
    float3 Tf;
    float Ns;
    float d;
    float Ni;

    int texture_id;
    int bumpMap_id;
    int normalMap_id;

    float scatteringIntensity;
    float scatteringRadius;
};
struct TriBASE{
    float3 v1,v2,v3;
    float3 centroid, vn;
};

struct TriEX{
    int materialID;
    int objectID;
    float2 uv1, uv2, uv3;

    // paramaters that can be precalulated for each triangle for map coordinates
    float3 vv0, vv1;
    float d00, d01, d11, invDenom;
};
struct Sphere {
    int materialID;
    float3 position;
    float radius;
};
struct Camera {
    float3 position;
    float3 lookDirection;
    float3 upVec;
    float fieldOfView;
    int width;
    int height;
};
struct PointLight{
    float3 position;
    float3 Ke;
};
struct DirectionalLight{
    float3 direction;
    float3 Ke;
};

struct Map{
    int width;
    int height;
    int offset;
};

struct BVHNode{
    float3 min;
    int leftFirst;
    float3 max;
    int triCount;
};

