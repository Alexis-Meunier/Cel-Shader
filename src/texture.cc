#include "texture.hh"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ImageInfo load_image(const std::string& filepath)
{
    vector<unsigned char> pixels;
    int width = 0, height = 0, channels = 0;

    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
    if (!data)
        throw std::runtime_error("ImageTexture: failed to load " + filepath);

    channels = 3;
    pixels.assign(data, data + width * height * 3);
    stbi_image_free(data);

    return { pixels, width, height, channels };
}

