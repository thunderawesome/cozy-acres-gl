#pragma once
#include <glad/glad.h>

class Mesh
{
public:
    Mesh();
    ~Mesh();

    void draw() const;

private:
    unsigned int VAO, VBO;
};