#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>

namespace cozy::core
{
    /**
     * @brief Base class for any resource that lives on the GPU (VAO, Texture, Shader)
     */
    class IGpuResource
    {
    public:
        virtual ~IGpuResource() = default;

        // Returns the underlying API handle (e.g., OpenGL ID)
        [[nodiscard]] virtual uint32_t GetRendererID() const noexcept = 0;
    };

    /**
     * @brief Interface for geometric data
     */
    class IMesh : public IGpuResource
    {
    public:
        [[nodiscard]] virtual uint32_t GetVertexCount() const noexcept = 0;
        [[nodiscard]] virtual uint32_t GetIndexCount() const noexcept = 0;
    };

    /**
     * @brief Interface for GPU programs and uniform management
     */
    class IShader : public IGpuResource
    {
    public:
        virtual void SetMat4(const std::string &name, const glm::mat4 &mat) const = 0;
        virtual void SetVec3(const std::string &name, const glm::vec3 &val) const = 0;
        virtual void SetInt(const std::string &name, int val) const = 0;

        // Helper for activating the shader
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
    };

    /**
     * @brief Interface for image data on the GPU
     */
    class ITexture : public IGpuResource
    {
    public:
        [[nodiscard]] virtual uint32_t GetWidth() const = 0;
        [[nodiscard]] virtual uint32_t GetHeight() const = 0;

        virtual void Bind(uint32_t slot = 0) const = 0;
    };
}