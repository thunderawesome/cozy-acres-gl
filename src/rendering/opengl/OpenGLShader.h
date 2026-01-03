#pragma once

#include "core/graphics/IGpuResource.h"
#include <string>
#include <glm/glm.hpp>

namespace cozy::rendering
{
    class OpenGLShader final : public core::IShader
    {
    public:
        // Default constructor uses embedded shaders
        OpenGLShader();
        OpenGLShader(const char *vertexSource, const char *fragmentSource);
        ~OpenGLShader() override;

        void Bind() const override;
        void Unbind() const override;

        // Uniform Setters
        void SetMat4(const std::string &name, const glm::mat4 &mat) const override;
        void SetVec3(const std::string &name, const glm::vec3 &val) const override;
        void SetInt(const std::string &name, int val) const override;

        [[nodiscard]] uint32_t GetRendererID() const noexcept override { return m_id; }

    private:
        uint32_t m_id{0};

        void checkCompileErrors(uint32_t shader, const std::string &type);
        void checkLinkErrors(uint32_t program);
    };
}