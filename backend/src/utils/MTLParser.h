//
// Created by nikla on 17.11.2023.
//

#ifndef ROYAL_TRACER_MTLPARSER_H
#define ROYAL_TRACER_MTLPARSER_H

#include <type_traits>
#include <functional>
#include <filesystem>

#include "../architecture/Parser.h"
#include "../architecture/TxrStorageSystem.h"
#include "Material.h"
#include "Map.h"

using float3 = std::array<float, 3>;

class MTLParser: public virtual Parser<std::vector<Material>> {
    std::vector<std::array<float, 3>> textureData;
    std::vector<std::array<float, 1>> bumpData;
    std::vector<std::array<float, 3>> normalData;

    float3 parseFloatTriplet(std::vector<std::string> triplet);

    void saveMapData(Material& m);

    void parseKx(std::vector<std::string> tokens, const std::function<void()>& modifier);

    /// This method returns the data contained in a image file acting as a texture/map
    template<int no_channels>
    Map parseMap(std::string filepath, std::vector<std::array<float, no_channels>>& data) {
        int mapWidth;
        int mapHeight;
        int mappp;

        // Load the image using stb_image
        std::uint8_t* image = TxrStorageSystem::get_instance().stbi_fetch(filepath, mapWidth, mapHeight, mappp, no_channels);

        if(image == nullptr) {
            return Map{0, 0, 0};
        }

        Map map{};
        map.width = mapWidth;
        map.height = mapHeight;
        map.offset = data.size();

        // Populate `data` with the image data
        for (int x = mapHeight - 1; x >= 0; x--) {
            for (int y = 0; y < mapWidth; y++) {
                int pixelIndex = (x * mapWidth + y) * no_channels;
                std::array<float, no_channels> pixelData;
                for(int i = 0; i < no_channels; i++) {
                    pixelData[i] = static_cast<float>(image[pixelIndex + i]) / 255.0f;
                }
                data.push_back(pixelData);
            }
        }

        // Free the loaded image data
        stbi_image_free(image);

        return map;
    }

    void determineElement(std::string line, std::shared_ptr<std::vector<Material>> res) override;

public:
    std::shared_ptr<std::vector<Material>> parse(const std::string &content) override;

    static Material getDefaultMaterial();
};


#endif //ROYAL_TRACER_MTLPARSER_H
