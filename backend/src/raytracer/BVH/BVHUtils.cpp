//
// Created by m on 21.11.2023.
//
/*
#include <random>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL.h>
#include <iostream>
#include "BVHUtils.h"
#include "../SceneArchitecture/SceneStructs.h"
#include "BVH.h"

// Function to generate a random float in the range [min, max]
float randomFloat(float min, float max) {
    // static is used here to initialize the random engine and distribution only once
    static std::random_device rd;  // Obtain a random number from hardware
    static std::mt19937 eng(rd()); // Seed the generator
    static std::uniform_real_distribution<> distr(min, max); // Define the range

    return static_cast<float>(distr(eng));
}


Scene BVHUtil::createRandomTris(int number) {
    Scene out;
    // initialize a scene with N random triangles
    for (int i = 0; i < number; i++) {
        Triangle tri;

        // Randomly position the first vertex within the entire range
        tri.v1 = {randomFloat(-10, 10), randomFloat(-10, 10), randomFloat(-10, 10)};

        // Create other vertices near v1 to keep the triangle size smaller
        float maxOffset = 1.0f; // Max distance from v1, adjust as needed
        tri.v2 = {tri.v1.x + randomFloat(-maxOffset, maxOffset),
                  tri.v1.y + randomFloat(-maxOffset, maxOffset),
                  tri.v1.z + randomFloat(-maxOffset, maxOffset)};

        tri.v3 = {tri.v1.x + randomFloat(-maxOffset, maxOffset),
                  tri.v1.y + randomFloat(-maxOffset, maxOffset),
                  tri.v1.z + randomFloat(-maxOffset, maxOffset)};

        out.triangles.push_back(tri);
    }

    return out;
}


SDL_Rect TransformToScreenSpace(cl_float3 min, cl_float3 max, int windowWidth, int windowHeight) {
    // Define the actual range of your BVH data
    float minX = -10.0f, maxX = 10.0f; // Update these values based on your data
    float minY = -10.0f, maxY = 10.0f;

    // Calculate scale factors based on the coordinate range and window size
    float scaleX = windowWidth / (maxX - minX);
    float scaleY = windowHeight / (maxY - minY);

    // Calculate translation offsets
    float offsetX = -minX * scaleX;
    float offsetY = -minY * scaleY;

    SDL_Rect rect;
    rect.x = static_cast<int>((min.x * scaleX) + offsetX);
    rect.y = static_cast<int>((windowHeight - (min.y * scaleY)) - offsetY);  // Inverting Y-axis for SDL
    rect.w = static_cast<int>((max.x - min.x) * scaleX);
    rect.h = static_cast<int>((max.y - min.y) * scaleY);

    // Invert the height calculation since SDL's Y-axis grows downwards
    rect.y -= rect.h;

    return rect;
}

SDL_Point TransformToScreenSpace(cl_float3 point, cl_float3 centroid) {
    // Define the actual range of your BVH data
    float minX = -10.0f, maxX = 10.0f; // Update these values based on your data
    float minY = -10.0f, maxY = 10.0f;

    // Translate and scale points
    float scaleX = 1024 / (maxX - minX);
    float scaleY = 720 / (maxY - minY);
    float offsetX = -minX * scaleX;
    float offsetY = -minY * scaleY;

    // Apply transformation
    SDL_Point screenPoint;
    screenPoint.x = static_cast<int>((point.x) * scaleX + offsetX);
    screenPoint.y = static_cast<int>((720 - ((point.y) * scaleY)) - offsetY);

    return screenPoint;
}

void DrawAABB(SDL_Renderer* renderer, cl_float3 aabbMin, cl_float3 aabbMax, int depth) {
    // Convert 3D coordinates to 2D (ignore z-axis)
    SDL_Rect rect = TransformToScreenSpace(aabbMin, aabbMax, 1024, 720);

    // Set color based on depth
    int greyShade = 20*depth; // You can adjust this value for lighter or darker grey
    SDL_SetRenderDrawColor(renderer, greyShade, greyShade, greyShade, 255);

    SDL_RenderDrawRect(renderer, &rect);
}

void DrawTriangle(SDL_Renderer* renderer, cl_float3 v1, cl_float3 v2, cl_float3 v3, cl_float3 centroid) {
    // Transform coordinates to screen space
    SDL_Point p1 = TransformToScreenSpace(v1, centroid);
    SDL_Point p2 = TransformToScreenSpace(v2, centroid);
    SDL_Point p3 = TransformToScreenSpace(v3, centroid);

    // Set color to green
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // RGBA for green

    // Draw the triangle
    SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);
    SDL_RenderDrawLine(renderer, p2.x, p2.y, p3.x, p3.y);
    SDL_RenderDrawLine(renderer, p3.x, p3.y, p1.x, p1.y);
}

cl_float3 CalculateCentroid(const Triangle& tri) {
    return {
            (tri.v1.x + tri.v2.x + tri.v3.x) / 3.0f,
            (tri.v1.y + tri.v2.y + tri.v3.y) / 3.0f,
            (tri.v1.z + tri.v2.z + tri.v3.z) / 3.0f
    };
}


void BVHUtil::RenderBVHAndTriangles(SDL_Renderer* renderer, BVHNode* nodes, int nodeIdx, std::vector<Triangle> triangles, int depth = 0) {
    BVHNode& node = nodes[nodeIdx];

    // Draw the bounding box
    DrawAABB(renderer, node.aabbMin, node.aabbMax, depth);

    // Draw the triangles if it's a leaf node
    if (node.isLeaf()) {
        for (int i = 0; i < node.triCount; ++i) {
            Triangle& tri = triangles[node.leftFirst + i];
            tri.centroid = CalculateCentroid(tri);
            DrawTriangle(renderer, tri.v1, tri.v2, tri.v3, tri.centroid);
        }
    } else {
        // Recurse for child nodes
        RenderBVHAndTriangles(renderer, nodes, node.leftFirst, triangles, depth + 1);
        RenderBVHAndTriangles(renderer, nodes, node.leftFirst + 1, triangles, depth + 1);
    }
}

void BVHUtil::showDebugRender(int numberTris) {

    Scene scene = createRandomTris(numberTris);

    BVH bvh;


    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();

    // Call the function you want to benchmark
    bvh.BuildBVH(scene);

    // Stop the timer
    auto stop = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    // Output the time taken
    std::cout << "BuildBVH took " << duration.count() << " milliseconds." << std::endl;

    //bvh.PrintBVHTree(0,0);


    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }

    SDL_Window* window = SDL_CreateWindow(
            "Preview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 720, SDL_WINDOW_OPENGL
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    RenderBVHAndTriangles(renderer, bvh.bvhNode, bvh.rootNodeIdx, scene.triangles); // Assuming rootNodeIdx is the index of your root node

    SDL_RenderPresent(renderer);

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void BVHUtil::showSceneRender(Scene scene) {


    BVH bvh;


    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();

    // Call the function you want to benchmark
    bvh.BuildBVH(scene);

    // Stop the timer
    auto stop = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    // Output the time taken
    std::cout << "BuildBVH took " << duration.count() << " milliseconds." << std::endl;

    bvh.PrintBVHTree(0,0);


    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }

    SDL_Window* window = SDL_CreateWindow(
            "Preview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 720, SDL_WINDOW_OPENGL
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    RenderBVHAndTriangles(renderer, bvh.bvhNode, bvh.rootNodeIdx, scene.triangles); // Assuming rootNodeIdx is the index of your root node

    SDL_RenderPresent(renderer);

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
*/