#pragma once
#include "rendering/Light.h"
#include <vector>
#include <glm/glm.hpp>

namespace cozy::core
{
    class IShader;
}

namespace cozy::rendering
{
    /**
     * @brief Manages all scene lights and applies them to shaders
     */
    class LightManager
    {
    public:
        LightManager();

        // Light management
        void SetDirectionalLight(const DirectionalLight &light);
        void AddPointLight(const PointLight &light);
        void ClearPointLights();
        void RemovePointLight(size_t index);

        /**
         * @brief Apply all lights to the given shader
         * @param shader The shader to receive light uniforms
         * @param viewPos Camera position for specular calculations
         */
        void ApplyToShader(const core::IShader &shader, const glm::vec3 &viewPos) const;

        // Getters
        const DirectionalLight &GetDirectionalLight() const { return m_directionalLight; }
        const std::vector<PointLight> &GetPointLights() const { return m_pointLights; }

        static constexpr size_t MAX_POINT_LIGHTS = 4;

    private:
        DirectionalLight m_directionalLight;
        std::vector<PointLight> m_pointLights;
    };
}
