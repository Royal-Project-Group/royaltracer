//
// Created by stemp on 12.10.2023.
//

#include <iostream>
#include <cmath>
#include <chrono>
#include <tuple>
#include "RTManager.h"
#include "../KernelManager/KernelManager.h"
#include "../BVH/BVH.h"
#include "../../utils/Map.h"
#include "../SceneArchitecture/OptimizedKernelStructs.h"


void RTManager::startKernel(Scene sceneData, BVH bvh) {

    /*
     * Method for:
     *  - converting the scene data into kernel supported struct arrays
     *  - passing the scene data to the kernel
     *  - starting the kernel program on the kernel
     *  - supervising the kernel execution and creating a realtime preview image
     *  - collecting the data output from the kernel
     *  - returning the pixel data output as image (!CLASS WILL FOLLOW, CURRENTLY EMPTY FILLER!)
     */

    //Write the data into the shared memory
    delegateRendering(sceneData, bvh);
}


void RTManager::delegateRendering(Scene scene, BVH bvh){
    //____________________________________________________________________________________________________________
    std::cout<<"Benchmarking\n";
    std::cout<<"Start Raytracer\n";
    std::cout << "Load: " << scene.triangles.size() << " tries." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    //____________________________________________________________________________________________________________




    //____________________________________________________________________________________________________________
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Build BVH took " << duration.count() << " milliseconds." << std::endl;
    std::cout<<"________________________________________________________________________________"<<std::endl;
    //____________________________________________________________________________________________________________


    //Split triangles in two structs to speed up tree traversal
    std::vector<TriBASE>d_triBase(scene.triangles.size());
    std::vector<TriEX>d_triEx(scene.triangles.size());
    std::vector<PointLight>d_pointLights(scene.pointLights.size());
    std::vector<DirectionalLight>d_directionalLights(scene.directionalLights.size());
    std::vector<CLMaterial>d_materials(scene.materials.size());
    std::vector<Camera> d_camera(1);
    d_camera[0] = scene.camera;

    std::vector<Map> d_textures(scene.textures.size());
    std::vector<Map> d_bumpMaps(scene.bumpMaps.size());
    std::vector<Map> d_normalMaps(scene.normalMaps.size());

    std::vector<BVHNode> d_bvhNodes = bvh.GetBVHNodes();

    std::vector<cl_float3> d_textureData(scene.textureData.size());
    std::vector<cl_float> d_bumpMapData(scene.bumpMapData.size());
    std::vector<cl_float3> d_normalMapData(scene.normalMapData.size());
    std::vector<cl_uint> d_triIdx = bvh.GetTriIdx();

    //The data structure to hold the output color per pixel
    //(r, g, b, index)
    std::vector<cl_float4> pixelOut(scene.camera.width * scene.camera.height);
    //(r, g, b, index)
    std::vector<cl_float4> albedoOut(scene.camera.width * scene.camera.height);
    //(x, y, z, index)
    std::vector<cl_float4> normalOut(scene.camera.width * scene.camera.height);

    int d_triCount(scene.triangles.size());
    int d_poiLCount(scene.pointLights.size());
    int d_dirLCount(scene.directionalLights.size());
    int d_matCount(scene.materials.size());

    int d_bounces(scene.bounces);
    int d_samples(scene.samples);

    //Fill in the separate vectors
    //Triangles
    for(int i = 0; i<d_triCount;i++){
        TriBASE triBase;
        triBase.v1 = scene.triangles[i].v1;
        triBase.v2 = scene.triangles[i].v2;
        triBase.v3 = scene.triangles[i].v3;
        triBase.centroid = scene.triangles[i].centroid;
        triBase.vn = scene.triangles[i].vn;
        d_triBase[i] = triBase;

        TriEX triEx;
        triEx.materialID = scene.triangles[i].materialID;
        triEx.objectID = scene.triangles[i].objectID;
        triEx.uv1 = scene.triangles[i].uv1;
        triEx.uv2 = scene.triangles[i].uv2;
        triEx.uv3 = scene.triangles[i].uv3;

        triEx.vv0 = scene.triangles[i].vv0;
        triEx.vv1 = scene.triangles[i].vv1;
        triEx.d00 = scene.triangles[i].d00;
        triEx.d01 = scene.triangles[i].d01;
        triEx.d11 = scene.triangles[i].d11;
        triEx.invDenom = scene.triangles[i].invDenom;
        d_triEx[i] = triEx;
    }
    //Point Lights
    for(int i = 0; i<d_poiLCount;i++){
        d_pointLights[i] = scene.pointLights[i];
    }
    for(int i = 0; i<d_dirLCount;i++) {
        d_directionalLights[i] = scene.directionalLights[i];
    }
    //Materials
    for(int i = 0; i<d_matCount;i++){
        d_materials[i] = scene.materials[i];
    }
    //Textures
    for(int i =0; i<scene.textures.size();i++){
        d_textures[i] = scene.textures[i];
    }
    for(int i =0; i<scene.textureData.size();i++){
        d_textureData[i] = scene.textureData[i];
    }
    for(int i =0; i<scene.bumpMaps.size();i++){
        d_bumpMaps[i] = scene.bumpMaps[i];
    }
    for(int i =0; i<scene.bumpMapData.size();i++){
        d_bumpMapData[i] = scene.bumpMapData[i];
    }
    for(int i =0; i<scene.normalMaps.size();i++){
        d_normalMaps[i] = scene.normalMaps[i];
    }
    for(int i =0; i<scene.normalMapData.size();i++){
        d_normalMapData[i] = scene.normalMapData[i];
    }




    //KernelManager
    KernelManager kMan;
    std::string kfunctionName("gpuKernel");

    std::tuple inputTuple = std::make_tuple(
            InputBufferInfo<TriBASE>{d_triBase},
            InputBufferInfo<TriEX>{d_triEx},
            InputBufferInfo<PointLight>{d_pointLights},
            InputBufferInfo<DirectionalLight>{d_directionalLights},
            InputBufferInfo<CLMaterial>{d_materials},
            InputBufferInfo<Camera> {d_camera},
            InputBufferInfo<Map>{d_textures},
            InputBufferInfo<Map>{d_bumpMaps},
            InputBufferInfo<Map>{d_normalMaps},
            InputBufferInfo<BVHNode>{d_bvhNodes},
            InputBufferInfo<cl_uint>{d_triIdx},
            InputBufferInfo<cl_float3>{d_textureData},
            InputBufferInfo<cl_float>{d_bumpMapData},
            InputBufferInfo<cl_float3>{d_normalMapData}
    );

    std::tuple outputTuple = std::make_tuple(
            OutputBufferInfo<cl_float4>{&pixelOut[0],sizeof(cl_float4)*pixelOut.size()},
            OutputBufferInfo<cl_float4>{&albedoOut[0],sizeof(cl_float4)*albedoOut.size()},
            OutputBufferInfo<cl_float4>{&normalOut[0],sizeof(cl_float4)*normalOut.size()}
    );

    std::tuple outputAdditionalArgs = std::make_tuple(
            d_triCount,
            d_poiLCount,
            d_dirLCount,
            d_matCount,
            d_bounces,
            d_samples
    );

    std::cout<< "Setting up kernel for raytracing" << std::endl;

    {
        std::lock_guard<std::mutex> guard(setupMutex);
        std::cout<< "setup\n";
        kMan.setupKernel("kernelV3.cl");
        std::cout<< "setup done\n";
    }


    kMan.executeOpenCL(
            scene.camera.width * scene.camera.height, kfunctionName,
            inputTuple,
            outputTuple,
            outputAdditionalArgs
    );


    std::vector<std::vector<cl_float3>> matrix(scene.camera.height, std::vector<cl_float3>(scene.camera.width));
    std::vector<std::vector<cl_float3>> albedoMatrix(scene.camera.height, std::vector<cl_float3>(scene.camera.width));
    std::vector<std::vector<cl_float3>> normalMatrix(scene.camera.height, std::vector<cl_float3>(scene.camera.width));

    for (const auto& item : pixelOut) {
        int x = (int)item.w % scene.camera.width;
        int y = (int)item.w / scene.camera.width;
        matrix[y][x] = (cl_float3) {item.x,item.y, item.z};
    }

    for (const auto& item : albedoOut) {
        int x = (int)item.w % scene.camera.width;
        int y = (int)item.w / scene.camera.width;
        albedoMatrix[y][x] = (cl_float3) {item.x * 255.0f, item.y* 255.0f, item.z* 255.0f};
        //std::cout<<item.x<<item.y<<item.z<<"\n";
    }

    for (const auto& item : normalOut) {
        int x = (int)item.w % scene.camera.width;
        int y = (int)item.w / scene.camera.width;
        normalMatrix[y][x] = (cl_float3) {item.x,item.y, item.z};
    }

    width=scene.camera.width;
    height=scene.camera.height;

    pixels=matrix;
    albedos=albedoMatrix;
    normals=normalMatrix;
}

