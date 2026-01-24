// src/rendering/debug/DebugGizmoRenderer.h
#pragma once
#include <memory>
#include <vector>

namespace cozy::core
{
    class IShader;
    class ICamera;
}
namespace cozy::rendering
{
    class LightManager;
    // Changed to struct to match definitions and fix C4099
    struct DirectionalLight;
    struct PointLight;
}

namespace cozy::rendering::debug
{
    class DebugMesh;

    class DebugGizmoRenderer
    {
    public:
        DebugGizmoRenderer();
        ~DebugGizmoRenderer();

        void RenderLightGizmos(
            const LightManager &lights,
            const core::IShader &shader,
            const core::ICamera &camera);

        void SetEnabled(bool enabled) { m_enabled = enabled; }
        void SetDirectionalLightLength(float length) { m_dirLightLength = length; }
        void SetPointLightSize(float size) { m_pointLightSize = size; }

    private:
        void CreateDebugMeshes();

        // Removed core::ICamera from these helper methods
        void RenderDirectionalLight(const DirectionalLight &light, const core::IShader &shader);
        void RenderPointLight(const PointLight &light, const core::IShader &shader);

        bool m_enabled{true};
        float m_dirLightLength{10.0f};
        float m_pointLightSize{1.0f};

        std::unique_ptr<DebugMesh> m_sphereMesh;
        std::unique_ptr<DebugMesh> m_arrowMesh;
    };
}