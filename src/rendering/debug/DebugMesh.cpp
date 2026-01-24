#include "rendering/debug/DebugMesh.h"
#include <glad/glad.h>

namespace cozy::rendering::debug
{
    DebugMesh::DebugMesh(const std::vector<DebugVertex> &vertices)
        : m_vertexCount(static_cast<uint32_t>(vertices.size()))
    {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(DebugVertex),
                     vertices.data(),
                     GL_STATIC_DRAW);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              sizeof(DebugVertex), (void *)0);

        // Color
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              sizeof(DebugVertex),
                              (void *)offsetof(DebugVertex, color));

        glBindVertexArray(0);
    }

    DebugMesh::~DebugMesh()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
    }

    void DebugMesh::UpdateVertices(const std::vector<DebugVertex> &vertices)
    {
        m_vertexCount = static_cast<uint32_t>(vertices.size());
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(DebugVertex),
                     vertices.data(),
                     GL_STATIC_DRAW);
    }

    void DebugMesh::Bind() const
    {
        glBindVertexArray(m_vao);
    }

    void DebugMesh::Unbind() const
    {
        glBindVertexArray(0);
    }

    void DebugMesh::Draw() const
    {
        glBindVertexArray(m_vao);
        glDrawArrays(GL_LINES, 0, m_vertexCount); // or GL_TRIANGLES for spheres
        glBindVertexArray(0);
    }
}