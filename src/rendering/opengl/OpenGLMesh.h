#pragma once

#include "core/graphics/IGpuResource.h"
#include <cstdint>

namespace cozy::rendering
{
    /**
     * @brief OpenGL implementation of a static mesh.
     * Currently defaults to a Cube primitive for testing.
     */
    class OpenGLMesh final : public core::IMesh
    {
    public:
        OpenGLMesh();
        ~OpenGLMesh() override;

        // Prevent copying to avoid accidental GPU resource deletion
        OpenGLMesh(const OpenGLMesh &) = delete;
        OpenGLMesh &operator=(const OpenGLMesh &) = delete;

        void Bind() const;
        void Unbind() const;

        [[nodiscard]] uint32_t GetRendererID() const noexcept override { return m_vao; }

        // 36 vertices (6 faces * 2 triangles * 3 vertices)
        [[nodiscard]] uint32_t GetVertexCount() const noexcept override { return 36; }

        // Currently using glDrawArrays, so index count matches vertex count
        [[nodiscard]] uint32_t GetIndexCount() const noexcept override { return 36; }

    private:
        uint32_t m_vao{0};
        uint32_t m_vbo{0};
    };
}