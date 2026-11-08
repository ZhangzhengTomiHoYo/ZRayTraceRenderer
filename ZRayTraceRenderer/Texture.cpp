#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture* Texture::CreateFromFile(const std::string& filePath)
{
    int w, h, channel;
    uint8_t* data = stbi_load(filePath.c_str(), &w, &h, &channel, 0);
    if (data == nullptr)
    {
        return nullptr;
    }
    Texture* tex = new Texture(w, h, channel, data);
    stbi_image_free(data);

    return tex;
}
