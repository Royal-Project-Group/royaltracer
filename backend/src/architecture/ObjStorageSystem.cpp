//
// Created by kneeclass on 11.12.23.
//

#include "ObjStorageSystem.h"

ObjStorageSystem::ObjStorageSystem() {
    // create the directory for 3D objects
    dir = std::string(RES_ROOT_DIR) + std::string(OBJ_DIR);
    std::filesystem::create_directory(dir);
}

bool ObjStorageSystem::store(std::string filename, std::string content) {
    try {
        std::fstream objFile(dir + filename, std::ios::out | std::ios::binary);
        objFile << content;
        objFile.close();

        return true;
    } catch (const std::exception& exception) {
        return false;
    }
}

bool ObjStorageSystem::remove(std::string filename) {
    return std::filesystem::remove(dir + filename);
}

std::string ObjStorageSystem::fetch(std::string filename) {
    std::ifstream inputFile(dir + filename);
    std::string content((std::istreambuf_iterator<char>(inputFile)),
                        std::istreambuf_iterator<char>());
    inputFile.close();
    if(content.empty()) {
        throw std::runtime_error("File '"+filename+"' not found!");
    }
    return content;
}