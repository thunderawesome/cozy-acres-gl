#include "rendering/debug/DebugGizmoRenderer.h"
#include "rendering/debug/DebugMesh.h"
#include "rendering/debug/DebugPrimitives.h"
#include "rendering/LightManager.h"
#include "core/graphics/IGpuResource.h"
#include "core/camera/ICamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

namespace cozy::rendering::debug
{
    DebugGizmoRenderer::DebugGizmoRenderer()
    {
        CreateDebugMeshes();
    }

    DebugGizmoRenderer::~DebugGizmoRenderer() = default;

    void DebugGizmoRenderer::CreateDebugMeshes()
    {
        // Create sphere for point lights (warm orange color)
        auto sphereVerts = CreateIcosphere(1.0f, glm::vec3(1.0f, 0.7f, 0.3f));
        m_sphereMesh = std::make_unique<DebugMesh>(sphereVerts);

        // Create arrow for directional lights (yellow color)
        auto arrowVerts = CreateArrow(glm::vec3(1.0f, 1.0f, 0.3f));
        m_arrowMesh = std::make_unique<DebugMesh>(arrowVerts);
    }

    void DebugGizmoRenderer::RenderLightGizmos(
        const LightManager &lights,
        const core::IShader &shader,
        const core::ICamera &camera)
    {
        if (!m_enabled)
            return;

        shader.Bind();

        // Set camera matrices
        float aspect = 1280.0f / 720.0f;
        shader.SetMat4("u_View", camera.GetViewMatrix());
        shader.SetMat4("u_Projection", camera.GetProjectionMatrix(aspect));

        // Render directional light
        RenderDirectionalLight(lights.GetDirectionalLight(), shader, camera);

        // Render point lights
        for (const auto &light : lights.GetPointLights())
        {
            RenderPointLight(light, shader, camera);
        }
    }

    void DebugGizmoRenderer::RenderDirectionalLight(
        const DirectionalLight &light,
        const core::IShader &shader,
        const core::ICamera &camera)
    {
        // OPTION 1: Place arrow at world origin (0, 0, 0) pointing in light direction
        glm::vec3 arrowPos = glm::vec3(0.0f, 0.0f, 0.0f);

        // OPTION 2: Place arrow at a fixed visible location in your scene
        // glm::vec3 arrowPos = glm::vec3(40.0f, 50.0f, 40.0f);  // Above town center

        // Calculate rotation to point arrow in light direction
        glm::vec3 lightDir = glm::normalize(light.direction);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // Build model matrix
        glm::mat4 model = glm::translate(glm::mat4(1.0f), arrowPos);

        // Rotate arrow to point in light direction
        // Arrow points down (-Y) by default, light.direction points toward surface
        // So we want to align -Y axis with light.direction
        glm::vec3 defaultDir = glm::vec3(0.0f, -1.0f, 0.0f);

        // Calculate rotation axis and angle
        glm::vec3 rotationAxis = glm::cross(defaultDir, lightDir);
        if (glm::length(rotationAxis) > 0.001f) // Avoid division by zero
        {
            rotationAxis = glm::normalize(rotationAxis);
            float angle = std::acos(glm::dot(defaultDir, lightDir));
            model = glm::rotate(model, angle, rotationAxis);
        }

        // Scale arrow
        model = glm::scale(model, glm::vec3(m_dirLightLength));

        shader.SetMat4("u_Model", model);

        m_arrowMesh->Bind();
        glDrawArrays(GL_LINES, 0, m_arrowMesh->GetVertexCount());
    }

    void DebugGizmoRenderer::RenderPointLight(
        const PointLight &light,
        const core::IShader &shader,
        const core::ICamera &camera)
    {
        // Point light visualization: sphere at the light's position
        glm::mat4 model = glm::translate(glm::mat4(1.0f), light.position);
        model = glm::scale(model, glm::vec3(m_pointLightSize));

        shader.SetMat4("u_Model", model);

        m_sphereMesh->Bind();
        glDrawArrays(GL_TRIANGLES, 0, m_sphereMesh->GetVertexCount());
    }
}