#pragma once

#include <glad/glad.h>
#include <string>

class Texture
{
public:
    unsigned int ID = 0;

    Texture(const std::string &filename = "placeholder.jpg"); // default filename
    ~Texture();

    void bind(int slot = 0) const;

private:
    unsigned int loadFromFile(const std::string &path);
    unsigned int loadFromMemory();
    unsigned int createWhiteFallback();
};