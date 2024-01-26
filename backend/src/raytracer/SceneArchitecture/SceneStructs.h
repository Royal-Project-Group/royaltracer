//
// Created by m on 30.10.2023.
//

#ifndef TEST_SCENESTRUCTS_H
#define TEST_SCENESTRUCTS_H

#include <CL/cl2.hpp>
#include "../../utils/Map.h"
#include <vector>

struct CLMaterial {
    // ambient color (unrealistic, should be disabled in the pathtracer)
    cl_float3 Ka;
    // diffuse color
    cl_float3 Kd;
    // specular color
    cl_float3 Ks;
    // emissive color
    cl_float3 Ke;
    //Transparency color
    cl_float3 Tf;
    //shininess (how perfect are the reflections)
    cl_float Ns;
    //transparency
    cl_float d;
    //Refractive index
    cl_float Ni;

    // id for maps:
    int texture_id = -1;
    int bumpMap_id = -1;
    int normalMap_id = -1;

    // subsurface coefficient (has to be set in the editor)
    cl_float scatteringIntensity = 0.0f;
    cl_float scatteringRadius = 10000000.0f;
};
struct Triangle {
    cl_int materialID;
    cl_int objectID;
    cl_float3 v1, v2, v3;
    cl_float3 centroid; //Neu
    cl_float3 vn;
    cl_float2 uv1, uv2, uv3;

    // paramaters that can be precalulated for each triangle for map coordinates
    cl_float3 vv0, vv1;
    cl_float d00, d01, d11, invDenom;
};
struct Sphere {
    cl_int materialID;
    cl_float3 position;
    cl_float radius;
};
struct Camera {
    cl_float3 position;
    cl_float3 lookDirection;
    cl_float3 upVec;
    cl_float fieldOfView;
    cl_int width;
    cl_int height;
};
struct PointLight{
    cl_float3 position;
    cl_float3 Ke;
};
struct DirectionalLight {
    cl_float3 direction;
    cl_float3 Ke;
};
struct Object3D {
  cl_int id;
  cl_float3 position;
  cl_float3 rotation;
  cl_float3 scale;
};

struct Scene {
    //These equal to all the necessary parameters to describe the scene for the RT
    std::vector<Triangle> triangles;
    std::vector<Sphere> spheres;
    std::vector<PointLight> pointLights;
    std::vector<DirectionalLight> directionalLights;
    std::vector<CLMaterial> materials;
    std::vector<Object3D> objects;
    std::vector<Map> textures;
    std::vector<Map> bumpMaps;
    std::vector<Map> normalMaps;
    std::vector<cl_float3> textureData;
    std::vector<cl_float> bumpMapData;
    std::vector<cl_float3> normalMapData;
    Camera camera;

    //Additional Information concerning the render
    int bounces;
    int samples;
};

#endif //TEST_SCENESTRUCTS_H
