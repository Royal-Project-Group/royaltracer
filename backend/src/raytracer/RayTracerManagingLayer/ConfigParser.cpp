//
// Created by olafs on 27.10.2023.
//

#include "ConfigParser.h"

constexpr float LIGHT_INTENSITY_MULTIPLICATOR = 5000.0f;


//Configfile String needed for the constructor!

//Method to call to get the Scene
std::shared_ptr<Scene> ConfigParser::parse() {

    // Turn \n characters into real line breaks.
    size_t found = config.find("\\n");
    while (found != std::string::npos) {
        config.replace(found, 2, " ");
        found = config.find("\\n", found + 1);
    }
    found = config.find("\\");
    while (found != std::string::npos) {
        config.replace(found, 1, "");
        found = config.find("\\", found + 1);
    }
    scene->materials.push_back(*convertToCLMaterial(MTLParser::getDefaultMaterial()));

    if (checkKeyword("models:") && checkKeyword("pointLights:") && checkKeyword("camera:") &&
        checkKeyword("extraArgs:")) {
        findModels();
        findPointLights();
        parseCamera();
        parseExtraArgs();

    } else {
        throw std::invalid_argument("Config is insufficient!");
    }
    if(scene->triangles.empty()) {
        throw std::runtime_error("Cannot pass empty list of triangles to kernel!");
    }
    return this->scene;
}

// Auxiliary method
// Find the position of the given keyword in the main configuration string.
size_t ConfigParser::findKeyword(std::string keyword) {
    return config.find(keyword);
}

// Auxiliary method
// Find the position of the given keyword within a specific string.
size_t ConfigParser::findKeyword(std::string keyword, std::string section) {
    return section.find(keyword);
}

// Auxiliary method
// Check if the keyword exists in the main configuration string.
bool ConfigParser::checkKeyword(std::string kw) {
    return findKeyword(kw) != std::string::npos;
}

// Auxiliary method
// Check if the keyword exists within a specific string.
bool ConfigParser::checkKeyword(std::string kw, std::string section) {
    return findKeyword(kw, section) != std::string::npos;
}

//Logic to extract the modelssections in the configfilestring and pass it to Auxiliary method to process it.
void ConfigParser::findModels() {

    std::string str;
    str = "pointLights";
    size_t pos_p_Lights = findKeyword(str);
    str = "models:";
    size_t pos_models = findKeyword(str) + str.length();

    std::string modelsSection = config.substr(pos_models, pos_p_Lights - pos_models);

    str = "- filePath:";
    size_t pos_next_filePath;
    size_t pos_this_filePath;

    while (checkKeyword(str, modelsSection)) {
        pos_next_filePath = findKeyword(str, modelsSection.substr(str.length()));
        if (pos_next_filePath != std::string::npos)
            pos_next_filePath += str.length();
        else
            pos_next_filePath = modelsSection.length();
        parseModel(modelsSection.substr(0, pos_next_filePath));
        pos_this_filePath = pos_next_filePath;
        modelsSection = modelsSection.substr(pos_this_filePath);

    }
}

// Auxiliary method
// Input a String "PATH" and extracting file path String
std::string parseFilePath(std::string section) {

    auto pathPattern = std::regex("\"([A-Za-z0-9\\./()-_*@#%&^=+]+)\"");
    std::smatch pathMatch;
    std::regex_search(section, pathMatch, pathPattern);
    return pathMatch.str().substr(1, pathMatch.str().length() - 2);

}

// Auxiliary method
// Convert a simple triangle to a complex Triangle structure.
std::shared_ptr<Triangle> ConfigParser::convertToCLTriangle(SimpleTriangle t) {
    std::shared_ptr<Triangle> res = std::make_shared<Triangle>();
    res->objectID = t.obj_id;
    res->materialID = t.materialID;
    res->v1 = {t.v1[0], t.v1[1], t.v1[2]};
    res->v2 = {t.v2[0], t.v2[1], t.v2[2]};
    res->v3 = {t.v3[0], t.v3[1], t.v3[2]};

    res->vn = {t.vn[0], t.vn[1], t.vn[2]};

    res->uv1 = {t.vt1[0], t.vt1[1]};
    res->uv2 = {t.vt2[0], t.vt2[1]};
    res->uv3 = {t.vt3[0], t.vt3[1]};

    return res;
}

