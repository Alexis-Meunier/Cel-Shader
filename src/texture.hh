#pragma once

#include <vector>
#include <iostream>

using std::vector;

struct ImageInfo
{
    vector<int8_t> pixels;
    int width = 0;
    int height = 0;
    int channels = 0;
};

ImageInfo load_image(const std::string& filepath);
