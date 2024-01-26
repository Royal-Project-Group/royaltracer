//
// Created by m on 20.10.2023.
//

#include "ImageDenoiser.h"
#include "../../utils/Material.h"
#include <OpenImageDenoise/oidn.hpp>
#include <iostream>
#include <chrono>


std::vector<std::vector<cl_float3>> ImageDenoiser::denoise(
        std::vector<std::vector<cl_float3>> pixels,
        std::vector<std::vector<cl_float3>> albedos,
        std::vector<std::vector<cl_float3>> normals
        ) {

    int WIDTH =pixels[0].size();
    int HEIGHT=pixels.size();

    std::cout<< WIDTH;

    std::vector<float3> image(WIDTH * HEIGHT);
    std::vector<float3> albedo(WIDTH * HEIGHT);
    std::vector<float3> normal(WIDTH * HEIGHT);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            image[y * WIDTH + x] = (float3) {pixels[y][x].x,pixels[y][x].y, pixels[y][x].z};
            albedo[y * WIDTH + x] = (float3) {albedos[y][x].x,albedos[y][x].y, albedos[y][x].z};
            normal[y * WIDTH + x] = (float3) {normals[y][x].x,normals[y][x].y, normals[y][x].z};
            //std::cout<<pixels[y][x].x<<"  "<<pixels[y][x].y<<"  "<< pixels[y][x].z<<"\n";
        }
    }
    std::cout<<"__________________________________________________________________________________________________\n";
    // Initialize the denoiser
    oidn::DeviceRef dev = oidn::newDevice();
    dev.commit();

    // Create memory buffers using OIDNBuffer
    oidn::BufferRef colorBuffer = dev.newBuffer(sizeof(float3) * WIDTH * HEIGHT);
    oidn::BufferRef albedoBuffer = dev.newBuffer(sizeof(float3) * WIDTH * HEIGHT);
    oidn::BufferRef normalBuffer = dev.newBuffer(sizeof(float3) * WIDTH * HEIGHT);
    oidn::BufferRef outputBuffer = dev.newBuffer(sizeof(float3) * WIDTH * HEIGHT);


    oidn::FilterRef filter = dev.newFilter("RTLightmap");
    filter.setImage("color", colorBuffer, oidn::Format::Float3, WIDTH, HEIGHT);
    filter.setImage("albedo", albedoBuffer, oidn::Format::Float3, WIDTH, HEIGHT);
    filter.setImage("normal", normalBuffer, oidn::Format::Float3, WIDTH, HEIGHT);
    filter.setImage("output", outputBuffer, oidn::Format::Float3, WIDTH, HEIGHT);
    filter.set("hdr", true);
    filter.set("quality", OIDN_QUALITY_HIGH);
    filter.commit();


    // Copy your image data into the OIDNBuffer
    std::memcpy(colorBuffer.getData(), &image[0], sizeof(float3) * WIDTH * HEIGHT);
    std::memcpy(albedoBuffer.getData(), &albedo[0], sizeof(float3) * WIDTH * HEIGHT);
    std::memcpy(normalBuffer.getData(), &normal[0], sizeof(float3) * WIDTH * HEIGHT);

    //Profiling
    auto start = std::chrono::high_resolution_clock::now();

    // Denoise
    filter.execute();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time Denoiser: " << elapsed.count() << " seconds" << std::endl;

    // Check for errors
    const char* errorMessage;
    if (dev.getError(errorMessage) != oidn::Error::None) {
        // Handle the error
        std::cerr << "OIDN error: " << errorMessage << std::endl;
    }

    // Copy the denoised image back to your original data
    std::memcpy(&image[0], outputBuffer.getData(), sizeof(float) * 3 * WIDTH * HEIGHT);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            cl_float3 pixel =(cl_float3 ) {image[y * WIDTH + x][0],image[y * WIDTH + x][1],image[y * WIDTH + x][2]};
            //normalize the output
            //Every value that is higher than 255, cut off.
            if(pixel.x>255.0f)
                pixel.x= 255.0f;
            if(pixel.y>255.0f)
                pixel.y= 255.0f;
            if(pixel.z>255.0f)
                pixel.z= 255.0f;
            pixels[y][x] = pixel;
            //std::cout<<pixel.x<<"  "<<pixel.y<<"  "<< pixel.z<<"\n";
        }
    }

    colorBuffer.release();
    albedoBuffer.release();
    normalBuffer.release();
    outputBuffer.release();


    return pixels;
}
