#include "OpenGLMesh.h"
#include "PrimitiveData.h"
#include <glad/glad.h>

namespace cozy::rendering
{
    OpenGLMesh::OpenGLMesh()
    {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        // Use the refactored primitive data
        glBufferData(GL_ARRAY_BUFFER, sizeof(primitives::CubeVertices), primitives::CubeVertices, GL_STATIC_DRAW);

        // Attribute layout matches your 11-float stride
        // Pos(3) + Normal(3) + Tex(2) + Color(3) = 11 floats
        GLsizei stride = 11 * sizeof(float);

        // 0: Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glEnableVertexAttribArray(0);

        // 1: Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // 2: TexCoords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // 3: Color
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void *)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glBindVertexArray(0);
    }

    OpenGLMesh::~OpenGLMesh()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
    }

    void OpenGLMesh::Bind() const
    {
        glBindVertexArray(m_vao);
    }

    void OpenGLMesh::Unbind() const
    {
        glBindVertexArray(0);
    }
}