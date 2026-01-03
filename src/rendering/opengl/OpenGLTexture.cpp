#include "OpenGLTexture.h"
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "embedded_textures.h"
#include <iostream>

namespace cozy::rendering
{
    OpenGLTexture::OpenGLTexture(const std::string &nameOrPath)
    {
        // 1. Try Loading from Disk
        m_id = loadFromFile(nameOrPath);

        // 2. Try Loading from Embedded Registry
        if (m_id == 0)
            m_id = loadFromRegistry(nameOrPath);

        // 3. Last Resort: White Pixel
        if (m_id == 0)
        {
            std::cerr << "[Texture] Failed to load: " << nameOrPath << ". Using fallback." << std::endl;
            m_id = createWhiteFallback();
        }
    }

    OpenGLTexture::~OpenGLTexture()
    {
        if (m_id != 0)
            glDeleteTextures(1, &m_id);
    }

    void OpenGLTexture::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    uint32_t OpenGLTexture::loadFromFile(const std::string &path)
    {
        int channels;
        unsigned char *data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 0);
        if (!data)
            return 0;

        uint32_t texID = createTexture(data, m_width, m_height, channels);
        stbi_image_free(data);
        return texID;
    }

    uint32_t OpenGLTexture::loadFromRegistry(const std::string &name)
    {
        // Strip extension if provided (e.g., "grass.png" -> "grass")
        std::string key = name;
        size_t lastdot = key.find_last_of(".");
        if (lastdot != std::string::npos)
            key = key.substr(0, lastdot);

        auto it = g_EmbeddedTextures.find(key);
        if (it == g_EmbeddedTextures.end())
            return 0;

        const auto &embedded = it->second;
        int channels;
        unsigned char *data = stbi_load_from_memory(embedded.data, (int)embedded.size, &m_width, &m_height, &channels, 0);
        if (!data)
            return 0;

        uint32_t texID = createTexture(data, m_width, m_height, channels);
        stbi_image_free(data);
        return texID;
    }

    uint32_t OpenGLTexture::createTexture(unsigned char *data, int width, int height, int channels)
    {
        uint32_t texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        // Animal Crossing Style: We might want Pixel Art look? Use GL_NEAREST if so.
        // For now, staying with Linear for smooth cozy look.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D);
        return texID;
    }

    uint32_t OpenGLTexture::createWhiteFallback()
    {
        uint32_t texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        unsigned char white[3] = {255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, white);
        m_width = m_height = 1;
        return texID;
    }
}