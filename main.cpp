#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Window.h"
#include "Time.h"
#include "Scene.h"
#include "Light.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "InputSystem.h"

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    // Input setup
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Initialize core objects
    Shader shader;
    Texture diffuseTexture;
    Mesh cubeMesh;
    InputSystem inputSystem;

    // Register GLFW mouse callback using InputSystem's static wrapper
    inputSystem.setupCallbacks(window);

    // Center cursor initially
    glfwSetCursorPos(window, WINDOW_WIDTH / 2.0, WINDOW_HEIGHT / 2.0);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process keyboard input
        inputSystem.processKeyboard(window, deltaTime);

        // Clear buffer
        glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader and set uniforms
        shader.use();

        // Camera matrices and view position
        Camera camera;
        camera.updateShaderUniforms(shader);

        // Lighting
        shader.setVec3("lightPos", LIGHT_POS);
        shader.setVec3("lightColor", LIGHT_COLOR);

        // Texture binding
        shader.setInt("diffuseTexture", 0);
        diffuseTexture.bind(0);

        // Draw main rotating cube
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, currentFrame * glm::radians(CUBE_ROTATION_SPEED), CUBE_ROTATION_AXIS);
        shader.setMat4("model", model);
        cubeMesh.draw();

        // Draw light source visualization cube
        model = glm::mat4(1.0f);
        model = glm::translate(model, LIGHT_POS);
        model = glm::scale(model, glm::vec3(LIGHT_SCALE));
        shader.setMat4("model", model);
        cubeMesh.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}