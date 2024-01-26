//
// Created by nikla on 24.10.2023.
//

#ifndef ROYAL_TRACER_CROSSPRODUCT_H
#define ROYAL_TRACER_CROSSPRODUCT_H

#include <array>
#include <cmath>
#include <memory>
#include <numeric>
#include <algorithm>

template<typename T, std::size_t Dimension>
std::array<T, Dimension>
crossProduct(const std::array<T, Dimension> &vector1, const std::array<T, Dimension> &vector2) {
    std::array<T, Dimension> result;

    for (std::size_t i = 0; i < Dimension; ++i) {
        std::size_t nextIndex = (i + 1) % Dimension;
        std::size_t prevIndex = (i + Dimension - 1) % Dimension;

        result[i] = vector1[nextIndex] * vector2[prevIndex] - vector1[prevIndex] * vector2[nextIndex];
    }

    return result;
}

template<typename T, std::size_t Dimension>
std::array<T, Dimension>
subtractVectors(const std::array<T, Dimension> &vector1, const std::array<T, Dimension> &vector2) {
    std::array<T, Dimension> result;

    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = vector1[i] - vector2[i];
    }

    return result;
}

template<typename T, std::size_t Dimension>
std::array<T, Dimension> addVectors(const std::array<T, Dimension> &vector1, const std::array<T, Dimension> &vector2) {
    std::array<T, Dimension> result;

    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = vector1[i] + vector2[i];
    }

    return result;
}

template<typename T, std::size_t Dimension>
float euclideanDistance(const std::array<T, Dimension> &vec) {
    return std::sqrt(std::accumulate(vec.begin(), vec.end(), 0.0f, [](float acc, float value) {
        return acc + value * value;
    }));
}

template<typename T, std::size_t Dim>
std::array<T, Dim> normalize(const std::array<T, Dim> &vec) {
    std::shared_ptr<std::array<T, Dim>> res = std::make_shared<std::array<T, Dim>>();
    const float distance = euclideanDistance(vec);

    for (int i = 0; i < vec.size(); i++) {
        (*res)[i] = vec[i] / distance;
    }
    return *res;
}

#endif //ROYAL_TRACER_CROSSPRODUCT_H
