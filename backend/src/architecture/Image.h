//
// Created by most on 14.11.2023.
//

#ifndef ROYAL_TRACER_IMAGE_H
#define ROYAL_TRACER_IMAGE_H

#include <vector>
#include <sstream>
#include <utility>

class Image {
    std::vector<std::vector<std::vector<float>>> pixels;
public:
    Image(std::vector<std::vector<std::vector<float>>>);
    std::string getAsPPM() const;
};

#endif //ROYAL_TRACER_IMAGE_H
