#pragma once

#include "platform/IWindow.h"
#include <glm/glm.hpp>

struct GLFWwindow;

namespace cozy::platform
{
    class GlfwWindow final : public IWindow
    {
    private:
        GLFWwindow *m_handle{nullptr};
        WindowConfig m_config;
        glm::ivec2 m_framebufferSize{0, 0};
        glm::vec2 m_scrollOffset{0.0f, 0.0f};

    public:
        explicit GlfwWindow(const WindowConfig &config = WindowConfig::Default());
        ~GlfwWindow() override;

        // Interface Implementation
        void *GetNativeHandle() const noexcept override;
        void SwapBuffers() noexcept override;
        void PollEvents() noexcept override;

        [[nodiscard]] bool ShouldClose() const noexcept override;
        void SetShouldClose(bool flag) noexcept override;

        [[nodiscard]] bool IsKeyPressed(int key) const noexcept override;
        [[nodiscard]] glm::vec2 GetCursorPosition() const noexcept override;
        [[nodiscard]] glm::vec2 GetMouseScroll() const noexcept override;

        [[nodiscard]] glm::ivec2 GetFramebufferSize() const noexcept override;
        [[nodiscard]] glm::ivec2 GetWindowSize() const noexcept override;

        void SetTitle(const std::string &title) override;
        void SetVSync(bool enabled) override;
        void SetCursorVisible(bool visible) override;
    };
}