std::shared_ptr<Object3D> ConfigParser::convertToCLObject(ThreeDObject o) {
    std::shared_ptr<Object3D> res = std::make_shared<Object3D>();
    res->id = o.id;
    res->position = {o.position[0], o.position[1], o.position[2]};
    res->rotation = {o.rotation[0], o.rotation[1], o.rotation[2]};
    res->scale = {o.scale[0], o.scale[1], o.scale[2]};

    return res;
}

std::shared_ptr<CLMaterial> ConfigParser::convertToCLMaterial(Material m) {
    std::shared_ptr<CLMaterial> res = std::make_shared<CLMaterial>();

    res->Ka = {m.sss_intensity, m.sss_radius, 0.0f};
    res->Kd = {m.Kd[0], m.Kd[1], m.Kd[2]};
    res->Ks = {1.0f - m.Pr, m.Pm, 0.0f};
    res->Ke = {m.Ke[0], m.Ke[1], m.Ke[2]};
    res->Tf = {m.Tf[0], m.Tf[1], m.Tf[2]};
    res->Ns = m.Ns;
    res->d = m.d;
    res->Ni = m.Ni;

    return res;
}

void ConfigParser::fillTextureData(std::vector<std::array<float, 3>> new_data) {
    for (std::array<float, 3> word : new_data) {
        scene->textureData.push_back({word[0], word[1], word[2]});
    }
}

void ConfigParser::fillBumpData(std::vector<std::array<float, 1>> new_data) {
    for (std::array<float, 1> word : new_data) {
        scene->bumpMapData.push_back(word[0]);
    }
}

void ConfigParser::fillNormalData(std::vector<std::array<float, 3>> new_data) {
    for (std::array<float, 3> word : new_data) {
        scene->normalMapData.push_back({word[0], word[1], word[2]});
    }
}

// Auxiliary method
// Logic to parse and process model information and add it to scene obj.
// TODO: Unfinished: missing ID-system and missing passing of position, rotation, scale
void ConfigParser::parseModel(std::string section) {
    std::string filepath = parseFilePath(section);
    ThreeDObject obj = facade->getObjectContent(filepath, objID_counter++, offset);

    //Find position, rotation and scale and add it to ThreeDObject
    std::string positionStr = "position:";
    size_t pos_position = findKeyword(positionStr, section);

    std::string rotationStr = "rotation:";
    size_t pos_rotation = findKeyword(rotationStr, section);

    std::string scaleStr = "scale:";
    size_t pos_scale = findKeyword(scaleStr, section);

    std::string positionSection = section.substr(pos_position + positionStr.length(),
                                                 pos_rotation - pos_position - positionStr.length());
    std::shared_ptr<std::array<float, 3>> xyz = extractXYZ(positionSection);
    obj.position = {(*xyz)[0], (*xyz)[1], (*xyz)[2]};

    std::string rotationSection = section.substr(pos_rotation + rotationStr.length(),
                                                 pos_scale - pos_rotation - rotationStr.length());
    xyz = extractXYZ(rotationSection);
    obj.rotation = {(*xyz)[0], (*xyz)[1], (*xyz)[2]};

    std::string scaleSection = section.substr(pos_scale + scaleStr.length(),
                                              section.length() - pos_scale - scaleStr.length());
    xyz = extractXYZ(scaleSection);
    obj.scale = {(*xyz)[0], (*xyz)[1], (*xyz)[2]};

    // add object to scene
    scene->objects.push_back(*convertToCLObject(obj));


    for(Material mat : obj.mat){
        if(obj.mat.size() != 0) {
            // check maps
            std::shared_ptr<CLMaterial> cl_mat = convertToCLMaterial(mat);
            if(!mat.map_Kd_data.empty()) {
                // adapt offset (if other textures have already been added)
                mat.map_Kd.offset += scene->textureData.size();
                scene->textures.push_back(mat.map_Kd);
                cl_mat->texture_id = scene->textures.size()-1;
                fillTextureData(mat.map_Kd_data);
            }
            if(!mat.bump_data.empty()) {
                // adapt offset (if other bump maps have already been added)
                mat.bump.offset += scene->bumpMaps.size();
                scene->bumpMaps.push_back(mat.bump);
                cl_mat->bumpMap_id = scene->bumpMaps.size()-1;
                fillBumpData(mat.bump_data);
            }
            if(!mat.norm_data.empty()) {
                // adapt offset (if other normal maps have already been added)
                mat.norm.offset += scene->normalMaps.size();
                scene->normalMaps.push_back(mat.norm);
                cl_mat->normalMap_id = scene->normalMaps.size()-1;
                fillNormalData(mat.norm_data);
            }
            offset++;
            scene->materials.push_back(*cl_mat);
        }
    }


    // add triangles to scene
    //std::vector <std::shared_ptr<Triangle>> cl_triangles;



    //std::transform(obj.triangles.begin(), obj.triangles.end(), cl_triangles.begin(), convertToCLTriangle);
    for (SimpleTriangle smpl_tri: obj.triangles) {
        std::shared_ptr<Triangle> cl_tri = convertToCLTriangle(smpl_tri);
        scene->triangles.push_back(*cl_tri);
    }
}

