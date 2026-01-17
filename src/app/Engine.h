#pragma once
#include <memory>
#include <vector>
#include "world/Town.h"

namespace cozy::platform
{
    class IWindow;
}

namespace cozy::core
{
    class ICamera;
    class IInputSystem;
    class TimeSystem;
}

namespace cozy::rendering
{
    class IRenderer;
    class OpenGLMesh;
    class OpenGLShader;
    class OpenGLTexture;
    class OpenGLInstancedMesh;
    class LightManager; // NEW: Forward declaration for lighting system
}

namespace cozy::app
{
    class Engine
    {
    private:
        std::unique_ptr<platform::IWindow> m_window;
        std::unique_ptr<rendering::IRenderer> m_renderer;
        std::unique_ptr<core::ICamera> m_camera;
        std::unique_ptr<core::IInputSystem> m_input;
        std::unique_ptr<core::TimeSystem> m_time;

        // Town System
        std::unique_ptr<world::Town> m_town;
        std::unique_ptr<rendering::OpenGLInstancedMesh> m_townMesh;
        std::unique_ptr<rendering::OpenGLShader> m_instancedShader;

        // Test objects
        std::unique_ptr<rendering::OpenGLTexture> m_testTexture;

        // Lighting system
        std::unique_ptr<rendering::LightManager> m_lightManager;

        // Input State for debouncing
        bool m_rKeyWasPressed = false;

        // Helper methods
        void RegenerateTown();
        void SetupLighting(); // Initialize lights

    public:
        Engine();
        ~Engine();
        void Run();
    };
}