//Include header files containing modularized functionality
#include "KernelConstants.h"
#include "KernelTransferStructs.h"
#include "KernelStructs.h"
#include "KernelTools.h"
#include "KernelRayIntersection.h"
#include "KernelLights.h"
#include "KernelMapTools.h"
#include "KernelBRDF.h"
#include "KernelTrace.h"


__kernel void gpuKernel(
    __global struct TriBASE* triBase,
    __global struct TriEX* triEx,
    __global struct PointLight* pointLights,
    __global struct DirectionalLight* directionalLights,
    __global struct Material* materials,
    __global struct Camera* cam,
    __global struct Map* texture,
    __global struct Map* bumpMaps,
    __global struct Map* normalMaps,
    __global struct BVHNode* bvhNodes,
    __global uint* triIdx,

    __global float3* textureData,
    __global float* bumpMapData,
    __global float3* normalMapData,

    __global float4* pixelOut,
    __global float4* albedoOut,
    __global float4* normalOut,

    int triCount,
    int poiCount,
    int dirCount,
    int matCount,

    int bounces,
    int samples
)
{
    int id = get_global_id(0);

    struct SceneData bulkData;

    //First, transfer the given data to the kernel data struct. Also transfer the initial seed params.

    int x = id % cam->width;
    int y = id / cam->width;

    bulkData.triBase = triBase;
    bulkData.triEx = triEx;

    bulkData.pointLights = pointLights;
    bulkData.directionalLights = directionalLights;
    bulkData.materials = materials;
    bulkData.cam = cam;
    bulkData.texture = texture;
    bulkData.bumpMaps = bumpMaps;
    bulkData.normalMaps = normalMaps;
    bulkData.bvhNodes = bvhNodes;
    bulkData.triIdx = triIdx;

    bulkData.textureData = textureData;
    bulkData.bumpMapData = bumpMapData;
    bulkData.normalMapData = normalMapData;

    bulkData.triCount = triCount;
    bulkData.poiCount = poiCount;
    bulkData.dirCount = dirCount;
    bulkData.matCount = matCount;

    bulkData. samples = samples;
    bulkData.bounces = bounces;

    bulkData.seed0 = x;
    bulkData.seed1 = y;

    bulkData.pixelID = id;

    bulkData.pixelOut = pixelOut;
    bulkData.albedoOut = albedoOut;
    bulkData.normalOut = normalOut;




    //MOVE OUTSIDE THE KERNEL______________________________________________________

    // Derive the right vector from lookDirection and upVec
    float3 right = normalize(cross(cam->lookDirection, cam->upVec));
    float3 up = normalize(cam->upVec);
    float foV = cam->fieldOfView * (-3.1415f/180);

    // Calculate the pixel size based on FOV and width
    float pixelSize = 2 * tan(foV * 0.5f) / cam->width;
    float3 screenCenter = cam->position + normalize(cam->lookDirection);

    //______________________________________________________________________________


    //Create the first ray
    struct Ray ray;

    //Create the variables for tracking the path values. These are not stored a struct cause this will cause memory issues.
    float3 accumEmission = (float3)(0,0,0);
    float3 accumBRDF = (float3)(1,1,1);

    float aaStrength = 0.0f;

    //Tracer for each sample
    for(int i = 0; i < bulkData.samples; i++){

        bulkData.seed0 += i * y;
        bulkData.seed1 += i * x;

        // Pixel anti-aliasing
        float offsetX = (generateRandom(&bulkData.seed0, &bulkData.seed1) - 0.5f) * pixelSize * aaStrength;
        float offsetY = (generateRandom(&bulkData.seed0, &bulkData.seed1) - 0.5f) * pixelSize * aaStrength;

        float3 samplePixelPos = screenCenter
                                    + (((x - cam->width * 0.5f) * pixelSize + offsetX) * right)
                                    + (((y - cam->height * 0.5f) * pixelSize + offsetY) * up);
        ray.dir = normalize(samplePixelPos - cam->position);
        ray.orig = cam->position;


        tracePath(&bulkData, &ray, &accumEmission, &accumBRDF);
        accumBRDF = (float3)(1,1,1);
    }
    accumEmission /= samples;

    //Align the output
    //Cut every value >255
    if(accumEmission.x>255.0f)
        accumEmission.x= 255.0f;
    if(accumEmission.y>255.0f)
        accumEmission.y= 255.0f;
    if(accumEmission.z>255.0f)
        accumEmission.z= 255.0f;

    bulkData.pixelOut[id] = (float4)(accumEmission.x, accumEmission.y, accumEmission.z, id);
}