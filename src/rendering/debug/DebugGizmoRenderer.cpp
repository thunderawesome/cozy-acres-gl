#include "rendering/debug/DebugGizmoRenderer.h"
#include "rendering/debug/DebugMesh.h"
#include "rendering/debug/DebugPrimitives.h"
#include "rendering/LightManager.h"
#include "rendering/Light.h"
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
        auto sphereVerts = CreateIcosphere(1.0f, glm::vec3(1.0f, 0.7f, 0.3f));
        m_sphereMesh = std::make_unique<DebugMesh>(sphereVerts);

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

        // TODO: Pass aspect ratio from outside
        float aspect = 1280.0f / 720.0f;
        shader.SetMat4("u_View", camera.GetViewMatrix());
        shader.SetMat4("u_Projection", camera.GetProjectionMatrix(aspect));

        RenderDirectionalLight(lights.GetDirectionalLight(), shader);

        for (const auto &light : lights.GetPointLights())
        {
            RenderPointLight(light, shader);
        }
    }

    void DebugGizmoRenderer::RenderDirectionalLight(
        const DirectionalLight &light,
        const core::IShader &shader)
    {
        glm::vec3 arrowPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 lightDir = glm::normalize(light.direction);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), arrowPos);

        glm::vec3 defaultDir = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 rotationAxis = glm::cross(defaultDir, lightDir);

        if (glm::length(rotationAxis) > 0.001f)
        {
            rotationAxis = glm::normalize(rotationAxis);
            float angle = std::acos(glm::dot(defaultDir, lightDir));
            model = glm::rotate(model, angle, rotationAxis);
        }

        model = glm::scale(model, glm::vec3(m_dirLightLength));
        shader.SetMat4("u_Model", model);

        m_arrowMesh->Bind();
        glDrawArrays(GL_LINES, 0, m_arrowMesh->GetVertexCount());
    }

    void DebugGizmoRenderer::RenderPointLight(
        const PointLight &light,
        const core::IShader &shader)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), light.position);
        model = glm::scale(model, glm::vec3(m_pointLightSize));

        shader.SetMat4("u_Model", model);

        m_sphereMesh->Bind();
        glDrawArrays(GL_TRIANGLES, 0, m_sphereMesh->GetVertexCount());
    }
}