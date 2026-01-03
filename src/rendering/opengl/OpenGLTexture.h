#pragma once

#include "core/graphics/IGpuResource.h" // Ensure this defines ITexture
#include <string>
#include <cstdint>

namespace cozy::rendering
{
    class OpenGLTexture final : public core::ITexture
    {
    public:
        // Default to "placeholder" as a key for our registry
        explicit OpenGLTexture(const std::string &nameOrPath = "placeholder");
        ~OpenGLTexture() override;

        [[nodiscard]] uint32_t GetRendererID() const noexcept override { return m_id; }
        [[nodiscard]] uint32_t GetWidth() const override { return m_width; }
        [[nodiscard]] uint32_t GetHeight() const override { return m_height; }

        void Bind(uint32_t slot = 0) const override;

    private:
        uint32_t m_id{0};
        int m_width{0}, m_height{0};

        uint32_t loadFromFile(const std::string &path);
        uint32_t loadFromRegistry(const std::string &name);
        uint32_t createWhiteFallback();

        // Internal helper to avoid DRY (Don't Repeat Yourself) for GL setup
        uint32_t createTexture(unsigned char *data, int width, int height, int channels);
    };
}