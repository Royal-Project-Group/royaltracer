//
// Created by nikla on 31.10.2023.
//

#include "OpenCLRaytracingFacade.h"
#include "OutputLayer/ImageTools.h"

#define SUPERSAMPLING_FACTOR 1

OpenCLRaytracingFacade::OpenCLRaytracingFacade(RoyalMediator *rm) : RaytracingFacade(rm) {

}

Image OpenCLRaytracingFacade::renderImage(std::string config) {
    ConfigParser configParser(config, this);
    std::shared_ptr<Scene> scene = configParser.parse();

    {
        std::lock_guard<std::mutex> guard(oidnMutex);
        PrecomputeLayer preLayer;
        scene = preLayer.operate(scene);
    }
    scene->camera.width = scene->camera.width *SUPERSAMPLING_FACTOR;
    scene->camera.height = scene->camera.height *SUPERSAMPLING_FACTOR;

    //First, build the BV Hierarchy:
    BVH bvh;
    bvh.BuildBVH(*scene);

    {
        std::lock_guard<std::mutex> guard(oidnMutex);

        RTManager rtManager;

        ImageDenoiser denoiser;
        ImageTools imageTools;


        rtManager.startKernel(*scene, bvh);



        std::vector<std::vector<cl_float3>> pixels = rtManager.getPixels();
        std::vector<std::vector<cl_float3>> albedos = rtManager.getAlbedos();
        std::vector<std::vector<cl_float3>> normals = rtManager.getNormals();


        std::cout<< "denoising\n";
        pixels = denoiser.denoise(pixels, albedos, normals);

        std::cout<< "denoising done\n";

        pixels = imageTools.antiAliasImage(pixels, SUPERSAMPLING_FACTOR);

        std::vector<std::vector<std::vector<float>>> outputVector;

        for (const auto &row : pixels) {
            std::vector<std::vector<float>> convertedRow;
            for (const auto &pixel : row) {
                std::vector<float> convertedElement = {pixel.x, pixel.y, pixel.z};
                convertedRow.push_back(convertedElement);
            }
            outputVector.push_back(convertedRow);
        }

        return Image(outputVector);
    }

}

ThreeDObject OpenCLRaytracingFacade::getObjectContent(std::string filename, int id, int offset) {
    OBJParser objParser = OBJParser{};
    return *objParser.parse(
            mediator->mediateResourceFetch(Obj, filename), id, offset
    );
}

Material OpenCLRaytracingFacade::getMaterialContent(std::string filename) {
    mediator->mediateResourceFetch(Mat, filename);
}
