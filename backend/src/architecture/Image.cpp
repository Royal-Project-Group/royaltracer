//
// Created by mj0st on 19.11.2023.
//
#include "Image.h"

Image::Image(std::vector<std::vector<std::vector<float>>> pixels):pixels(std::move(pixels)) {}

std::string Image::getAsPPM() const {
    std::ostringstream oss;

    oss << "P3" << std::endl;
    oss << pixels[0].size() << " " << pixels.size() << std::endl;
    oss << "255" << std::endl;

    for(const auto& row : pixels) {
        for(const auto& pixel : row) {
            oss << static_cast<int>(pixel[0]) << " " << static_cast<int>(pixel[1]) << " " << static_cast<int>(pixel[2]) << std::endl;
        }
    }
    return oss.str();
}

// This code is initially from the ImageTools class but was neither ever used nor necessary...
// In case it will be needed at some point in the future it is saved here, as it would belong in this class.
/*
void ImageTools::exportPNG(const std::string &filePath, const std::vector<std::vector<cl_float3>> &image) {
    FILE *fp = fopen(filePath.c_str(), "wb");
    if (!fp) {
        fprintf(stderr, "Could not open file %s for writing\n", filePath.c_str());
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        fprintf(stderr, "Could not allocate write struct\n");
        fclose(fp);
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Could not allocate info struct\n");
        png_destroy_write_struct(&png_ptr, nullptr);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error during png creation\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, image[0].size(), image.size(), 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    for (int y = 0; y < image.size(); y++) {
        std::vector<png_byte> rowData;
        for (int x = 0; x < image[0].size(); x++) {
            // Clamp pixel values and convert to byte
            rowData.push_back(static_cast<png_byte>(std::clamp(image[y][x].x, 0.0f, 255.0f)));
            rowData.push_back(static_cast<png_byte>(std::clamp(image[y][x].y, 0.0f, 255.0f)));
            rowData.push_back(static_cast<png_byte>(std::clamp(image[y][x].z, 0.0f, 255.0f)));
        }
        png_write_row(png_ptr, rowData.data());
    }

    png_write_end(png_ptr, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);
}

std::vector<std::vector<cl_float3>>
ImageTools::antiAliasImage(const std::vector<std::vector<cl_float3>> &pixels, int factor) {
    // Extract the height and width from the pixel matrix size
    int originalHeight = pixels.size() / factor;
    int originalWidth = pixels.empty() ? 0 : pixels[0].size() / factor;

    std::vector<std::vector<cl_float3>> aliasedMatrix(originalHeight, std::vector<cl_float3>(originalWidth));

    for (int y = 0; y < originalHeight; y++) {
        for (int x = 0; x < originalWidth; x++) {
            cl_float3 sum = {0.0f, 0.0f, 0.0f};
            for (int dy = 0; dy < factor; dy++) {
                for (int dx = 0; dx < factor; dx++) {
                    sum.x += pixels[y * factor + dy][x * factor + dx].x;
                    sum.y += pixels[y * factor + dy][x * factor + dx].y;
                    sum.z += pixels[y * factor + dy][x * factor + dx].z;
                }
            }
            float divisor = static_cast<float>(factor * factor);
            aliasedMatrix[y][x].x = sum.x / divisor;
            aliasedMatrix[y][x].y = sum.y / divisor;
            aliasedMatrix[y][x].z = sum.z / divisor;
        }
    }
    return aliasedMatrix;
}
*/