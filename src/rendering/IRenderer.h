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
    class OpenGLInstancedMesh;
    class LightManager;

    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

        virtual void Initialize(void *nativeWindowHandle) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

        virtual void DrawMesh(
            const core::IMesh &mesh,
            const core::IShader &shader,
            const glm::mat4 &modelMatrix,
            const core::ICamera &camera) = 0;

        // Added optional lights parameter
        virtual void DrawInstanced(
            const OpenGLInstancedMesh &mesh,
            const core::IShader &shader,
            const core::ICamera &camera,
            const LightManager *lights = nullptr) = 0;

        virtual void BindTexture(const core::ITexture &texture, uint32_t slot = 0) = 0;
    };
}