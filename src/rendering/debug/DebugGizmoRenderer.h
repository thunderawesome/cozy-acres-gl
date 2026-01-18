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
    class DirectionalLight;
    class PointLight;
}

namespace cozy::rendering::debug
{
    class DebugMesh;

    class DebugGizmoRenderer
    {
    public:
        DebugGizmoRenderer();
        ~DebugGizmoRenderer();

        // Render light gizmos
        void RenderLightGizmos(
            const LightManager &lights,
            const core::IShader &shader,
            const core::ICamera &camera);

        // Configuration
        void SetEnabled(bool enabled) { m_enabled = enabled; }
        void SetDirectionalLightLength(float length) { m_dirLightLength = length; }
        void SetPointLightSize(float size) { m_pointLightSize = size; }

    private:
        void CreateDebugMeshes();
        void RenderDirectionalLight(
            const DirectionalLight &light,
            const core::IShader &shader,
            const core::ICamera &camera);
        void RenderPointLight(
            const PointLight &light,
            const core::IShader &shader,
            const core::ICamera &camera);

        bool m_enabled{true};
        float m_dirLightLength{10.0f};
        float m_pointLightSize{1.0f};

        std::unique_ptr<DebugMesh> m_sphereMesh;
        std::unique_ptr<DebugMesh> m_arrowMesh;
    };
}