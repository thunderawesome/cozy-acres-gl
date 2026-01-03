#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "Texture.h"
#include "embedded_texture.h" // generated header
#include <iostream>
#include <fstream>

Texture::Texture(const std::string &filename)
{
    // Try file first (modding support)
    ID = loadFromFile(filename);
    if (ID == 0)
    {
        std::cout << "File not found or failed, using embedded texture." << std::endl;
        ID = loadFromMemory();
    }
    if (ID == 0)
    {
        std::cerr << "Embedded texture failed, using white fallback." << std::endl;
        ID = createWhiteFallback();
    }
}

Texture::~Texture()
{
    if (ID != 0)
        glDeleteTextures(1, &ID);
}

void Texture::bind(int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);
}

unsigned int Texture::loadFromFile(const std::string &path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "Loaded texture from file: " << path << std::endl;
    }
    else
    {
        std::cerr << "Failed to load texture from file: " << path << " (" << stbi_failure_reason() << ")" << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }

    return textureID;
}

unsigned int Texture::loadFromMemory()
{
    if (embedded_texture_size == 0)
        return 0;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load_from_memory(
        embedded_texture_data, static_cast<int>(embedded_texture_size),
        &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "Loaded embedded texture." << std::endl;
    }
    else
    {
        std::cerr << "Failed to load embedded texture: " << stbi_failure_reason() << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }

    return textureID;
}

unsigned int Texture::createWhiteFallback()
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned char white[3] = {255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, white);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}