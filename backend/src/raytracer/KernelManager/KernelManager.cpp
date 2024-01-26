//
// Created by m on 23.10.2023.
//

#include <CL/cl2.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include "KernelManager.h"


// Convert a string to lowercase
std::string to_lower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lower_str;
}



// Check if the string contains any of the specified substrings
bool contains_any(const std::string& str, const std::vector<std::string>& substrings) {
    for (const auto& substring : substrings) {
        if (str.find(substring) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Check if the string contains the specified substring
bool contains(const std::string& str, const std::string& substring) {
    return str.find(substring) != std::string::npos;
}

bool is_gpu_device(const cl::Device& device) {
    cl_device_type device_type;
    device.getInfo(CL_DEVICE_TYPE, &device_type);
    return (device_type & CL_DEVICE_TYPE_GPU) != 0;
}


//private
void KernelManager::readAndCompile(std::string filename) {
    //OpenCL program compiler and take .cl file
    /*
     * Read in the .cl file into a string
     * The file has to be located in the build directory
     */
    cl::Program::Sources sources;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open OpenCL file: " + filename);
    }

    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    std::string kernel_code = ss.str();

    /*
     * Compile the program using the opencl compiler
     */
    sources.push_back({kernel_code.c_str(), kernel_code.length()});
    program=cl::Program(context, sources);
    std::string options = "-I";
    options += " C:\\Niklas\\Studium\\Informatik\\WiSe2324\\Softwareprojekt\\royal-tracer\\backend\\cmake-build-debug";
    if (program.build({ device  }/*, options.c_str()*/) != CL_SUCCESS) {
        std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
        throw std::runtime_error("kernel error");
    }
    std::cout<<"Finished build\n";
}


void KernelManager::assignDevice() {
    //List for Platforms
    std::vector<cl::Platform> all_platforms;
    //Get the platforms
    cl::Platform::get(&all_platforms);

    //Check if there's at least one supported device
    if(all_platforms.empty()) {
        std::cerr << "No platforms found. Check OpenCL installation!\n";
        return;
    }

    //Variable to store the best device:
    cl::Device bestDevice;
    //Variable to store the computational power
    int power = 0;

    std::cout<<"Available platforms \n";
    //Output all available platforms
    for(auto& platform : all_platforms) {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);  // get all devices in this platform


        int devicePower=0; //Used to compare the devices power

        //Simple algorithm to determine the strongest device (e.g. Nvidia 4070 vs INTEL UHD: 160k to 50k)
        for(auto& device : devices) {
            std::cout << "Device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;

            //New method for computing a devices fp32 gFlop performance
            bool is_gpu = is_gpu_device(device);
            int compute_units = (int)device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(); // compute units (CUs) can contain multiple cores depending on the microarchitecture
            int clock_frequency = (int)device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>(); // in MHz
            int ipc = ipc = device.getInfo<CL_DEVICE_TYPE>()==CL_DEVICE_TYPE_GPU ? 2 : 32; // IPC (instructions per cycle) is 2 for GPUs and 32 for most modern CPUs



            std::string name = device.getInfo<CL_DEVICE_NAME>(); // device name
            std::string vendor = device.getInfo<CL_DEVICE_VENDOR>(); // device vendor
            bool nvidia_192_cores_per_cu = contains_any(to_lower(name), {"gt 6", "gt 7", "gtx 6", "gtx 7", "quadro k", "tesla k"}) || (clock_frequency<1000u&&contains(to_lower(name), "titan")); // identify Kepler GPUs
            bool nvidia_64_cores_per_cu = contains_any(to_lower(name), {"p100", "v100", "a100", "a30", " 16", " 20", "titan v", "titan rtx", "quadro t", "tesla t", "quadro rtx"}) && !contains(to_lower(name), "rtx a"); // identify P100, Volta, Turing, A100, A30
            bool amd_128_cores_per_dualcu = contains(to_lower(name), "gfx10"); // identify RDNA/RDNA2 GPUs where dual CUs are reported
            bool amd_256_cores_per_dualcu = contains(to_lower(name), "gfx11"); // identify RDNA3 GPUs where dual CUs are reported

            float cores_per_cu_nvidia = (float)(contains(to_lower(vendor), "nvidia"))*(nvidia_64_cores_per_cu?64.0f:nvidia_192_cores_per_cu?192.0f:128.0f); // Nvidia GPUs have 192 cores/CU (Kepler), 128 cores/CU (Maxwell, Pascal, Ampere, Hopper, Ada) or 64 cores/CU (P100, Volta, Turing, A100, A30)
            float cores_per_cu_amd = (float)(contains_any(to_lower(vendor), {"amd", "advanced"}))*(is_gpu?(amd_256_cores_per_dualcu?256.0f:amd_128_cores_per_dualcu?128.0f:64.0f):0.5f); // AMD GPUs have 64 cores/CU (GCN, CDNA), 128 cores/dualCU (RDNA, RDNA2) or 256 cores/dualCU (RDNA3), AMD CPUs (with SMT) have 1/2 core/CU
            float cores_per_cu_intel = (float)(contains(to_lower(vendor), "intel"))*(is_gpu?8.0f:0.5f); // Intel integrated GPUs usually have 8 cores/CU, Intel CPUs (with HT) have 1/2 core/CU
            float cores_per_cu_apple = (float)(contains(to_lower(vendor), "apple"))*(128.0f); // Apple ARM GPUs usually have 128 cores/CU
            float cores_per_cu_arm = (float)(contains(to_lower(vendor), "arm"))*(is_gpu?8.0f:1.0f); // ARM GPUs usually have 8 cores/CU, ARM CPUs have 1 core/CU

            float cores_per_cu = cores_per_cu_nvidia+cores_per_cu_amd+cores_per_cu_intel+cores_per_cu_apple+cores_per_cu_arm; // for CPUs, compute_units is the number of threads (twice the number of cores with hyperthreading)


            int cores = compute_units*cores_per_cu;
            float tflops = 1E-6f*(float)cores*(float)ipc*(float)clock_frequency;

            //devicePower=device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() * device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() * device.getInfo<CL_DEV>();
            std::cout<< "Performance: " << tflops << " tflops" << std::endl;

            if(tflops>power && is_gpu){
                power=tflops;
                bestDevice=device;
            }

            //bestDevice = devices.back();
        }
    }
    device=bestDevice;
    std::cout<< "Using device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    context = cl::Context({bestDevice});
    queue = cl::CommandQueue(context, bestDevice);
    std::cout<<"Context updated!\n";
}




void KernelManager::setupKernel(std::string filename) {
    /*
     * Function for:
     *  - Setting up the global variables for the kernel: context, device, queue, ...
     *  - Collecting device information
     *  - Find the strongest device (e.g. NVIDIA GPU) on the system for parallel processing
     *  - Read in the kernel code and compile it
     */

    //Collect device information and choose the best option for running the kernel on (external GPU > on-chip GPU > CPU)


    assignDevice();
    //Read in the File and compile it
    readAndCompile(filename);

}

void KernelManager::testRun(){
    cl::Kernel kernel(program, "gpuKernel");
    cl::NDRange global_size(1);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_size);
    queue.finish();
}