//Logic to extract the pointLightsectors in the configfilestring and pass it to Auxiliary method to process it.
void ConfigParser::findPointLights() {
    std::string str;
    str = "camera:";
    size_t pos_camera = findKeyword(str);
    str = "pointLights:";
    size_t pos_p_Lights = findKeyword(str) + str.length();

    std::string pointLightsSection = config.substr(pos_p_Lights, pos_camera - pos_p_Lights);

    str = "- position:";
    size_t pos_next_position;
    size_t pos_this_position;

    while (checkKeyword(str, pointLightsSection)) {
        pos_next_position = findKeyword(str, pointLightsSection.substr(str.length()));
        if (pos_next_position != std::string::npos)
            pos_next_position += str.length();
        else
            pos_next_position = pointLightsSection.length();
        parsePointLight(pointLightsSection.substr(0, pos_next_position));
        pos_this_position = pos_next_position;
        pointLightsSection = pointLightsSection.substr(pos_this_position);

    }
}

// Auxiliary method
// Logic to parse and process point light details and add it to scene obj.
void ConfigParser::parsePointLight(std::string section) {
    PointLight pointLight{};

    std::string str = "position:";
    size_t positionPos = findKeyword(str, section) + str.length();
    str = "Ke:";
    size_t kePos = findKeyword(str, section) + str.length();

    std::string positionString = section.substr(positionPos, kePos - (positionPos + str.length()));
    std::shared_ptr<std::array<float, 3>> xyz = extractXYZ(positionString);
    pointLight.position = {(*xyz)[0], (*xyz)[1], (*xyz)[2]};

    std::string keString = section.substr(kePos);
    std::shared_ptr<std::array<float, 3>> rgb = extractRGB(keString);
    pointLight.Ke = {(*rgb)[0], (*rgb)[1], (*rgb)[2]};

    scene->pointLights.push_back(pointLight);

}

