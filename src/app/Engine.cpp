#include "app/Engine.h"

// Platform & Interfaces
#include "platform/GlfwWindow.h"
#include "rendering/opengl/OpenGLRenderer.h"

// Core
#include "core/camera/FreeCamera.h"
#include "core/camera/CameraConfig.h"
#include "core/input/InputSystem.h"
#include "core/time/TimeSystem.h"

// Rendering Implementations - MUST INCLUDE THESE TO FIX ERRORS
#include "rendering/opengl/OpenGLMesh.h"
#include "rendering/opengl/OpenGLShader.h"
#include "rendering/opengl/OpenGLTexture.h"

#include <glm/gtc/matrix_transform.hpp>

namespace cozy::app
{
    Engine::Engine()
        : m_window(std::make_unique<platform::GlfwWindow>(platform::WindowConfig::Default())), m_renderer(std::make_unique<rendering::OpenGLRenderer>()), m_camera(std::make_unique<core::FreeCamera>(core::CameraConfig::FreeFlyPreset())), m_input(std::make_unique<core::InputSystem>(core::InputConfig::Default())), m_time(std::make_unique<core::TimeSystem>())
    {
        m_renderer->Initialize(m_window->GetNativeHandle());

        // Now the compiler knows what these types are!
        m_testMesh = std::make_unique<rendering::OpenGLMesh>();
        m_testShader = std::make_unique<rendering::OpenGLShader>();
        m_testTexture = std::make_unique<rendering::OpenGLTexture>("placeholder.jpg");
    }

    Engine::~Engine() = default;

    void Engine::Run()
    {
        while (!m_window->ShouldClose())
        {
            m_time->Update();
            float deltaTime = m_time->GetDeltaTime();

            m_input->Update(*m_window, *m_camera, deltaTime);

            m_renderer->BeginFrame();

            if (m_testMesh && m_testShader && m_testTexture)
            {
                m_renderer->BindTexture(*m_testTexture, 0);

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, (float)m_time->GetTotalTimeHighPrecision(), glm::vec3(0.5f, 1.0f, 0.0f));

                m_renderer->DrawMesh(*m_testMesh, *m_testShader, model, *m_camera);
            }

            m_renderer->EndFrame();
            m_window->SwapBuffers();
            m_window->PollEvents();
        }
    }
}