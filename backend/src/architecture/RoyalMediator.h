//
// Created by nikla on 22.10.2023.
//

#ifndef ROYAL_TRACER_ROYALMEDIATOR_H
#define ROYAL_TRACER_ROYALMEDIATOR_H

#include "RaytracingFacade.h"
#include "EditorFacade.h"
#include "ObjStorageSystem.h"
#include "MtlStorageSystem.h"
#include "TxrStorageSystem.h"
#include "ResourceTypes.h"
#include "../utils/OBJParser.hpp"

/// This architectural class acts as a mediator between different components of the software.
/// It should only receive and make calls to other classes, without any real program logic.
class RoyalMediator {
private:
    const short PORT = 8080;
    std::string project_directory;
    RaytracingFacade *rf;
    EditorFacade *ef;
    OBJParser *objectParser;
public:
    explicit RoyalMediator(const char *project_dir);

    bool mediateResourceStorage(RESOURCE_TYPE type, std::string id, const std::string& content);

    bool mediateResourceStorage(RESOURCE_TYPE type, std::string id, std::vector<std::uint8_t>& content);

    bool mediateResourceRemoval(RESOURCE_TYPE type, std::string id);

    std::string mediateResourceFetch(RESOURCE_TYPE type, std::string filename);

    Image mediateImageRendering(std::string config);

    void setRf(RaytracingFacade *rf);

    void setEf(EditorFacade *ef);

};


#endif //ROYAL_TRACER_ROYALMEDIATOR_H
