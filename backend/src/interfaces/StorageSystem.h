//
// Created by kneeclass on 11.12.23.
//

#ifndef ROYAL_TRACER_STORAGESYSTEM_H
#define ROYAL_TRACER_STORAGESYSTEM_H

#define RES_ROOT_DIR "./resources/"

#include <string>
#include <memory>
#include "Singleton.h"

/// abstract class that acts as a blueprint for deriving storage systems.
/// Defines three interfaces store, remove and fetch.
template <typename ContentType>
class StorageSystem {
public:
    StorageSystem();
    // Idea: delete resources in destructor
    // ~StorageSystem();

    // Store a file with the given name and content
    virtual bool store(std::string filename, ContentType content)=0;

    // Remove the file with the given name
    virtual bool remove(std::string filename)=0;

    // Fetch the contents of the file with the given name
    virtual ContentType fetch(std::string filename)=0;
};

template<typename ContentType>
StorageSystem<ContentType>::StorageSystem() {
    // create root directory for resources
    std::filesystem::create_directory(RES_ROOT_DIR);
}

#endif //ROYAL_TRACER_STORAGESYSTEM_H
