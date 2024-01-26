# Testing the backend

The unit test framework in use is [Boost.Test](https://www.boost.org/doc/libs/1_83_0/libs/test/doc/html/index.html).

## Running the test build

To run the test build for the backend, you need to set the corresponding environment variable in CLion. Therefore, go to `File -> Settings -> Build, Execution, Deployment -> CMake`. 
There, in your selected profile, write "TEST=1" in the field called `Environment` (If other environment variables are set in that field, use a `;` to separate the new one from those). Apply those changes and CMake should load automatically.
In the CMake log, check if the message `"Generating test build"` is printed.
Then, click the run button, the automatic test validation should start.

## Writing tests

> Test everything that could possibly break.
> ~Maxim from Extreme Programming (XP)

This maxim should apply for this project too. Tests should be written for every component and every major subcomponent. The reason for writing tests is to check every "program path" the tested unit can take, so focus on that.
Instead of writing multiple tests with input that lead to the same program path, analyse the loops and conditional statements and write only one test per program path.

The test build is organized as a tree structure (see figure below), with each "category" we defined being a child node of root (this distinction is based on the folder structure of the project).
The categories are represented as "test suites" in the tree (see Glossary). For each class or subcategory, a new test suite should be defined and added, like it is shown in the code example below. 
These test suites contain all test cases for the functionality being tested, and each test case should make only one assertion.

![Tree structure](/docs/Documentation/media/TestBuild.svg)

```cpp
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

void init_utils_suite(test_suite* utils_suite) {
    auto* vecFuncsS = BOOST_TEST_SUITE("vectorFunctions_suite");
    test_vectorFunctions(vecFuncsS);
    utils_suite->add(vecFuncsS);
}
```
