//
// Created by kneeclass on 11.12.23.
//

#include "MtlStorageSystem.h"

MtlStorageSystem::MtlStorageSystem() {
    // create the material for materials
    dir = std::string(RES_ROOT_DIR) + std::string(MTL_DIR);
    std::filesystem::create_directory(dir);
}

bool MtlStorageSystem::store(std::string filename, std::string content) {
    try {
        std::fstream file(dir + filename, std::ios::out | std::ios::binary);
        file << content;
        file.close();

        return true;
    } catch (const std::exception& exception) {
        return false;
    }
}

bool MtlStorageSystem::remove(std::string filename) {
    return std::filesystem::remove(dir + filename);
}

std::string MtlStorageSystem::fetch(std::string filename) {
    std::ifstream inputFile(dir + filename);
    std::string content((std::istreambuf_iterator<char>(inputFile)),
                        std::istreambuf_iterator<char>());
    inputFile.close();
    if(content.empty()) {
        throw std::runtime_error("File '"+filename+"' not found!");
    }
    return content;
}
