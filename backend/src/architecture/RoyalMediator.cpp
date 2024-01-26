//
// Created by nikla on 22.10.2023.
//

#include "RoyalMediator.h"

RoyalMediator::RoyalMediator(const char *project_dir) : project_directory{project_dir} {}

bool RoyalMediator::mediateResourceStorage(RESOURCE_TYPE type, std::string id, const std::string& content) {
    // Call the storage system corresponding to the type of resource
    switch (type) {
        case Obj:
            return ObjStorageSystem::get_instance().store(id, content);
        case Mat:
            return MtlStorageSystem::get_instance().store(id, content);
        default:
            return false;
    }
}

bool RoyalMediator::mediateResourceStorage(RESOURCE_TYPE type, std::string id, std::vector<std::uint8_t> &content) {
    // Additional method needed for binary data (textures)
    switch (type) {
        case Txr:
            return TxrStorageSystem::get_instance().store(id, content);
        default:
            return false;
    }
}

bool RoyalMediator::mediateResourceRemoval(RESOURCE_TYPE type, std::string id) {
    // Call the storage system corresponding to the type of resource
    switch (type) {
        case Obj:
            return ObjStorageSystem::get_instance().remove(id);
        case Mat:
            return MtlStorageSystem::get_instance().remove(id);
        case Txr:
            return TxrStorageSystem::get_instance().remove(id);
        default:
            return false;
    }
}

std::string RoyalMediator::mediateResourceFetch(RESOURCE_TYPE type, std::string filename) {
    // Call the storage system corresponding to the type of resource
    switch (type) {
        case Obj:
            return ObjStorageSystem::get_instance().fetch(filename);
        case Mat:
            return MtlStorageSystem::get_instance().fetch(filename);
        default:
            return "";
    }
}

Image RoyalMediator::mediateImageRendering(std::string config) {

    Image image = rf->renderImage(config);

    return image;
}

void RoyalMediator::setRf(RaytracingFacade *rf) {
    RoyalMediator::rf = rf;
}

void RoyalMediator::setEf(EditorFacade *ef) {
    RoyalMediator::ef = ef;
}
