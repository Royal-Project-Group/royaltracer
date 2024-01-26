#ifndef OBJPARSER
#define OBJPARSER

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <numeric>
#include <filesystem>

#include "../architecture/Parser.h"
#include "ThreeDObject.hpp"
#include "ParsedTriangle.h"
#include "SimpleTriangle.h"
#include "vectorFunctions.h"
#include "MTLParser.h"
#include "../architecture/MtlStorageSystem.h"

class OBJParser : virtual Parser<ThreeDObject> {
    std::vector<std::array<float, 3>> vertices;
    std::vector<std::array<float, 3>> txt_coords;
    std::vector<std::array<float, 3>> normals;
    std::vector<Material> materials;
    int id;
    int temp_materialID;
    int offset;

    void determineElement(std::string line, std::shared_ptr<ThreeDObject> obj) override;

    // converts a deque of strings into an array of floats
    template<std::size_t NumParams>
    static std::array<float, NumParams> parseVertexData(std::vector<std::string> tokens) {
        std::array<float, NumParams> res{};
        // tokens.size() - 1, because we want to exclude the first param)
        std::size_t numEls = std::min(NumParams, tokens.size() - 1);
        std::transform(std::next(tokens.begin()), std::next(tokens.begin(), numEls + 1),
                       res.begin(), [](const std::string &par) {
                    return std::stof(par);
                });

        return res;
    }

    // creates a triangle from the previously gathered vertices, normals and txt_coordinates
    ParsedTriangle parseFace(const std::vector<std::string> &tokens);

    static void checkNormalsOnTriangle(std::shared_ptr<ParsedTriangle> t);

    static SimpleTriangle simplifyTriangle(const ParsedTriangle &parsed_t);

    static bool isEqual(float x, float y);

    std::vector<std::string> split(const std::string &s, char delim);

    std::shared_ptr<ThreeDObject> parse(const std::string& content) override;
public:
    // parses a .obj-file to a ThreeDObject
    std::shared_ptr<ThreeDObject> parse(const std::string& content, const int &id, const int &offset);
};

#endif