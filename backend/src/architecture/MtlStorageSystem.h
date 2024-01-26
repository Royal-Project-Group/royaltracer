//
// Created by kneeclass on 11.12.23.
//

#ifndef ROYAL_TRACER_MTLSTORAGESYSTEM_H
#define ROYAL_TRACER_MTLSTORAGESYSTEM_H

#define MTL_DIR "mtls/"

#include <filesystem>
#include <fstream>
#include "../interfaces/StorageSystem.h"

/// This class manages the materials sent to the backend. It offers the basic file operations store, remove and fetch.
class MtlStorageSystem: public virtual Singleton<MtlStorageSystem>, virtual StorageSystem<std::string> {
    friend class Singleton<MtlStorageSystem>;
public:
    bool store(std::string filename, std::string content) override;

    bool remove(std::string filename) override;

    std::string fetch(std::string filename) override;
private:
    std::string dir;
    MtlStorageSystem();
};


#endif //ROYAL_TRACER_MTLSTORAGESYSTEM_H
