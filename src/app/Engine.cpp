#include "app/Engine.h"
#include "shaders_embedded.h"

// Platform & Rendering
#include "platform/GlfwWindow.h"
#include "rendering/opengl/OpenGLRenderer.h"
#include "rendering/opengl/OpenGLInstancedMesh.h"
#include "rendering/opengl/OpenGLShader.h"
#include "rendering/opengl/OpenGLTexture.h"
#include "rendering/opengl/PrimitiveData.h"
#include "rendering/debug/DebugGizmoRenderer.h"
#include "rendering/LightManager.h"

// Core & World
#include "core/camera/FreeCamera.h"
#include "core/camera/CameraConfig.h"
#include "core/input/InputSystem.h"
#include "core/time/TimeSystem.h"
#include "world/Town.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h> // ADDED: For glDisable/glEnable
#include <random>

namespace cozy::app
{
    Engine::Engine()
        : m_window(std::make_unique<platform::GlfwWindow>(platform::WindowConfig::Default())),
          m_renderer(std::make_unique<rendering::OpenGLRenderer>()),
          m_camera(std::make_unique<core::FreeCamera>(core::CameraConfig::FreeFlyPreset())),
          m_input(std::make_unique<core::InputSystem>(core::InputConfig::Default())),
          m_time(std::make_unique<core::TimeSystem>()),
          m_lightManager(std::make_unique<rendering::LightManager>())
    {
        // Initialize OpenGL context FIRST
        m_renderer->Initialize(m_window->GetNativeHandle());

        // 1. Camera setup
        m_camera->SetPosition(glm::vec3(40.0f, 60.0f, 120.0f));
        m_camera->SetRotation(-90.0f, -45.0f);

        // 2. Town and Mesh setup
        m_town = std::make_unique<world::Town>();
        const float *cubeData = rendering::primitives::CubeVertices;
        size_t floatCount = sizeof(rendering::primitives::CubeVertices) / sizeof(float);
        m_townMesh = std::make_unique<rendering::OpenGLInstancedMesh>(cubeData, floatCount);

        // Perform initial generation
        RegenerateTown();

        // 3. Shader setup (AFTER OpenGL initialization)
        if (embedded_instanced_vert && embedded_instanced_frag)
        {
            m_instancedShader = std::make_unique<rendering::OpenGLShader>(
                embedded_instanced_vert,
                embedded_instanced_frag);
        }
        else
        {
            m_instancedShader = std::make_unique<rendering::OpenGLShader>();
        }

        m_testTexture = std::make_unique<rendering::OpenGLTexture>("placeholder.jpg");

        // 4. Debug visualization setup (AFTER OpenGL initialization)
        m_debugGizmos = std::make_unique<rendering::debug::DebugGizmoRenderer>();

        if (embedded_debug_vert && embedded_debug_frag)
        {
            m_debugShader = std::make_unique<rendering::OpenGLShader>(
                embedded_debug_vert,
                embedded_debug_frag);
        }

        // 5. Setup lighting
        SetupLighting();
    }

    Engine::~Engine() = default;

    void Engine::SetupLighting()
    {
        // Configure directional light (sun)
        rendering::DirectionalLight sun;
        sun.direction = glm::vec3(-0.5f, -1.0f, -0.3f);
        sun.color = glm::vec3(1.0f, 0.98f, 0.95f) * 1.5f; // Warm sunlight
        sun.ambient = 0.2f;
        sun.diffuse = 0.8f;
        sun.specular = 0.3f;
        m_lightManager->SetDirectionalLight(sun);

        // Add point lights for visual interest
        rendering::PointLight light1;
        light1.position = glm::vec3(20.0f, 15.0f, 20.0f);
        light1.color = glm::vec3(1.0f, 0.9f, 0.7f) * 2.0f; // Warm accent
        light1.constant = 1.0f;
        light1.linear = 0.09f;
        light1.quadratic = 0.032f;
        m_lightManager->AddPointLight(light1);

        rendering::PointLight light2;
        light2.position = glm::vec3(60.0f, 15.0f, 60.0f);
        light2.color = glm::vec3(0.7f, 0.9f, 1.0f) * 2.0f; // Cool accent
        light2.constant = 1.0f;
        light2.linear = 0.09f;
        light2.quadratic = 0.032f;
        m_lightManager->AddPointLight(light2);
    }

    void Engine::RegenerateTown()
    {
        // --- RANDOM SEED ASSIGNMENT ---
        std::random_device rd;
        uint64_t randomSeed = static_cast<uint64_t>(rd()) << 32 | rd();

        // --- CONFIGURATION ---
        world::TownConfig config;

        // Generate logic
        m_town->Generate(randomSeed, config);

        // Update GPU data
        auto instances = m_town->GenerateRenderData();
        if (m_townMesh)
        {
            m_townMesh->UpdateInstances(instances);
        }
    }

    void Engine::Run()
    {
        while (!m_window->ShouldClose())
        {
            m_time->Update();
            float deltaTime = m_time->GetDeltaTime();

            // 1. Update Input
            m_input->Update(*m_window, *m_camera, deltaTime);

            // 2. Handle Logic
            if (m_input->IsActionTriggered(core::InputAction::Regenerate))
            {
                RegenerateTown();
            }

            // Toggle debug gizmos
            if (m_input->IsActionTriggered(core::InputAction::ToggleDebug))
            {
                m_showDebugGizmos = !m_showDebugGizmos;
            }

            // 3. Rendering
            m_renderer->BeginFrame();

            if (m_townMesh && m_instancedShader)
            {
                m_renderer->BindTexture(*m_testTexture, 0);

                m_renderer->DrawInstanced(
                    *m_townMesh,
                    *m_instancedShader,
                    *m_camera,
                    m_lightManager.get());
            }

            // Draw debug gizmos AFTER scene (on top)
            if (m_showDebugGizmos && m_debugGizmos && m_debugShader)
            {
                // Disable depth test so gizmos are always visible
                glDisable(GL_DEPTH_TEST);

                m_debugGizmos->RenderLightGizmos(
                    *m_lightManager,
                    *m_debugShader,
                    *m_camera);

                glEnable(GL_DEPTH_TEST);
            }

            m_renderer->EndFrame();
            m_window->SwapBuffers();
            m_window->PollEvents();
        }
    }
}