//
// Created by kneeclass on 08.12.23.
//
#define LEXPP_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "../src/architecture/RoyalMediator.h"
#include "../src/api/WebEditorFacade.h"

#include "raytracer_tests.h"
#include "utils_tests.h"
#include "arch_tests.h"

using namespace boost::unit_test;

test_suite* init_unit_test_suite( int /*argc*/, char* /*argv*/[] )
{
    auto* raytracer = BOOST_TEST_SUITE("raytracer_suite" );
    init_raytracer_suite(raytracer);

    auto* utils = BOOST_TEST_SUITE("utils_suite" );
    init_utils_suite(utils);

    auto* arch = BOOST_TEST_SUITE( "architecture_suite" );
    init_arch_suite(arch);

    framework::master_test_suite().add(raytracer);
    framework::master_test_suite().add(utils);
    framework::master_test_suite().add(arch);

    return nullptr;
}
