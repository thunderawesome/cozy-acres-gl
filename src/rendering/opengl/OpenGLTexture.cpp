#include "OpenGLTexture.h"
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "embedded_texture.h"
#include <iostream>

namespace cozy::rendering
{
    OpenGLTexture::OpenGLTexture(const std::string &filename)
    {
        m_id = loadFromFile(filename);
        if (m_id == 0)
            m_id = loadFromMemory();
        if (m_id == 0)
            m_id = createWhiteFallback();
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
        uint32_t texID;
        int nrChannels;
        unsigned char *data = stbi_load(path.c_str(), &m_width, &m_height, &nrChannels, 0);
        if (!data)
            return 0;

        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        return texID;
    }

    uint32_t OpenGLTexture::loadFromMemory()
    {
        if (embedded_texture_size == 0)
            return 0;
        uint32_t texID;
        int nrChannels;
        unsigned char *data = stbi_load_from_memory(embedded_texture_data, (int)embedded_texture_size, &m_width, &m_height, &nrChannels, 0);
        if (!data)
            return 0;

        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexImage2D(GL_TEXTURE_2D, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB), m_width, m_height, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
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