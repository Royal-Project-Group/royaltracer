//
// Created by nikla on 17.11.2023.
//

#include "MTLParser.h"

constexpr float LIGHT_INTENSITY_MULTIPLICATOR = 2500.0f;

/// This method returns a default material, for objects that are missing a material reference.
Material MTLParser::getDefaultMaterial() {
    Material res = Material();
    res.name = "default";
    res.Ka = {0, 0, 0};
    res.Kd = {.9f, .9f, .9f};
    res.Ks = {.0f, .0f, .0f};
    res.Tf = {1.0000, 1.0000, 1.0000};
    res.Ns = 1;
    res.Ni = 1;

    return res;
}

/// This method parses a triplet of three strings
float3 MTLParser::parseFloatTriplet(std::vector<std::string> triplet) {
    float a = std::stof(triplet[0]);
    float b = std::stof(triplet[1]);
    float c = std::stof(triplet[2]);
    return {a,b,c};
}

/// Save the map data in the material. Called whenever a newmtl statement is found
void MTLParser::saveMapData(Material& m) {
    if(!textureData.empty()) {
        m.map_Kd_data = textureData;
        textureData.clear();
    }
    if(!bumpData.empty()) {
        m.bump_data = bumpData;
        bumpData.clear();
    }
    if(!normalData.empty()) {
        m.norm_data = normalData;
        normalData.clear();
    }
}

/// Calls the given callback on a Kx statement
void MTLParser::parseKx(std::vector<std::string> tokens, const std::function<void()> &modifier) {
    const std::string& illuminator = tokens[1];
    if(illuminator == "spectral") {
        // TODO
    } else if (illuminator == "xyz") {
        // TODO
    } else {
        if (tokens.size() == 4) {
            modifier();
        }
    }
}

std::shared_ptr<std::vector<Material>> MTLParser::parse(const std::string &content) {
    std::shared_ptr<std::vector<Material>> res = Parser<std::vector<Material>>::parse(content);
    // save data to last material
    saveMapData(res->back());

    return res;
}

void MTLParser::determineElement(std::string line, std::shared_ptr<std::vector<Material>> res) {
    if(line.empty()) {
        return;
    }
    std::vector<std::string> tokens = lexpp::lex(line, " \t", false);

    const std::string& specifier = tokens[0];

    if(specifier == "#") {
        return;
    } else if(specifier == "newmtl") {
        // save data to old material
        saveMapData(res->back());
        // Create new material
        res->emplace_back();
        res->back().name = tokens[1];
    } else if (specifier == "Ka") {
        // ambient color (ignored in pathtracer)
        auto lambda = [res, this, tokens]() {
            res->back().Ka = parseFloatTriplet({tokens.begin()+1, tokens.end()});
        };
        parseKx(tokens, lambda);
    } else if (specifier == "Kd") {
        // diffuse color
        auto lambda = [res, this, tokens]() {
            res->back().Kd = parseFloatTriplet({tokens.begin()+1, tokens.end()});
        };
        parseKx(tokens,lambda);
    } else if (specifier == "Ks") {
        // specular color (ignored in pathtracer)
        auto lambda = [res, this, tokens]() {
            res->back().Ks = parseFloatTriplet({tokens.begin()+1, tokens.end()});
        };
        parseKx(tokens,lambda);
    } else if (specifier == "Ke") {
        // emissive color
        auto lambda = [res, this, tokens]() {
            float3 keVal = parseFloatTriplet({tokens.begin()+1, tokens.end()});

            keVal[0] *= LIGHT_INTENSITY_MULTIPLICATOR;
            keVal[1] *= LIGHT_INTENSITY_MULTIPLICATOR;
            keVal[2] *= LIGHT_INTENSITY_MULTIPLICATOR;

            res->back().Ke = keVal;
        };
        parseKx(tokens,lambda);
    } else if (specifier == "Ns") {
        // reflective exponent (1 to 1000)
        res->back().Ns = std::stof(tokens[1])/1000.0f;
    } else if (specifier == "d") {
        // transparency (0 = transparent, 1 = visible)
        res->back().d = 1.0f - std::stof(tokens[1]);
    } else if (specifier == "Tf") {
        // transparency color
        auto lambda = [res, this, tokens]() {
            res->back().Tf = parseFloatTriplet({tokens.begin()+1, tokens.end()});
        };
        parseKx(tokens,lambda);
    } else if (specifier == "Ni") {
        // refractive index
        res->back().Ni = std::stof(tokens[1]);
    } else if (specifier == "illum") {
        // illumination model (ignored in pathtracer)
        res->back().illum = static_cast<short>(std::stoi(tokens[1]));
    } else if (specifier == "Pm") {
        // metallicity
        res->back().Pm = std::stof(tokens[1]);
    } else if (specifier == "Pr") {
        // roughness
        res->back().Pr = std::stof(tokens[1]);
    } else if (specifier == "sss_radius") {
        // subsurface scattering radius
        res->back().sss_radius = std::stof(tokens[1]);
    } else if (specifier == "sss_intensity") {
        // subsurface scattering intensity
        res->back().sss_intensity = std::stof(tokens[1]);
    } else if (specifier == "map_Ka") {
        //res->back().map_Ka = parseMap<3>(tokens[1]);
    } else if (specifier == "map_Kd") {
        // texture map
        res->back().map_Kd = parseMap<3>(tokens.back(), textureData);
    } else if (specifier == "map_Ks") {
        //res->back().map_Ks = parseMap(tokens[1], 3);
    } else if (specifier == "map_Ke") {
        //res->back().map_Ke = parseMap(tokens[1], 3);
    } else if (specifier == "map_Ns") {
        //res->back().map_Ns = parseMap(tokens[1], 1);
    } else if (specifier == "map_d") {
        //res->back().map_d = parseMap(tokens[1], 1);
    } else if (specifier == "disp") {
        // diplacement map
        // TODO
    } else if (specifier == "map_bump" || specifier == "bump") {
        // bump map
        res->back().bump = parseMap<1>(tokens.back(), bumpData);
    } else if (specifier == "norm") {
        // normal map
        res->back().norm = parseMap<3>(tokens.back(), normalData);
    }
}
