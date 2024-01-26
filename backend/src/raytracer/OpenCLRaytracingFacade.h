//
// Created by nikla on 31.10.2023.
//

#ifndef ROYAL_TRACER_OPENCLRAYTRACINGFACADE_H
#define ROYAL_TRACER_OPENCLRAYTRACINGFACADE_H

#include "../architecture/RoyalMediator.h"
#include "../utils/OBJParser.hpp"
#include "../utils/ThreeDObject.hpp"
#include "../utils/Material.h"
#include "../architecture/RaytracingFacade.h"
#include "RayTracerManagingLayer/ConfigParser.h"
#include "PrecomputeLayer/PrecomputeLayer.h"
#include "RayTracerManagingLayer/RTManager.h"
#include "SceneArchitecture/SceneStructs.h"
#include "OutputLayer/ImageDenoiser.h"
#include <mutex>

class OpenCLRaytracingFacade : public RaytracingFacade {
    std::mutex oidnMutex;
    std::mutex setupMutex;
private:

public:
    ~OpenCLRaytracingFacade() override = default;

    explicit OpenCLRaytracingFacade(RoyalMediator *rm);

    Image renderImage(std::string config) override;

    ThreeDObject getObjectContent(std::string filename, int id, int offset) override;

    Material getMaterialContent(std::string filename) override;
};


#endif //ROYAL_TRACER_OPENCLRAYTRACINGFACADE_H
