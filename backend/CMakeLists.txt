cmake_minimum_required(VERSION 3.25)
project(royal-tracer)

set(CMAKE_CXX_STANDARD 23)


# Platform-specific settings
if(WIN32)
        # Windows-specific settings
        message("Running build on Windows")

        # Define OIDN root directory here; the user only needs to modify this line
        #set(OIDN_ROOT C:/oidn-2.1.0.x64.windows)
        if (DEFINED ENV{NIKLAS_BUILD})
                message("Running on Niklas' laptop...")
                set(OIDN_ROOT C:/Niklas/Studium/Informatik/WiSe2324/Softwareprojekt/oidn-2.1.0.x64.windows)

        elseif (DEFINED ENV{JOHANNES_BUILD})
                message("Running on Johannes machine")
                set(OIDN_ROOT C:/Users/mj0st/Downloads/oidn-2.1.0.x64.windows)
        elseif (DEFINED ENV{JOHANNES2_BUILD})
                message("Running on Johannes 2nd machine")
                set(OIDN_ROOT C:/Users/most/Downloads/oidn-2.1.0.x64.windows)
        elseif (DEFINED ENV{OLAFS})
                message("Running on Marcel machine")
                set(OIDN_ROOT C:/Users/olafs/Downloads/oidn-2.1.0.x64.windows/oidn-2.1.0.x64.windows)
        else()
                set(OIDN_ROOT C:/oidn-2.1.0.x64.windows)

        endif ()

        set(OpenImageDenoise_INCLUDE_DIRS ${OIDN_ROOT}/include)
        set(OpenImageDenoise_LIBRARIES ${OIDN_ROOT}/lib/OpenImageDenoise.lib)
        set(OpenImageDenoise_DIR ${OIDN_ROOT}/lib/cmake/OpenImageDenoise-2.1.0)
elseif(UNIX)
        message("Running build on UNIX")
        # Linux-specific settings
        if(DEFINED ENV{DOCKER_BUILD})
                message("Running inside a Docker build")
                set(OIDN_ROOT /base/oidn-2.1.0.x86_64.linux)

                set(ZLIB_INCLUDE_DIR /base/vcpkg/packages/zlib_x64-linux/include)

                set(OpenCL_ROOT /base/vcpkg/packages/opencl_x64-linux)
                set(OpenCL_INCLUDE_DIR /base/vcpkg/packages/opencl_x64-linux/include)
                set(OpenCL_LIBRARY /base/vcpkg/packages/opencl_x64-linux/lib/libOpenCL.a)

        elseif (DEFINED ENV{FLO_BUILD})
                message("Running on Flos machine")
                set(OIDN_ROOT /home/florian/thu/oidn-2.1.0.x86_64.linux)
        elseif (DEFINED ENV{NIKLAS_BUILD})
                message("Running on Niklas' Linux machine")
                set(OIDN_ROOT /home/kneeclass/Downloads/oidn-2.1.0.x86_64.linux)
        endif ()

        set(OpenImageDenoise_INCLUDE_DIRS ${OIDN_ROOT}/include)
        set(OpenImageDenoise_DIR ${OIDN_ROOT}/lib/cmake/OpenImageDenoise-2.1.0)
        set(OpenImageDenoise_LIBRARIES
                ${OIDN_ROOT}/lib/libOpenImageDenoise_core.so.2.1.0
                ${OIDN_ROOT}/lib/libOpenImageDenoise_device_cpu.so.2.1.0
                ${OIDN_ROOT}/lib/libOpenImageDenoise_device_cuda.so.2.1.0
                ${OIDN_ROOT}/lib/libOpenImageDenoise.so.2.1.0
        )
endif()

# packages for raytracing
find_package(OpenCL REQUIRED)
find_package(OpenImageDenoise REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)


#packages for the http-server
find_package(Crow)
find_package(nlohmann_json)

# Add the executable target
file(GLOB_RECURSE SOURCE_FILES RELATIVE "${PROJECT_SOURCE_DIR}/" "src/*.cpp" "src/*.h")
if (DEFINED ENV{TEST})
        message("Generating test build")
        list(REMOVE_ITEM SOURCE_FILES "src/main.cpp")
        file(GLOB_RECURSE TEST_FILES "test/*.cpp" "test/*.h")
        list(APPEND SOURCE_FILES ${TEST_FILES})
endif ()
add_executable(royal-tracer ${SOURCE_FILES})

# Set your own mingw path here!
#set(MINGW_PATH "C:/Program Files/JetBrains/CLion 2023.2.2/bin/mingw")
#set(STD_LIBS "libgcc_s_seh-1.dll" "libstdc++-6.dll" "libwinpthread-1.dll")
#foreach(STD_LIB ${STD_LIBS})
#        add_custom_command(TARGET royal-tracer POST_BUILD
#                COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                "${MINGW_PATH}/${STD_LIB}"
#                $<TARGET_FILE_DIR:royal-tracer>)
#endforeach()


target_include_directories(royal-tracer PRIVATE
        ${OpenCL_INCLUDE_DIRS}
        ${OpenImageDenoise_INCLUDE_DIRS}
)

if(WIN32)
        target_link_libraries(royal-tracer PUBLIC
                Crow::Crow
                wsock32
                ws2_32
                nlohmann_json::nlohmann_json
        )
elseif(UNIX)
        target_link_libraries(royal-tracer PUBLIC
                Crow::Crow
                nlohmann_json::nlohmann_json
        )
endif ()

target_link_libraries(royal-tracer PRIVATE
        PNG::PNG
        ${OpenCL_LIBRARIES}
        ${OpenImageDenoise_LIBRARIES}
)


# Copy kernel files to build directory
file(GLOB KERNEL_FILES ${CMAKE_SOURCE_DIR}/src/raytracer/kernelFiles/*)
file(COPY ${KERNEL_FILES} DESTINATION ${CMAKE_BINARY_DIR})

# Copy oidn dll files to build directory
file(GLOB DLL_FILES ${OIDN_ROOT}/bin/*.dll)
file(COPY ${DLL_FILES} DESTINATION ${CMAKE_BINARY_DIR})

#To support the execution of the program outside of the IDE, the mingw std dlls have to be present in the build directory. As the project uses the integrated
#mingw, it is not possible to simply copy the dlls to the build directory from the IDE directory. To address this issue, a separate directory was created to store the dlls.
file(GLOB DLL_FILES_STD ${CMAKE_SOURCE_DIR}/build_dlls/*.dll)
file(COPY ${DLL_FILES_STD} DESTINATION ${CMAKE_BINARY_DIR})
