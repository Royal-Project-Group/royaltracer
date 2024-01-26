//
// Created by m on 23.11.2023.
// Contains the internal data struct system for the kernel
//

//First, a big data structure to store references to all the information sent to the kernel. This makes them accessible from everywhere without the need for many params
struct SceneData{
    //Struct type arrays
    __global struct TriBASE* triBase; //Triangle will be split to speed up tree traversal
    __global struct TriEX* triEx;
    __global struct PointLight* pointLights;
    __global struct DirectionalLight* directionalLights;
    __global struct Material* materials;
    __global struct Camera* cam;
    __global struct Map* texture;
    __global struct Map* bumpMaps;
    __global struct Map* normalMaps;
    __global struct BVHNode* bvhNodes;

    //Primitive type arrays
    __global float3* textureData;
    __global float* bumpMapData;
    __global float3* normalMapData;
    __global uint* triIdx;


    //Counts
    int triCount;
    int poiCount;
    int dirCount;
    int matCount;


    //Additional data
    int bounces;
    int samples;
    uint seed0;
    uint seed1;

    int waveLength;
    int pixelID;

    struct Material airMaterial;


    //Output data
    __global float4* pixelOut;
    __global float4* albedoOut;
    __global float4* normalOut;

};

struct Intersection
{

    float t;			// intersection distance along ray
    float2 pTex;			// map coordinates of the interseciton point
    uint instPrim;		// instance index (12 bit) and primitive index (20 bit)
};

struct Ray{
    float3 orig;
    float3 dir;
    float3 rD;
    struct Intersection hit;
};

struct HitInformation{
    float3 hitPoint;
    float3 hitNormal;
    bool insideMaterial;
    int currentObjectId;
    int hitObjectId;
    struct Material hitMaterial;
    struct Material currentMaterial;
    float pdf;
    short brdfType; //The type classifying the intersection (this is used to sample the lights correctly)
    //As follows: 0 diffuse, 1 reflective, 2 refractive, 3 subsurface, 4 fog scattering
    //Depending on the type, the light sampling will be adjusted
    short bounce;
};