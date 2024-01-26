//
// Created by nikla on 29.12.2023.
//

#include "TxrStorageSystem.h"

TxrStorageSystem::TxrStorageSystem() {
    // create the directory for textures
    dir = std::string(RES_ROOT_DIR) + std::string(TXR_DIR);
    std::filesystem::create_directory(dir);
}

bool TxrStorageSystem::store(std::string filename, std::vector<std::uint8_t> content) {
    try {
        std::ofstream outputFile(dir + filename, std::ios::binary);

        // Get the stream buffer associated with the file
        std::streambuf* fileBuffer = outputFile.rdbuf();

        // Use the stream buffer to write bytes to the file
        fileBuffer->sputn(reinterpret_cast<const char*>(content.data()), content.size());

        outputFile.close();

        return true;
    } catch (const std::exception& exception) {
        return false;
    }
}

bool TxrStorageSystem::remove(std::string filename) {
    return std::filesystem::remove(dir + filename);
}

std::vector<uint8_t> TxrStorageSystem::fetch(std::string filename) {
    std::basic_ifstream<std::uint8_t> inputFile(dir + filename, std::ios::binary);
    std::vector<std::uint8_t> content;
    content.assign((std::istreambuf_iterator<std::uint8_t>(inputFile)),
                        std::istreambuf_iterator<std::uint8_t>());
    inputFile.close();
    return content;
}

std::uint8_t* TxrStorageSystem::stbi_fetch(std::string filename, int& w, int& h, int& n, int un) {
    return stbi_load((dir+filename).c_str(), &w, &h, &n, un);
}
