#include "rendering/LightManager.h"
#include "core/graphics/IGpuResource.h"
#include <algorithm>

namespace cozy::rendering
{
    LightManager::LightManager()
    {
        // Default directional light (sun from above-right)
        m_directionalLight.direction = glm::vec3(-0.5f, -1.0f, -0.3f);
        m_directionalLight.color = glm::vec3(1.0f, 0.98f, 0.95f); // Warm sunlight
        m_directionalLight.ambient = 0.2f;
        m_directionalLight.diffuse = 0.8f;
        m_directionalLight.specular = 0.3f;
    }

    void LightManager::SetDirectionalLight(const DirectionalLight &light)
    {
        m_directionalLight = light;
    }

    void LightManager::AddPointLight(const PointLight &light)
    {
        if (m_pointLights.size() < MAX_POINT_LIGHTS)
        {
            m_pointLights.push_back(light);
        }
    }

    void LightManager::ClearPointLights()
    {
        m_pointLights.clear();
    }

    void LightManager::RemovePointLight(size_t index)
    {
        if (index < m_pointLights.size())
        {
            m_pointLights.erase(m_pointLights.begin() + index);
        }
    }

    void LightManager::ApplyToShader(const core::IShader &shader, const glm::vec3 &viewPos) const
    {
        // Bind camera position for specular calculations
        shader.SetVec3("u_ViewPos", viewPos);

        // Bind directional light
        shader.SetVec3("u_DirLight.direction", m_directionalLight.direction);
        shader.SetVec3("u_DirLight.color", m_directionalLight.color);
        shader.SetFloat("u_DirLight.ambient", m_directionalLight.ambient);
        shader.SetFloat("u_DirLight.diffuse", m_directionalLight.diffuse);
        shader.SetFloat("u_DirLight.specular", m_directionalLight.specular);

        // Bind point lights
        int numLights = static_cast<int>(std::min(m_pointLights.size(), MAX_POINT_LIGHTS));
        shader.SetInt("u_NumPointLights", numLights);

        for (int i = 0; i < numLights; ++i)
        {
            std::string base = "u_PointLights[" + std::to_string(i) + "]";
            const PointLight &light = m_pointLights[i];

            shader.SetVec3(base + ".position", light.position);
            shader.SetVec3(base + ".color", light.color);
            shader.SetFloat(base + ".constant", light.constant);
            shader.SetFloat(base + ".linear", light.linear);
            shader.SetFloat(base + ".quadratic", light.quadratic);
        }
    }
}