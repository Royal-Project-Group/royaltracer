//
// Created by m on 23.10.2023.
//

#include "PrecomputeKernelCommunicator.h"

void PrecomputeKernelCommunicator::delegateGPU(std::vector<Triangle> triangles, cl_float16 matrix, std::vector<Triangle>& output) {
    int d_triCount(triangles.size());
    std::string kfunctionName("gpuKernel");

    std::vector<cl_float4> mat = std::vector<cl_float4 >(4);

    for(int i = 0; i< 4; i++){
        mat[i] = { matrix.s[i*4], matrix.s[i*4+1], matrix.s[i*4+2], matrix.s[i*4+3]};
    }

    std::tuple inputTuple = std::make_tuple(
            InputBufferInfo<Triangle>{triangles},
            InputBufferInfo<cl_float4>{mat}
    );

    std::tuple outputTuple = std::make_tuple(
            OutputBufferInfo<Triangle>{&output[0],sizeof(Triangle)*output.size()}
    );

    std::tuple outputAdditionalArgs = std::make_tuple(
            d_triCount
    );

    kernelManager.executeOpenCL(
            triangles.size(), kfunctionName,
            inputTuple,
            outputTuple,
            outputAdditionalArgs
    );

}

void PrecomputeKernelCommunicator::setUp(std::string filename) {
    KernelManager man;
    this->kernelManager = man;
    kernelManager.setupKernel(filename);
}
