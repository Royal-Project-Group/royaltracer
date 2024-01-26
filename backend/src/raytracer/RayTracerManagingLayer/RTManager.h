//
// Created by stemp on 12.10.2023.
//

#ifndef TEST_RTMANAGER_H
#define TEST_RTMANAGER_H

#include <string>
#include "../SceneArchitecture/SceneStructs.h"
#include "../BVH/BVH.h"
#include <mutex>

class RTManager{

    std::mutex setupMutex;
public:
    void startKernel(Scene sceneData, BVH bvh);
    Scene createTestScene();
    std::vector<std::vector<cl_float3>> getPixels();
    std::vector<std::vector<cl_float3>> getAlbedos();
    std::vector<std::vector<cl_float3>> getNormals();

private:

    std::vector<std::vector<cl_float3>> pixels;
    std::vector<std::vector<cl_float3>> albedos;
    std::vector<std::vector<cl_float3>> normals;
    int width, height;

    void delegateRendering(Scene scene, BVH bvh);
};

#endif //TEST_RTMANAGER_H
