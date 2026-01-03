#include "OpenGLInstancedMesh.h"
#include <glad/glad.h>

namespace cozy::rendering
{
    OpenGLInstancedMesh::OpenGLInstancedMesh(const float *vertices, size_t vertexCount)
        : m_VertexCount(vertexCount / 11)
    { // 11 floats per vertex

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_InstanceVBO);

        glBindVertexArray(m_VAO);

        // 1. Static Geometry (The Cube)
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);

        GLsizei stride = 11 * sizeof(float);
        glEnableVertexAttribArray(0); // Pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glEnableVertexAttribArray(1); // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2); // Tex
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));

        // 2. Dynamic Instance Data (Model Matrix and Color)
        glBindBuffer(GL_ARRAY_BUFFER, m_InstanceVBO);

        // Matrix takes 4 attribute slots (locations 3, 4, 5, 6)
        size_t instanceStride = sizeof(TileInstance);
        for (int i = 0; i < 4; i++)
        {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, instanceStride, (void *)(sizeof(float) * i * 4));
            glVertexAttribDivisor(3 + i, 1); // Tell OpenGL this updates per instance, not per vertex
        }

        // Color (location 7)
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, instanceStride, (void *)offsetof(TileInstance, color));
        glVertexAttribDivisor(7, 1);

        glBindVertexArray(0);
    }

    void OpenGLInstancedMesh::UpdateInstances(const std::vector<TileInstance> &instances)
    {
        m_InstanceCount = instances.size();
        glBindBuffer(GL_ARRAY_BUFFER, m_InstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(TileInstance), instances.data(), GL_DYNAMIC_DRAW);
    }

    void OpenGLInstancedMesh::Draw() const
    {
        if (m_InstanceCount == 0)
            return;
        glBindVertexArray(m_VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)m_VertexCount, (GLsizei)m_InstanceCount);
    }

    OpenGLInstancedMesh::~OpenGLInstancedMesh()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_InstanceVBO);
    }
}