//Logic to parse and process camera settings and add it to scene obj.
void ConfigParser::parseCamera() {
    std::string cameraStr = "camera:";
    size_t cameraPos = findKeyword(cameraStr) + cameraStr.length();
    std::string extraArgsStr = "extraArgs:";
    size_t extraArgsPos = findKeyword(extraArgsStr);
    std::string cameraString = config.substr(cameraPos, extraArgsPos - cameraPos);

    if (checkKeyword("position") && checkKeyword("lookAt") && checkKeyword("upVec") && checkKeyword("fieldOfView") &&
        checkKeyword("width") && checkKeyword("height")) {
        std::string positionStr = "position:";
        size_t positionPos = findKeyword(positionStr, cameraString) + positionStr.length();
        std::string lookAtStr = "lookAt:";
        size_t lookAtPos = findKeyword(lookAtStr, cameraString) + lookAtStr.length();
        std::string upVecStr = "upVec:";
        size_t upVecPos = findKeyword(upVecStr, cameraString) + upVecStr.length();
        std::string fieldOfViewStr = "fieldOfView:";
        size_t fieldOfViewPos = findKeyword(fieldOfViewStr, cameraString) + fieldOfViewStr.length();
        std::string widthStr = "width:";
        size_t widthPos = findKeyword(widthStr, cameraString) + widthStr.length();
        std::string heightStr = "height:";
        size_t heightPos = findKeyword(heightStr, cameraString) + heightStr.length();

        std::string positionString = cameraString.substr(positionPos, (lookAtPos - positionPos - lookAtStr.length()));
        std::shared_ptr<std::array<float, 3>> xyz = extractXYZ(positionString);
        scene->camera.position = {(*xyz)[0], (*xyz)[1], (*xyz)[2]};

        std::string lookAtString = cameraString.substr(lookAtPos, (upVecPos - lookAtPos - upVecStr.length()));
        xyz = extractXYZ(lookAtString);
        scene->camera.lookDirection = {(*xyz)[0] - scene->camera.position.x , (*xyz)[1] - scene->camera.position.y , (*xyz)[2] - scene->camera.position.z };

        std::string upVecString = cameraString.substr(upVecPos, (fieldOfViewPos - upVecPos - fieldOfViewStr.length()));
        xyz = extractXYZ(upVecString);
        scene->camera.upVec = {(*xyz)[0], (*xyz)[1], (*xyz)[2]};

        std::string fieldOfViewValue = cameraString.substr(fieldOfViewPos, (widthPos - fieldOfViewPos - widthStr.length()));
        scene->camera.fieldOfView = std::stof(fieldOfViewValue);

        std::string widthValue = cameraString.substr(widthPos, (heightPos - widthPos - heightStr.length()));
        scene->camera.width = std::stoi(widthValue);

        std::string heightValue = cameraString.substr(heightPos);
        scene->camera.height = std::stoi(heightValue);

    }
}

// Logic to parse and process additional arguments and add it to scene obj.
void ConfigParser::parseExtraArgs() {
    std::string str = "extraArgs:";
    size_t pos = findKeyword(str) + str.length();
    std::string extraArgsString = config.substr(pos);

    if(checkKeyword("directionalLights:", extraArgsString)) {
        findDirectionalLightSection();
    }
    if (checkKeyword("bounces") && checkKeyword("samples")) {
        str = "bounces:";
        size_t bouncesPos = findKeyword(str, extraArgsString);
        str = "samples:";
        size_t samplesPos = findKeyword(str, extraArgsString);
        scene->bounces = std::stoi(
                extraArgsString.substr(bouncesPos + str.length(), samplesPos - (bouncesPos + str.length())));
        scene->samples = std::stoi(extraArgsString.substr(samplesPos + str.length()));

    }
}

// Auxiliary method
// Extract x,y,z from a string like { x: 2.0, y: 2.0, z: 2.0 } and return it as unique pointer for a array of floats
std::shared_ptr<std::array<float, 3>> ConfigParser::extractXYZ(const std::string &input) {
    std::shared_ptr<std::array<float, 3>> values = std::make_shared<std::array<float, 3>>();

    // patterns that match "x/y/z: <float>" respectively
    auto xPattern = std::regex(R"(x:\s*(-?\d+\.?\d*))");
    auto yPattern = std::regex(R"(y:\s*(-?\d+\.?\d*))");
    auto zPattern = std::regex(R"(z:\s*(-?\d+\.?\d*))");
    std::smatch inpMatch;

    // find matches in the section
    if(std::regex_search(input, inpMatch, xPattern)) {
        (*values)[0] = std::stof(inpMatch[1]);
    }
    if(std::regex_search(input, inpMatch, yPattern)) {
        (*values)[1] = std::stof(inpMatch[1]);
    }
    if(std::regex_search(input, inpMatch, zPattern)) {
        (*values)[2] = std::stof(inpMatch[1]);
    }

    return values;
}

