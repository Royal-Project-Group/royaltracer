//
// Created by stemp on 12.10.2023.
//

#include "ImageTools.h"
#include <png.h>

#include <iostream>
#include <algorithm>

/*
void ImageTools::previewRender(int time, std::vector<std::vector<cl_float3>> pixels) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }

    SDL_Window* window = SDL_CreateWindow(
            "Preview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pixels[0].size(), pixels.size(), SDL_WINDOW_OPENGL
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    for (int y = 0; y < pixels.size(); y++) {
        for (int x = 0; x < pixels[0].size(); x++) {
            SDL_SetRenderDrawColor(renderer, pixels[y][x].x, pixels[y][x].y, pixels[y][x].z, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

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



std::vector<std::vector<cl_float3>>
ImageTools::antiAliasImage(const std::vector<std::vector<cl_float3>> &pixels, int factor) {
    // Extract the height and width from the pixel matrix size
    int originalHeight = pixels.size() / factor;
    int originalWidth = pixels.empty() ? 0 : pixels[0].size() / factor;

    std::vector<std::vector<cl_float3>> aliasedMatrix(originalHeight, std::vector<cl_float3>(originalWidth));

    for (int y = 0; y < originalHeight; y++) {
        for (int x = 0; x < originalWidth; x++) {
            cl_float3 sum = {0.0f, 0.0f, 0.0f};
            for (int dy = 0; dy < factor; dy++) {
                for (int dx = 0; dx < factor; dx++) {
                    sum.x += pixels[y * factor + dy][x * factor + dx].x;
                    sum.y += pixels[y * factor + dy][x * factor + dx].y;
                    sum.z += pixels[y * factor + dy][x * factor + dx].z;
                }
            }
            float divisor = static_cast<float>(factor * factor);
            aliasedMatrix[y][x].x = sum.x / divisor;
            aliasedMatrix[y][x].y = sum.y / divisor;
            aliasedMatrix[y][x].z = sum.z / divisor;
        }
    }
    return aliasedMatrix;
}