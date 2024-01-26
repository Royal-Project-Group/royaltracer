//
// Created by kneeclass on 11.12.23.
//

#ifndef ROYAL_TRACER_OBJSTORAGESYSTEM_H
#define ROYAL_TRACER_OBJSTORAGESYSTEM_H

#define OBJ_DIR "objs/"

#include <fstream>
#include <filesystem>
#include "../interfaces/StorageSystem.h"

/// This class manages the objects sent to the backend. It offers the basic file operations store, remove and fetch
class ObjStorageSystem: public virtual Singleton<ObjStorageSystem>, virtual StorageSystem<std::string> {
    friend class Singleton<ObjStorageSystem>;
public:
    bool store(std::string filename, std::string content) override;

    bool remove(std::string filename) override;

    std::string fetch(std::string filename) override;
private:
    std::string dir;
    ObjStorageSystem();
};


#endif //ROYAL_TRACER_OBJSTORAGESYSTEM_H