// Auxiliary method
// Extract r,g,b from a string like { r: 2.0, g: 2.0, b: 2.0 } and return it as shared pointer for a array of floats
std::shared_ptr<std::array<float, 3>> ConfigParser::extractRGB(const std::string &input) {
    std::shared_ptr<std::array<float, 3>> values = std::make_shared<std::array<float, 3>>();
    float intensity = 1.0;

    // patterns that match "r/g/b: <float>" respectively
    auto rPattern = std::regex("r:\\s*(\\d+\\.?\\d*)");
    auto gPattern = std::regex("g:\\s*(\\d+\\.?\\d*)");
    auto bPattern = std::regex("b:\\s*(\\d+\\.?\\d*)");
    auto intensityPattern = std::regex("intensity:\\s*(\\d+\\.?\\d*)");
    std::smatch inpMatch;

    // find matches in section
    if(std::regex_search(input, inpMatch, rPattern)) {
        (*values)[0] = std::stof(inpMatch[1]);
    }
    if(std::regex_search(input, inpMatch, gPattern)) {
        (*values)[1] = std::stof(inpMatch[1]);
    }
    if(std::regex_search(input, inpMatch, bPattern)) {
        (*values)[2] = std::stof(inpMatch[1]);
    }
    if(std::regex_search(input, inpMatch, intensityPattern)) {
        intensity = std::stof(inpMatch[1]);
    }

    // apply scaling to lights to get proper results in physically-based rendering
    for(float& val: *values) {
        val *= LIGHT_INTENSITY_MULTIPLICATOR * intensity;
    }

    return values;
}

void ConfigParser::findDirectionalLightSection() {
    std::string str = "directionalLights:";
    size_t pos_dir_Lights = findKeyword(str) + str.length();
    str = "bounces:";
    size_t pos_bounces = findKeyword(str);
    str = "samples:";
    size_t pos_samples = findKeyword(str);

    // case 1: bounces and samples are specified before directional lights
    if (pos_dir_Lights > pos_bounces && pos_dir_Lights > pos_samples) {
        // just substring until EOF
        std::string dirLightsSection = config.substr(pos_dir_Lights);

        findDirectionalLights(dirLightsSection);
    }
    // case 2: bounces and samples are specified after directional lights
    else if (pos_dir_Lights < pos_bounces && pos_dir_Lights < pos_samples) {
        // get the first other property after directional lights
        size_t pos_closer = pos_bounces > pos_samples ? pos_samples : pos_bounces;
        std::string dirLightsSection = config.substr(pos_dir_Lights, pos_closer - pos_dir_Lights);

        findDirectionalLights(dirLightsSection);
    }
    // case 3: directional lights are specified in between bounces and samples
    else {
        // get the property specified after directional lights (denotes end of directional lights section)
        size_t pos_further = pos_bounces > pos_samples ? pos_bounces : pos_samples;
        std::string dirLightsSection = config.substr(pos_dir_Lights, pos_further - pos_dir_Lights);

        findDirectionalLights(dirLightsSection);
    }
}

void ConfigParser::findDirectionalLights(std::string dirLightsSection) {
    std::string str = "- direction:";
    size_t pos_next_direction;
    size_t pos_this_direction;

    while (checkKeyword(str, dirLightsSection)) {
        pos_next_direction = findKeyword(str, dirLightsSection.substr(str.length()));
        if (pos_next_direction != std::string::npos)
            pos_next_direction += str.length();
        else
            pos_next_direction = dirLightsSection.length();
        parseDirectionalLight(dirLightsSection.substr(0, pos_next_direction));
        pos_this_direction = pos_next_direction;
        dirLightsSection = dirLightsSection.substr(pos_this_direction);

    }
}

void ConfigParser::parseDirectionalLight(std::string section) {
    DirectionalLight directionalLight{};

    std::string str = "direction:";
    size_t directionPos = findKeyword(str, section) + str.length();
    str = "Ke:";
    size_t kePos = findKeyword(str, section) + str.length();

    std::string directionString = section.substr(directionPos, kePos - (directionPos + str.length()));
    std::shared_ptr<std::array<float, 3>> xyz = extractXYZ(directionString);
    directionalLight.direction = {(*xyz)[0], (*xyz)[1], (*xyz)[2]};

    std::string keString = section.substr(kePos);
    std::shared_ptr<std::array<float, 3>> rgb = extractRGB(keString);
    directionalLight.Ke = {(*rgb)[0], (*rgb)[1], (*rgb)[2]};

    scene->directionalLights.push_back(directionalLight);
}
