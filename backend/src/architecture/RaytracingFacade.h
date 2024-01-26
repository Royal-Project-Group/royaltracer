//
// Created by nikla on 22.10.2023.
//

#ifndef ROYAL_TRACER_RAYTRACINGFACADE_H
#define ROYAL_TRACER_RAYTRACINGFACADE_H

#include <vector>
#include <string>
#include "../interfaces/Colleague.h"
#include "../utils/Material.h"
#include "../utils/ThreeDObject.hpp"
#include "./Image.h"

/// Facade class that encapsulates the raytracing subsystem.
/// All calls from outside to inside the raytracer should pass this class, and vice versa.
/// @inherit Colleague, as a communication partner to the @a RoyalMediator
class RaytracingFacade : public Colleague {
public:
    virtual ~RaytracingFacade() = default;

    explicit RaytracingFacade(RoyalMediator *rm);
    // methods that go outside raytracer -> inside raytracer

    /// Called when the client requests to raytrace a scene.
    /// Is responsible for initializing & invoking the raytracer and getting the resulting
    /// image.
    /// @param
    /// @returns an image of the raytraced scene.
    virtual Image renderImage(std::string config) = 0;

    // methods that go inside raytracer -> outside raytracer

    /// Called when a reference to a 3D Object has been encountered by the raytracer.
    /// Is responsible for fetching the contents of the 3D Object from the OBJ Storage System.
    /// @returns the struct that corresponds to the referenced object.
    virtual ThreeDObject getObjectContent(std::string filename, int id, int offset) = 0;

    /// Called when a reference to a Material has been encountered by the raytracer.
    /// Is responsible for fetching the contents of the Material from the MTL Storage System.
    /// @returns the struct that corresponds to the referenced material.
    virtual Material getMaterialContent(std::string filename) = 0;
};


#endif //ROYAL_TRACER_RAYTRACINGFACADE_H
