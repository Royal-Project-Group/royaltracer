//
// Created by stemp on 24.10.2023.
//

#include "PrecomputeLayer.h"
#include <iostream>
#include <cmath>

#include "../../utils/stb_image.h"

// The precompute layer is responsible for the matrix transformation of the triangles.
PrecomputeLayer::PrecomputeLayer() : precomputeKernelCommunicator() {
    this->precomputeKernelCommunicator.setUp("transformation.cl");
}


std::shared_ptr<Scene> PrecomputeLayer::operate(std::shared_ptr<Scene> scene) {

    int overallTriangleIndex = 0;
    // iterate over each object mesh
    for (int i = 0; i < scene->objects.size(); i++) {
        // input vector for kernel
        int id = scene->objects[i].id;
        std::vector<Triangle> input;

        // iterate over each triangle and build input vector for an object
        for (auto &triangle: scene->triangles) {

            if (triangle.objectID != id)
                continue;
            input.push_back(triangle);
        }

        // build translation matrix
        cl_float3 position = scene->objects[i].position;
        cl_float3 rotation = scene->objects[i].rotation;
        cl_float3 scale = scene->objects[i].scale;

        cl_float16 rotationMat = MathUtils::identiyMatrix();

        rotationMat = MathUtils::rotateX(rotationMat, rotation.x);
        rotationMat = MathUtils::rotateY(rotationMat, rotation.y);
        rotationMat = MathUtils::rotateZ(rotationMat, rotation.z);

        cl_float16 scaleMat = MathUtils::identiyMatrix();
        scaleMat = MathUtils::scale(scaleMat, scale.x, scale.y, scale.z);

        cl_float16 translationMat = MathUtils::identiyMatrix();
        translationMat = MathUtils::translate(translationMat, position.x, position.y, position.z);

        cl_float16 transformationMat = MathUtils::multiply( rotationMat,scaleMat);
        transformationMat = MathUtils::multiply(translationMat, transformationMat);

        std::vector<Triangle> output = std::vector<Triangle>(input.size());

        // delegate to GPU and transform triangles (fast)
        this->precomputeKernelCommunicator.delegateGPU(input, transformationMat, output);

        for (int j = 0; j < output.size(); j++) {

            scene->triangles[overallTriangleIndex] = output[j];

            overallTriangleIndex += 1;
        }

    }


    return scene;
}