std::vector<std::vector<cl_float3>> RTManager::getPixels() {

   // invert image so x = widht is x = 0
    std::vector<std::vector<cl_float3>> pixels(height, std::vector<cl_float3>(width));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; ++x) {
            pixels[y][x] = this->pixels[y][width - x - 1];
        }
    }


    return pixels;
}

std::vector<std::vector<cl_float3>> RTManager::getAlbedos() {
    return albedos;
}

std::vector<std::vector<cl_float3>> RTManager::getNormals() {
    return normals;
}





//____________________________________________________________________________________________________________________________________________

//Test functions
cl_float3 cross(const cl_float3 &a, const cl_float3 &b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
cl_float3 subtract(const cl_float3 &a, const cl_float3 &b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
cl_float3 normalize(const cl_float3 &v) {
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return {v.x / length, v.y / length, v.z / length};
}



// Function to translate a point
cl_float3 translate(cl_float3 point, float x, float y, float z) {
    return {point.x + x, point.y + y, point.z + z};
}

void addCubeToScene(Scene& scene, float width, float height, float depth,
                    float translateX, float translateY, float translateZ) {
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    float halfDepth = depth / 2.0f;

    // Define the 8 corners of the cube (like the box)
    cl_float3 topLeft     = {-halfWidth,  halfHeight,  halfDepth};
    cl_float3 topRight    = { halfWidth,  halfHeight,  halfDepth};
    cl_float3 bottomLeft  = {-halfWidth, -halfHeight,  halfDepth};
    cl_float3 bottomRight = { halfWidth, -halfHeight,  halfDepth};
    cl_float3 backTopLeft     = {-halfWidth,  halfHeight, -halfDepth};
    cl_float3 backTopRight    = { halfWidth,  halfHeight, -halfDepth};
    cl_float3 backBottomLeft  = {-halfWidth, -halfHeight, -halfDepth};
    cl_float3 backBottomRight = { halfWidth, -halfHeight, -halfDepth};

    topLeft = translate(topLeft, translateX, translateY, translateZ);
    topRight = translate(topRight, translateX, translateY, translateZ);
    bottomLeft = translate(bottomLeft, translateX, translateY, translateZ);
    bottomRight = translate(bottomRight, translateX, translateY, translateZ);
    backTopLeft = translate(backTopLeft, translateX, translateY, translateZ);
    backTopRight = translate(backTopRight, translateX, translateY, translateZ);
    backBottomLeft = translate(backBottomLeft, translateX, translateY, translateZ);
    backBottomRight = translate(backBottomRight, translateX, translateY, translateZ);


    // Front face
    scene.triangles.push_back({3,0, topLeft,     bottomLeft,  topRight,    {0,0,1}});
    scene.triangles.push_back({3,0, bottomLeft,  bottomRight, topRight,    {0,0,1}});

    // Back face
    scene.triangles.push_back({3,0, backTopLeft,      backTopRight,     backBottomLeft,   {0,0,-1}});
    scene.triangles.push_back({3,0, backBottomLeft,   backBottomRight,  backTopRight,     {0,0,-1}});

    // Left face
    scene.triangles.push_back({3,0, topLeft,     backTopLeft,      bottomLeft,  {-1,0,0}});
    scene.triangles.push_back({3,0, bottomLeft,  backBottomLeft,   backTopLeft,      {-1,0,0}});

    // Right face
    scene.triangles.push_back({3,0, topRight,    bottomRight, backTopRight,     {1,0,0}});
    scene.triangles.push_back({3,0, bottomRight, backBottomRight,  backTopRight,     {1,0,0}});

    // Top face
    scene.triangles.push_back({3,0, topLeft,     topRight,    backTopLeft,      {0,1,0}});
    scene.triangles.push_back({3,0, topRight,    backTopRight,     backTopLeft,      {0,1,0}});

    // Bottom face
    scene.triangles.push_back({3,0, bottomLeft,  backBottomLeft,   bottomRight, {0,-1,0}});
    scene.triangles.push_back({3,0, bottomRight, backBottomLeft,   backBottomRight,  {0,-1,0}});
}




void addWallsToScene(Scene& scene, float width, float height, float depth) {
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    float halfDepth = depth / 2.0f;

    // Define the 8 corners of the box
    cl_float3 frontTopLeft     = {-halfWidth,  halfHeight,  halfDepth};
    cl_float3 frontTopRight    = { halfWidth,  halfHeight,  halfDepth};
    cl_float3 frontBottomLeft  = {-halfWidth, -halfHeight,  halfDepth};
    cl_float3 frontBottomRight = { halfWidth, -halfHeight,  halfDepth};

    cl_float3 backTopLeft      = {-halfWidth,  halfHeight, -halfDepth};
    cl_float3 backTopRight     = { halfWidth,  halfHeight, -halfDepth};
    cl_float3 backBottomLeft   = {-halfWidth, -halfHeight, -halfDepth};
    cl_float3 backBottomRight  = { halfWidth, -halfHeight, -halfDepth};

    // Now, form triangles for each face using these corners

    // Front face
    scene.triangles.push_back({2,0, frontTopLeft,     frontBottomLeft,  frontTopRight,    {0,0,-1}});
    scene.triangles.push_back({2,0, frontBottomLeft,  frontBottomRight, frontTopRight,    {0,0,-1}});

    // Back face
    scene.triangles.push_back({2,0, backTopLeft,      backTopRight,     backBottomLeft,   {0,0,1}});
    scene.triangles.push_back({2,0, backBottomLeft,   backBottomRight,  backTopRight,     {0,0,1}});

    // Left face
    scene.triangles.push_back({2,0, frontTopLeft,     backTopLeft,      frontBottomLeft,  {1,0,0}});
    scene.triangles.push_back({2,0, frontBottomLeft,  backBottomLeft,   backTopLeft,      {1,0,0}});

    // Right face
    scene.triangles.push_back({2,0, frontTopRight,    frontBottomRight, backTopRight,     {-1,0,0}});
    scene.triangles.push_back({2, 0,frontBottomRight, backBottomRight,  backTopRight,     {-1,0,0}});

    // Top face
    scene.triangles.push_back({2,0, frontTopLeft,     frontTopRight,    backTopLeft,      {0,-1,0}});
    scene.triangles.push_back({2, 0,frontTopRight,    backTopRight,     backTopLeft,      {0,-1,0}});

    // Bottom face
    scene.triangles.push_back({2,0, frontBottomLeft,  backBottomLeft,   frontBottomRight, {0,1,0}});
    scene.triangles.push_back({2, 0,frontBottomRight, backBottomLeft,   backBottomRight,  {0,1,0}});
}


void addTriangularPrismToScene(Scene& scene, float width, float height, float depth,
                               float translateX, float translateY, float translateZ) {
    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;

    // Define the 6 corners of the triangular prism
    cl_float3 frontTop    = { 0,         height,  halfDepth};
    cl_float3 frontLeft   = {-halfWidth, 0,       halfDepth};
    cl_float3 frontRight  = { halfWidth, 0,       halfDepth};
    cl_float3 backTop     = { 0,         height, -halfDepth};
    cl_float3 backLeft    = {-halfWidth, 0,      -halfDepth};
    cl_float3 backRight   = { halfWidth, 0,      -halfDepth};

    frontTop = translate(frontTop, translateX, translateY, translateZ);
    frontLeft = translate(frontLeft, translateX, translateY, translateZ);
    frontRight = translate(frontRight, translateX, translateY, translateZ);
    backTop = translate(backTop, translateX, translateY, translateZ);
    backLeft = translate(backLeft, translateX, translateY, translateZ);
    backRight = translate(backRight, translateX, translateY, translateZ);

    // Front face
    scene.triangles.push_back({1,0, frontTop, frontLeft, frontRight, {0,0,1}});

    // Back face
    scene.triangles.push_back({1, 0,backTop, backRight, backLeft, {0,0,-1}});

    // Left face
    scene.triangles.push_back({1,0, frontLeft, backLeft, frontTop, {-1,0,0}});
    scene.triangles.push_back({1,0, frontTop, backLeft, backTop, {-1,0,0}});

    // Right face
    scene.triangles.push_back({1,0, frontRight, frontTop, backRight, {1,0,0}});
    scene.triangles.push_back({1, 0,backRight, frontTop, backTop, {1,0,0}});

    // Bottom face
    scene.triangles.push_back({1,0, frontLeft, frontRight, backLeft, {0,-1,0}});
    scene.triangles.push_back({1, 0,backLeft, frontRight, backRight, {0,-1,0}});
}




Scene RTManager::createTestScene() {
    Scene scene;

    // Materials
    CLMaterial emissiveMaterial, cubeMaterial, wallMaterial, redMaterial, redTransMaterial;

    // Emissive material
    emissiveMaterial.Ka = {1.0f, 1.0f, 1.0f};
    emissiveMaterial.Kd = {1.0f, 1.0f, 1.0f};
    emissiveMaterial.Ks = {0.0f, 0.0f, 0.0f};
    emissiveMaterial.Ke = {00.0f, 00.0f, 00.0f};
    emissiveMaterial.Tf = {1.0f, 1.0f, 1.0f};
    emissiveMaterial.Ns = 0.0f;
    emissiveMaterial.d = 0.0f;
    emissiveMaterial.Ni = 1.0f;
    emissiveMaterial.scatteringIntensity = 0.0f;
    emissiveMaterial.scatteringRadius = 0.0f;
    scene.materials.push_back(emissiveMaterial);

    // Cube material
    cubeMaterial.Ka = {1.0f, 1.0f, 1.0f};
    cubeMaterial.Kd = {1.0f, 1.0f, 1.0f};
    cubeMaterial.Ks = {1.0f, 1.0f, 1.0f};
    cubeMaterial.Ke = {0.0f, 0.0f, 0.0f};
    cubeMaterial.Tf = {1.0f, 1.0f, 1.0f};
    cubeMaterial.Ns = 1.0f;
    cubeMaterial.d = 1.0f;
    cubeMaterial.Ni = 1.5f;
    cubeMaterial.scatteringIntensity = 0.0f;
    cubeMaterial.scatteringRadius = 100000.0f;
    scene.materials.push_back(cubeMaterial);

    // Wall material
    wallMaterial.Ka = {1.0f, 1.0f, 1.0f};
    wallMaterial.Kd = {0.1f, 0.1f, 0.1f};
    wallMaterial.Ks = {0.0f, 0.0f, 0.0f};
    wallMaterial.Ke = {0.0f, 0.0f, 0.0f};
    wallMaterial.Tf = {1.0f, 1.0f, 1.0f};
    wallMaterial.Ns = 0.0f;
    wallMaterial.d = 0.0f;
    wallMaterial.Ni = 1.0f;
    wallMaterial.scatteringIntensity = 0.0f;
    wallMaterial.scatteringRadius = 0.0f;
    scene.materials.push_back(wallMaterial);

    // cube material
    redMaterial.Ka = {1.0f, 1.0f, 1.0f};
    redMaterial.Kd = {0.2f, 0.7f, 0.5f};
    redMaterial.Ks = {1.0f, 1.0f, 1.0f};
    redMaterial.Ke = {0.0f, 0.0f, 0.0f};
    redMaterial.Tf = {1.0f, 1.0f, 1.0f};
    redMaterial.Ns = 0.95f;
    redMaterial.d = 0.0f;
    redMaterial.Ni = 1.0f;
    redMaterial.scatteringIntensity = 0.0f;
    redMaterial.scatteringRadius = 0.0f;
    scene.materials.push_back(redMaterial);

    // Skin Material
    redTransMaterial.Ka = {1.0f, 1.0f, 1.0f};
    redTransMaterial.Kd = {0.992f, 0.863f, 0.706f};
    redTransMaterial.Ks = {0.7f, 0.7f, 0.7f};
    redTransMaterial.Ke = {0.0f, 0.0f, 0.0f};
    redTransMaterial.Tf = {1.0f, 0.8f, 0.8f};
    redTransMaterial.Ns = 0.97f;
    redTransMaterial.d = 0.8f;
    redTransMaterial.Ni = 1.35f;
    redTransMaterial.scatteringIntensity = 0.98f;
    redTransMaterial.scatteringRadius = 0.5f;
    scene.materials.push_back(redTransMaterial);


    // Emissive Sphere as Light
    Sphere lightSphere;
    lightSphere.position = {0.0f, 1.0f, 0.0f}; // Positioned above
    lightSphere.radius = 0.55f;
    lightSphere.materialID = 0; // Emissive material
    //scene.spheres.push_back(lightSphere);

    // Emissive Sphere as Light
    Sphere lightSphere2;
    lightSphere2.position = {-1.0f,-1.0f,0.0f}; // Positioned above
    //lightSphere2.position = {.2f,.1f,-1.0f};
    lightSphere2.radius = 0.3f;
    lightSphere2.materialID = 1; // Emissive material
    //scene.spheres.push_back(lightSphere2);


    Sphere otherSphere;
    otherSphere.position = {0.4f, 0.2f, 1.0f};
    otherSphere.radius = 0.2f;
    otherSphere.materialID = 4; // reflective material
    //scene.spheres.push_back(otherSphere);;



    //scene.triangles = testTri;
    addTriangularPrismToScene(scene, 0.5f, 0.5f, 0.5f, -0.5f,-1.4f,0.0f);
    //addCubeToScene(scene, 1.0f,1.0f,1.0f,0.5f,-1.1f,2.0f);

    PointLight light1;
    light1.Ke = {10000.0f,10000.0f,10000.0f};
    light1.position = {-1.5f,0,0};
    scene.pointLights.push_back(light1);

    addWallsToScene(scene,4.0f,2.5f,7.0f);

    // Camera settings
    scene.camera.position = {0.0f, -0.3f, -3.0f};
    scene.camera.lookDirection = {0.0f, 0.0f, 1.0f};
    scene.camera.upVec = {0.0f, 1.0f, 0.0f};
    scene.camera.fieldOfView =65.0f;
    scene.camera.width = 1280*2;
    scene.camera.height =720*2;

    // Rendering settings
    scene.bounces = 8;
    scene.samples = 100;

    return scene;
}

