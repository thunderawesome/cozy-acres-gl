#include "OpenGLRenderer.h"
#include "rendering/opengl/OpenGLInstancedMesh.h"
#include "core/graphics/IGpuResource.h"
#include "core/camera/ICamera.h"
#include <glad/glad.h>
#include <iostream>

namespace cozy::rendering
{
    void OpenGLRenderer::Initialize(void *nativeWindowHandle)
    {
        if (!gladLoadGL())
        {
            std::cerr << "[OpenGLRenderer] Failed to initialize GLAD" << std::endl;
            return;
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        std::cout << "[OpenGLRenderer] Context initialized (CCW Winding)" << std::endl;
    }

    void OpenGLRenderer::BeginFrame()
    {
        glClearColor(0.45f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRenderer::DrawMesh(
        const core::IMesh &mesh,
        const core::IShader &shader,
        const glm::mat4 &modelMatrix,
        const core::ICamera &camera)
    {
        shader.Bind();
        shader.SetMat4("u_Model", modelMatrix);
        shader.SetMat4("u_View", camera.GetViewMatrix());

        float aspect = 1280.0f / 720.0f; // TODO: Pull from window config
        shader.SetMat4("u_Projection", camera.GetProjectionMatrix(aspect));

        glBindVertexArray(mesh.GetRendererID());
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mesh.GetVertexCount());
        glBindVertexArray(0);
    }

    void OpenGLRenderer::DrawInstanced(
        const OpenGLInstancedMesh &mesh,
        const core::IShader &shader,
        const core::ICamera &camera)
    {
        shader.Bind();

        float aspect = 1280.0f / 720.0f;
        shader.SetMat4("u_View", camera.GetViewMatrix());
        shader.SetMat4("u_Projection", camera.GetProjectionMatrix(aspect));

        mesh.Draw();
    }

    void OpenGLRenderer::EndFrame() { /* SwapBuffers handled in Engine */ }

    void OpenGLRenderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glViewport(x, y, width, height);
    }

    void OpenGLRenderer::BindTexture(const core::ITexture &texture, uint32_t slot)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture.GetRendererID());
    }
}