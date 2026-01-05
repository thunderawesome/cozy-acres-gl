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

        // 2. Town setup with Random Seed
        m_town = std::make_unique<world::Town>();

        // --- RANDOM SEED ASSIGNMENT ---
        std::random_device rd;
        uint64_t randomSeed = static_cast<uint64_t>(rd()) << 32 | rd();

        // --- CONFIGURATION ---
        world::TownConfig config;
        config.cliffSmoothness = 0.2f;  // Let's make it a bit smoother
        config.riverWidth = 4;          // A slightly wider river
        config.riverMeanderChance = 25; // More curves

        m_town->Generate(randomSeed, config);
        m_town->DebugDump();

        // 3. Mesh setup
        const float *cubeData = rendering::primitives::CubeVertices;
        size_t floatCount = sizeof(rendering::primitives::CubeVertices) / sizeof(float);
        m_townMesh = std::make_unique<rendering::OpenGLInstancedMesh>(cubeData, floatCount);

        // Sync Town data to the Mesh
        auto instances = m_town->GenerateRenderData();
        m_townMesh->UpdateInstances(instances);

        // 4. Shader setup
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

    void Engine::Run()
    {
        while (!m_window->ShouldClose())
        {
            m_time->Update();
            float deltaTime = m_time->GetDeltaTime();

            // Update Input (Camera/Movement)
            m_input->Update(*m_window, *m_camera, deltaTime);

            m_renderer->BeginFrame();

            // Render the Town
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