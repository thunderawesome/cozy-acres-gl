#include "platform/GlfwWindow.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace cozy::platform
{
    GlfwWindow::GlfwWindow(const WindowConfig &config) : m_config(config)
    {
        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

        m_handle = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
        if (!m_handle)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(m_handle);
        glfwSwapInterval(config.vsync ? 1 : 0);
        glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(m_handle, &fbWidth, &fbHeight);
        m_framebufferSize = {fbWidth, fbHeight};

        // 1. Set the User Pointer so the callback can find this object
        glfwSetWindowUserPointer(m_handle, this);

        // 2. Set the Scroll Callback
        glfwSetScrollCallback(m_handle, [](GLFWwindow *window, double xoffset, double yoffset)
                              {
            auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            if (self) {
                // Accumulate scroll in case multiple events happen in one frame
                self->m_scrollOffset += glm::vec2(static_cast<float>(xoffset), static_cast<float>(yoffset));
            } });
    }

    GlfwWindow::~GlfwWindow()
    {
        if (m_handle)
            glfwDestroyWindow(m_handle);
        glfwTerminate();
    }

    void *GlfwWindow::GetNativeHandle() const noexcept { return m_handle; }

    bool GlfwWindow::ShouldClose() const noexcept { return glfwWindowShouldClose(m_handle); }

    void GlfwWindow::SetShouldClose(bool flag) noexcept
    {
        glfwSetWindowShouldClose(m_handle, flag ? GLFW_TRUE : GLFW_FALSE);
    }

    bool GlfwWindow::IsKeyPressed(int key) const noexcept
    {
        return glfwGetKey(m_handle, key) == GLFW_PRESS;
    }

    glm::vec2 GlfwWindow::GetCursorPosition() const noexcept
    {
        double x, y;
        glfwGetCursorPos(m_handle, &x, &y);
        return {static_cast<float>(x), static_cast<float>(y)};
    }

    // 3. Implement the missing getter
    glm::vec2 GlfwWindow::GetMouseScroll() const noexcept
    {
        return m_scrollOffset;
    }

    glm::ivec2 GlfwWindow::GetFramebufferSize() const noexcept { return m_framebufferSize; }

    glm::ivec2 GlfwWindow::GetWindowSize() const noexcept
    {
        int w, h;
        glfwGetWindowSize(m_handle, &w, &h);
        return {w, h};
    }

    void GlfwWindow::SwapBuffers() noexcept { glfwSwapBuffers(m_handle); }

    void GlfwWindow::PollEvents() noexcept
    {
        // 4. Reset the scroll delta BEFORE polling new events
        m_scrollOffset = glm::vec2(0.0f);

        glfwPollEvents();

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(m_handle, &fbWidth, &fbHeight);
        m_framebufferSize = {fbWidth, fbHeight};
    }

    void GlfwWindow::SetTitle(const std::string &title)
    {
        m_config.title = title;
        glfwSetWindowTitle(m_handle, title.c_str());
    }

    void GlfwWindow::SetVSync(bool enabled)
    {
        m_config.vsync = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void GlfwWindow::SetCursorVisible(bool visible)
    {
        glfwSetInputMode(m_handle, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
}