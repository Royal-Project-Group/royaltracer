//
// Created by kneeclass on 08.12.23.
//

#ifndef ROYAL_TRACER_UTILS_TESTS_H
#define ROYAL_TRACER_UTILS_TESTS_H

#include <boost/math/special_functions/relative_difference.hpp>
#include <boost/test/included/unit_test.hpp>
#include "../src/utils/vectorFunctions.h"

using namespace boost::unit_test;

void test_vectorFunctions(test_suite* suite) {
    suite->add(BOOST_TEST_CASE(([&] {
        // define input
        std::array<float, 3> input = {0.0f,0.0f,0.0f};
        // get output
        float dist = euclideanDistance(input);
        // define oracle
        float oracle = 0.0f;
        // special: define epsilon for floating point equality
        float epsilon = 1e-5;
        BOOST_TEST(boost::math::relative_difference(dist, oracle) < epsilon);
    })));
}



void test_materialParser(test_suite* suite){
    suite->add(BOOST_TEST_CASE(([&]{
        MTLParser parser;

        // Test getDefaultMaterial
        {
            Material defaultMaterial = parser.getDefaultMaterial();
            BOOST_TEST(defaultMaterial.name == "default");
            BOOST_TEST(defaultMaterial.Ka == std::array<float, 3>{0, 0, 0});
            BOOST_TEST(defaultMaterial.Kd == std::array<float, 3>{.9f, .9f, .9f});
            BOOST_TEST(defaultMaterial.Ks == std::array<float, 3>{.0f, .0f, .0f});
            BOOST_TEST(defaultMaterial.Tf == std::array<float, 3>{1.0000, 1.0000, 1.0000});
            BOOST_TEST(defaultMaterial.Ns == 1);
            BOOST_TEST(defaultMaterial.Ni == 1);
        }

        // Test parseFloatTriplet
        {
            std::vector<std::string> triplet = {"0.1", "0.2", "0.3"};
            float3 values = parser.parseFloatTriplet(triplet);
            BOOST_TEST(values.x == 0.1f);
            BOOST_TEST(values.y == 0.2f);
            BOOST_TEST(values.z == 0.3f);
        }

        // Test parseKx with "Ka" specifier
        {
            std::vector<std::string> tokens = {"Ka", "0.1", "0.2", "0.3"};
            auto materials = std::make_shared<std::vector<Material>>();
            materials->emplace_back(parser.getDefaultMaterial());
            auto lambda = [&materials, &parser, tokens]() {
                materials->back().Ka = parser.parseFloatTriplet({tokens.begin()+1, tokens.end()});
            };
            parser.parseKx(tokens, lambda);
            BOOST_TEST(materials->back().Ka == std::array<float, 3>{0.1f, 0.2f, 0.3f});
        }


        // Test parse method with a simple MTL content
        {
            std::string content = "newmtl Material1\nKa 0.1 0.2 0.3\nKd 0.4 0.5 0.6\n";
            auto materials = parser.parse(content);
            BOOST_TEST(materials->size() == 1);
            BOOST_TEST(materials->back().name == "Material1");
            BOOST_TEST(materials->back().Ka == std::array<float, 3>{0.1f, 0.2f, 0.3f});
            BOOST_TEST(materials->back().Kd == std::array<float, 3>{0.4f, 0.5f, 0.6f});
        }

    })));
}


void init_utils_suite(test_suite* utils_suite) {
    auto* vecFuncsS = BOOST_TEST_SUITE("vectorFunctions_suite");
    auto* matParsS = BOOST_TEST_SUITE("materialParse_suite");
    test_vectorFunctions(vecFuncsS);
    test_materialParser(matParsS);
    utils_suite->add(vecFuncsS);
    utils_suite->add(matParsS);
}
#endif //ROYAL_TRACER_UTILS_TESTS_H
