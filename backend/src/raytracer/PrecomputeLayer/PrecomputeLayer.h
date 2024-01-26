//
// Created by stemp on 24.10.2023.
//

#ifndef TEST_PRECOMPUTELAYER_H
#define TEST_PRECOMPUTELAYER_H


#include "../RayTracerManagingLayer/RTManager.h"
#include "./PrecomputeKernelCommunicator.h"
#include "../../utils/MathUtils.h"
#include "../../utils/Map.h"

// class for precomputing the triangles (i.e. matrix transformation)
class PrecomputeLayer {
public:
    PrecomputeLayer();
    std::shared_ptr<Scene> operate(std::shared_ptr<Scene>);
private:
    PrecomputeKernelCommunicator precomputeKernelCommunicator;
};


#endif //TEST_PRECOMPUTELAYER_H
