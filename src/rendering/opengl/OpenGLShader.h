#pragma once
#include "core/graphics/IGpuResource.h"
#include <glad/glad.h>
#include <unordered_map>

namespace cozy::rendering
{
    class OpenGLShader : public core::IShader
    {
    public:
        OpenGLShader();
        OpenGLShader(const char *vertexSource, const char *fragmentSource);
        ~OpenGLShader() override;

        [[nodiscard]] uint32_t GetRendererID() const noexcept override { return m_id; }

        void Bind() const override;
        void Unbind() const override;
        void SetMat4(const std::string &name, const glm::mat4 &mat) const override;
        void SetVec3(const std::string &name, const glm::vec3 &val) const override;
        void SetInt(const std::string &name, int val) const override;
        void SetFloat(const std::string &name, float val) const override;
        void SetVec4(const std::string &name, const glm::vec4 &val) const override;

    private:
        void checkCompileErrors(uint32_t shader, const std::string &type);
        void checkLinkErrors(uint32_t program);

        GLint GetUniformLocation(const std::string &name) const;

        uint32_t m_id{0};

        // Performance optimization: cache uniform locations
        mutable std::unordered_map<std::string, GLint> m_uniformCache;
    };
}