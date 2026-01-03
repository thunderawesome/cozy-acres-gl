#pragma once
#include "core/graphics/IGpuResource.h"
#include "rendering/InstanceData.h"
#include <vector>

namespace cozy::rendering
{
    class OpenGLInstancedMesh
    {
    public:
        OpenGLInstancedMesh(const float *vertices, size_t vertexCount);
        ~OpenGLInstancedMesh();

        void UpdateInstances(const std::vector<TileInstance> &instances);
        void Draw() const;

    private:
        uint32_t m_VAO, m_VBO, m_InstanceVBO;
        size_t m_VertexCount;
        size_t m_InstanceCount = 0;
    };
}