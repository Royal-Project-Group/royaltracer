//
// Created by m on 23.10.2023.
//

#ifndef TEST_KERNELMANAGER_H
#define TEST_KERNELMANAGER_H


#include <string>
#include <iostream>
#include "CL/cl2.hpp"
#include <tuple>
#include <vector>
#include <array>

template <typename T>
struct InputBufferInfo {
    const std::vector<T>& data;
    cl_mem_flags flags = CL_MEM_READ_ONLY;
};

template <typename T>
struct OutputBufferInfo {
    T* data;
    size_t size;
    cl_mem_flags flags = CL_MEM_WRITE_ONLY;
};

class KernelManager {
public:
    void setupKernel(std::string filename);
    void testRun();
    template <typename... Inputs, typename... Outputs, typename... KernelArgs>
    void executeOpenCL(
            int globalSize,
            std::string& kernelName,
            std::tuple<Inputs...>& inputBufferInfos,
            std::tuple<Outputs...>& outputBufferInfos,
            std::tuple<KernelArgs...>& kernelArgs);
private:
    cl::Context context;
    cl::Device device;
    cl::CommandQueue queue;
    cl::Program program;

    void readAndCompile(std::string filename);
    void assignDevice();

};


//Implemented in the header to prevent linker errors

/*
 * Use to pass arguments to the kernel and execute it automatically.
 * Pass input data (arrays), output data (arrays) and non buffered kernel args to the function.
 * The global size determines the number of IDs/Threads the kernel will be executing.
 *
 * IMPORTANT: the order in which the kernelArgs are passed is important; inputBuffers -> outputBuffers -> primitiveArgs
 */
/*
 *
Usage example:

int someIntValue = 5;
float someFloatValue = 3.14f;
// ... [other non-buffered values]

executeOpenCL(
    "gpuKernel",
    std::make_tuple(
        InputBufferInfo<Triangle>{d_tris},
        InputBufferInfo<Sphere>{d_spheres}
        // ... other input buffers
    ),
    std::make_tuple(
        OutputBufferInfo<cl_float4>{&pixelOutData[0], pixelOut.size()},
        OutputBufferInfo<cl_float4>{&albedoOutData[0], albedoOut.size()}
        // ... other output buffers
    ),
    std::make_tuple(someIntValue, someFloatValue)  // non-buffered kernel arguments
);
 */
template <typename... Inputs, typename... Outputs, typename... KernelArgs>
void KernelManager::executeOpenCL(
        int globalSize,
        std::string& kernelName,
        std::tuple<Inputs...>& inputBufferInfos,
        std::tuple<Outputs...>& outputBufferInfos,
        std::tuple<KernelArgs...>& kernelArgs
) {

    // Construct and enqueue input buffers
    auto inputBuffers = std::apply([this](const auto&... info) {
        return std::array{
                cl::Buffer(context, info.flags, sizeof(typename std::decay<decltype(info.data)>::type::value_type) * info.data.size())...
        };
    }, inputBufferInfos);

    std::apply([this, &inputBuffers](const auto&... info) {
        int index = 0;
        (queue.enqueueWriteBuffer(inputBuffers[index++], CL_TRUE, 0, sizeof(typename std::decay<decltype(info.data)>::type::value_type) * info.data.size(), &info.data[0]), ...);
    }, inputBufferInfos);

    // Construct output buffers
    auto outputBuffers = std::apply([this](const auto&... info) {
        return std::array{
                cl::Buffer(context, info.flags, info.size)...
        };
    }, outputBufferInfos);

    // Set the kernel program
    cl::Kernel kernel(program, kernelName.c_str());

    // Set the kernel args
    int argIndex = 0;
    for (auto& buf : inputBuffers) {
        kernel.setArg(argIndex++, buf);
    }
    for (auto& buf : outputBuffers) {
        kernel.setArg(argIndex++, buf);
    }

    std::apply([&kernel, &argIndex](auto&... args) {
        (kernel.setArg(argIndex++, args), ...);
    }, kernelArgs);
    //____________________________________________________________________________________________________________
    std::cout<<"Kernel Rendering\n";
    auto start = std::chrono::high_resolution_clock::now();
    //____________________________________________________________________________________________________________
    // Enqueue kernel
    cl::NDRange global_size(globalSize);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_size);
    queue.finish();
    //____________________________________________________________________________________________________________
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Rendering took " << duration.count() << " milliseconds." << std::endl;
    //____________________________________________________________________________________________________________

    std::apply([this, &outputBuffers](const auto&... info) {
        int index = 0;
        (queue.enqueueReadBuffer(outputBuffers[index++], CL_TRUE, 0, info.size, info.data), ...);
    }, outputBufferInfos);

}

#endif //TEST_KERNELMANAGER_H

