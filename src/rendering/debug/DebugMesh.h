#pragma once
#include "core/graphics/IGpuResource.h"
#include "DebugPrimitives.h"
#include <vector>

namespace cozy::rendering::debug
{
    class DebugMesh : public core::IGpuResource
    {
    public:
        DebugMesh(const std::vector<DebugVertex> &vertices);
        ~DebugMesh() override;

        void UpdateVertices(const std::vector<DebugVertex> &vertices);
        void Draw() const;
        void Bind() const;
        void Unbind() const;

        [[nodiscard]] uint32_t GetRendererID() const noexcept override { return m_vao; }
        [[nodiscard]] uint32_t GetVertexCount() const noexcept { return m_vertexCount; }

    private:
        uint32_t m_vao{0};
        uint32_t m_vbo{0};
        uint32_t m_vertexCount{0};
    };
}