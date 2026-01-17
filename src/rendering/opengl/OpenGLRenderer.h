#pragma once
#include "rendering/IRenderer.h"

namespace cozy::rendering
{
    class OpenGLRenderer : public IRenderer
    {
    public:
        OpenGLRenderer() = default;
        ~OpenGLRenderer() override = default;

        void Initialize(void *nativeWindowHandle) override;
        void BeginFrame() override;
        void EndFrame() override;
        void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

        void DrawMesh(
            const core::IMesh &mesh,
            const core::IShader &shader,
            const glm::mat4 &modelMatrix,
            const core::ICamera &camera) override;

        void DrawInstanced(
            const OpenGLInstancedMesh &mesh,
            const core::IShader &shader,
            const core::ICamera &camera,
            const LightManager *lights = nullptr) override;

        void BindTexture(const core::ITexture &texture, uint32_t slot = 0) override;
    };
}