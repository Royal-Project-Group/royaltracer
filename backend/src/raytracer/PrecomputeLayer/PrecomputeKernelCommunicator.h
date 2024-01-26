//
// Created by m on 23.10.2023.
//

#ifndef TEST_PRECOMPUTEKERNELCOMMUNICATOR_H
#define TEST_PRECOMPUTEKERNELCOMMUNICATOR_H


#include <vector>
#include "../KernelManager/KernelManager.h"
#include "../SceneArchitecture/SceneStructs.h"

class PrecomputeKernelCommunicator {
public:
    void setUp(std::string filename);
    void delegateGPU(std::vector<Triangle> triangles, cl_float16 matrix, std::vector<Triangle>& output );
private:
    KernelManager kernelManager;
};


#endif TEST_PRECOMPUTEKERNELCOMMUNICATOR_H
