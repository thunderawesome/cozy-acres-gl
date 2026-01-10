#include "app/Engine.h"
#include "shaders_embedded.h"

// Platform & Rendering
#include "platform/GlfwWindow.h"
#include "rendering/opengl/OpenGLRenderer.h"
#include "rendering/opengl/OpenGLInstancedMesh.h"
#include "rendering/opengl/OpenGLShader.h"
#include "rendering/opengl/OpenGLTexture.h"
#include "rendering/opengl/PrimitiveData.h"

// Core & World
#include "core/camera/FreeCamera.h"
#include "core/camera/CameraConfig.h"
#include "core/input/InputSystem.h"
#include "core/time/TimeSystem.h"
#include "world/Town.h"

#include <glm/gtc/matrix_transform.hpp>
#include <random>

namespace cozy::app
{
    Engine::Engine()
        : m_window(std::make_unique<platform::GlfwWindow>(platform::WindowConfig::Default())),
          m_renderer(std::make_unique<rendering::OpenGLRenderer>()),
          m_camera(std::make_unique<core::FreeCamera>(core::CameraConfig::FreeFlyPreset())),
          m_input(std::make_unique<core::InputSystem>(core::InputConfig::Default())),
          m_time(std::make_unique<core::TimeSystem>())
    {
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

        // 3. Shader setup
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
    }

    Engine::~Engine() = default;

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

            // 1. Update Input (Handles Camera movement and Action edge detection)
            m_input->Update(*m_window, *m_camera, deltaTime);

            // 2. Handle Logic via Actions (Polished & Decoupled)
            // Note: m_rKeyWasPressed is no longer needed here as the InputSystem handles state.
            if (m_input->IsActionTriggered(core::InputAction::Regenerate))
            {
                RegenerateTown();
            }

            // 3. Rendering
            m_renderer->BeginFrame();

            if (m_townMesh && m_instancedShader)
            {
                m_renderer->BindTexture(*m_testTexture, 0);
                m_renderer->DrawInstanced(*m_townMesh, *m_instancedShader, *m_camera);
            }

            m_renderer->EndFrame();

            m_window->SwapBuffers();
            m_window->PollEvents();
        }
    }
}