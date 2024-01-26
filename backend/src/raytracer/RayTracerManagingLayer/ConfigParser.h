#ifndef PROJECT_CONFIGPARSER_H
#define PROJECT_CONFIGPARSER_H

#include <string>
#include <regex>
#include <algorithm>
#include <filesystem>
#include <fstream>

#include "../OpenCLRaytracingFacade.h"
#include "../SceneArchitecture/SceneStructs.h"
#include "../../utils/SimpleTriangle.h"
#include "../../architecture/ObjStorageSystem.h"
#include <CL/cl2.hpp>
#include "../../utils/OBJParser.hpp"

class ConfigParser {
private:
    std::string config;
    RaytracingFacade* facade;
    std::shared_ptr <Scene> scene;
    int objID_counter = 0;
    int offset = 1; //Offset, accumulation of all previous materials (starts with 1 default)

    void findModels();

    void parseModel(std::string section);

    void findPointLights();

    void parsePointLight(std::string section);

    void findDirectionalLightSection();

    void findDirectionalLights(std::string dirLightsSection);

    void parseDirectionalLight(std::string section);

    void parseCamera();

    void parseExtraArgs();

    size_t findKeyword(std::string keyword);

    size_t findKeyword(std::string keyword, std::string section);

    bool checkKeyword(std::string keyword);

    bool checkKeyword(std::string keyword, std::string section);

    static std::shared_ptr<Triangle> convertToCLTriangle(SimpleTriangle t);

    static std::shared_ptr<Object3D> convertToCLObject(ThreeDObject o);

    std::shared_ptr<std::array<float, 3>> extractXYZ(const std::string &input);

    std::shared_ptr<std::array<float, 3>> extractRGB(const std::string &input);

    static std::shared_ptr<CLMaterial> convertToCLMaterial(Material m);

    void fillTextureData(std::vector<std::array<float, 3>> new_data);

    void fillBumpData(std::vector<std::array<float, 1>> new_data);

    void fillNormalData(std::vector<std::array<float, 3>> new_data);

public:
    ConfigParser(std::string conf, RaytracingFacade* _facade) :
            config{conf},
            scene{std::make_shared<Scene>()},
            facade(_facade) {}

    std::shared_ptr <Scene> parse();
};


#endif //PROJECT_CONFIGPARSER_H
