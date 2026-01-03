#pragma once

#include <memory>

// Forward declarations
namespace cozy::platform
{
    class IWindow;
}
namespace cozy::rendering
{
    class IRenderer;
    class OpenGLMesh;    // Add this
    class OpenGLShader;  // Add this
    class OpenGLTexture; // Add this
}
namespace cozy::core
{
    class ICamera;
    class IInputSystem;
    class TimeSystem;
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

        // Concrete test objects
        std::unique_ptr<rendering::OpenGLMesh> m_testMesh;
        std::unique_ptr<rendering::OpenGLShader> m_testShader;
        std::unique_ptr<rendering::OpenGLTexture> m_testTexture;

    public:
        Engine();
        ~Engine(); // Remember: Explicitly defined in .cpp to handle unique_ptr deletion

        void Run();
    };
}