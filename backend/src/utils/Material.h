//
// Created by nikla on 18.10.2023.
//

#ifndef ROYAL_TRACER_MATERIAL_H
#define ROYAL_TRACER_MATERIAL_H

#include <array>
#include <string>

#include "Map.h"

using float3 = std::array<float, 3>;

/// class representation of a Wavefront Material
struct Material {
    // material name
    std::string name;
    // ambient color/map (unrealistic, should be disabled in the pathtracer)
    float3 Ka;
    Map map_Ka;
    // diffuse color/map (texture)
    float3 Kd = {1.0f, 1.0f, 1.0f};
    Map map_Kd;
    std::vector<std::array<float, 3>> map_Kd_data;
    // specular color/map
    float3 Ks;
    Map map_Ks;
    // emissive color/map
    float3 Ke;
    Map map_Ke;
    //Transparency color
    float3 Tf = {1.0f, 1.0f, 1.0f};
    //shininess / map
    float Ns = 1000.0f;
    Map map_Ns;
    //transparency / map
    float d = 1.0f;
    Map map_d;
    //Refractive index
    float Ni = 1.0f;
    // metallicity
    float Pm = 0.0f;
    // roughness
    float Pr = 1.0f;
    // subsurface scattering radius
    float sss_radius = 100000.0f;
    // subsurface scattering intensity
    float sss_intensity = 1.0f;
    // illumination model
    short illum;

    Map bump;
    std::vector<std::array<float, 1>> bump_data;
    Map norm;
    std::vector<std::array<float, 3>> norm_data;
};

#endif //ROYAL_TRACER_MATERIAL_H
