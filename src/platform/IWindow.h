#pragma once

#include <glm/glm.hpp>
#include <string>

namespace cozy::platform
{
    struct WindowConfig
    {
        int width{1280};
        int height{720};
        std::string title{"Cozy Acres GL"};
        bool vsync{true};
        bool resizable{true};

        static WindowConfig Default() noexcept
        {
            return {1280, 720, "Cozy Acres GL", true, true};
        }
    };

    class IWindow
    {
    public:
        virtual ~IWindow() = default;

        [[nodiscard]] virtual void *GetNativeHandle() const noexcept = 0;
        virtual void SwapBuffers() noexcept = 0;
        virtual void PollEvents() noexcept = 0;

        [[nodiscard]] virtual bool ShouldClose() const noexcept = 0;
        virtual void SetShouldClose(bool flag) noexcept = 0;

        // Abstraction Layer for Input
        [[nodiscard]] virtual bool IsKeyPressed(int key) const noexcept = 0;
        [[nodiscard]] virtual glm::vec2 GetCursorPosition() const noexcept = 0;
        [[nodiscard]] virtual glm::vec2 GetMouseScroll() const noexcept = 0;

        // Size and Metrics
        [[nodiscard]] virtual glm::ivec2 GetFramebufferSize() const noexcept = 0;
        [[nodiscard]] virtual glm::ivec2 GetWindowSize() const noexcept = 0;
        [[nodiscard]] virtual float GetAspectRatio() const noexcept
        {
            const auto size = GetFramebufferSize();
            return size.y > 0 ? static_cast<float>(size.x) / static_cast<float>(size.y) : 1.0f;
        }

        // State Management
        virtual void SetTitle(const std::string &title) = 0;
        virtual void SetVSync(bool enabled) = 0;
        virtual void SetCursorVisible(bool visible) = 0;
    };
}