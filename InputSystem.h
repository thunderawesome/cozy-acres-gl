#pragma once

#include <GLFW/glfw3.h>
#include "Window.h"

constexpr float MOUSE_SENSITIVITY = 0.1f;

class InputSystem
{
public:
    InputSystem();
    void processKeyboard(GLFWwindow *window, float deltaTime);
    void processMouse(GLFWwindow *window, double xpos, double ypos);
    void setupCallbacks(GLFWwindow *window);

private:
    static InputSystem *instance;
    static void mouseCallback(GLFWwindow *window, double xpos, double ypos);

    // Input-specific mouse state
    bool firstMouse = true;
    float lastX = 0.0f; // Will be initialized in constructor using WINDOW_WIDTH/HEIGHT
    float lastY = 0.0f;
};