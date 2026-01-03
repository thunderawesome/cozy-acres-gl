#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace cozy::core
{
    class ICamera;
    class IMesh;
    class IShader;
    class ITexture;
}

namespace cozy::rendering
{
    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

        /**
         * @brief Initialize the graphics context.
         * @param nativeWindowHandle The platform-specific window pointer (e.g., GLFWwindow*).
         */
        virtual void Initialize(void *nativeWindowHandle) = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        /**
         * @brief Updates the OpenGL viewport dimensions.
         */
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

        /**
         * @brief Renders a mesh using a specific shader and camera context.
         */
        virtual void DrawMesh(
            const core::IMesh &mesh,
            const core::IShader &shader,
            const glm::mat4 &modelMatrix,
            const core::ICamera &camera) = 0;

        /**
         * @brief Binds a texture to a specific GPU sampling slot.
         */
        virtual void BindTexture(const core::ITexture &texture, uint32_t slot = 0) = 0;
    };
}