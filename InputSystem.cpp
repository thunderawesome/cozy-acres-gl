#include "InputSystem.h"
#include "Camera.h" // for cameraPos, cameraFront, cameraUp, yaw, pitch, CAMERA_SPEED, PITCH_CLAMP
#include "Window.h" // for WINDOW_WIDTH, WINDOW_HEIGHT

InputSystem *InputSystem::instance = nullptr;

InputSystem::InputSystem()
{
    instance = this;
    lastX = static_cast<float>(WINDOW_WIDTH) / 2.0f;
    lastY = static_cast<float>(WINDOW_HEIGHT) / 2.0f;
    firstMouse = true;
}

void InputSystem::setupCallbacks(GLFWwindow *window)
{
    glfwSetCursorPosCallback(window, mouseCallback);
}

void InputSystem::mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    if (instance)
    {
        instance->processMouse(window, xpos, ypos);
    }
}

void InputSystem::processKeyboard(GLFWwindow *window, float deltaTime)
{
    float velocity = CAMERA_SPEED * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraFront * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraFront * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void InputSystem::processMouse(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Y is inverted

    lastX = xpos;
    lastY = ypos;

    xoffset *= MOUSE_SENSITIVITY;
    yoffset *= MOUSE_SENSITIVITY;

    yaw += xoffset;
    pitch += yoffset;

    // Clamp pitch to avoid flipping
    if (pitch > PITCH_CLAMP)
        pitch = PITCH_CLAMP;
    if (pitch < -PITCH_CLAMP)
        pitch = -PITCH_CLAMP;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}