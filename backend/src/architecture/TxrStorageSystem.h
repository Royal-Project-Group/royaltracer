//
// Created by nikla on 29.12.2023.
//

#ifndef ROYAL_TRACER_TXRSTORAGESYSTEM_H
#define ROYAL_TRACER_TXRSTORAGESYSTEM_H

#define TXR_DIR "txrs/"

#include <filesystem>
#include <fstream>
#include <vector>
#include "../interfaces/Singleton.h"
#include "../interfaces/StorageSystem.h"
#include "../utils/stb_image.h"

/// This class manages the textures and maps sent to the backend. It offers the basic file operations store, remove and fetch
/// Additionally, stbi_fetch gets the texture content using stb_image
class TxrStorageSystem: public virtual Singleton<TxrStorageSystem>, virtual StorageSystem<std::vector<std::uint8_t>> {
    friend class Singleton<TxrStorageSystem>;
public:
    bool store(std::string filename, std::vector<std::uint8_t> content) override;

    bool remove(std::string filename) override;

    std::vector<std::uint8_t> fetch(std::string filename) override;
    std::uint8_t* stbi_fetch(std::string filename, int& w, int& h, int& n, int un);
private:
    std::string dir;
    TxrStorageSystem();
};


#endif //ROYAL_TRACER_TXRSTORAGESYSTEM_H
