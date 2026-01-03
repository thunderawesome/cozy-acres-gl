#include "OpenGLShader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shaders_embedded.h"

namespace cozy::rendering
{
    OpenGLShader::OpenGLShader()
        : OpenGLShader(embedded_cube_vert, embedded_cube_frag) {}

    OpenGLShader::OpenGLShader(const char *vertexSource, const char *fragmentSource)
    {
        uint32_t vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertexSource, nullptr);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        uint32_t fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragmentSource, nullptr);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        m_id = glCreateProgram();
        glAttachShader(m_id, vertex);
        glAttachShader(m_id, fragment);
        glLinkProgram(m_id);
        checkLinkErrors(m_id);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    OpenGLShader::~OpenGLShader() { glDeleteProgram(m_id); }

    void OpenGLShader::Bind() const { glUseProgram(m_id); }
    void OpenGLShader::Unbind() const { glUseProgram(0); }

    void OpenGLShader::SetMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void OpenGLShader::SetVec3(const std::string &name, const glm::vec3 &val) const
    {
        glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(val));
    }

    void OpenGLShader::SetInt(const std::string &name, int val) const
    {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), val);
    }

    void OpenGLShader::checkCompileErrors(uint32_t shader, const std::string &type)
    {
        int success;
        char infoLog[1024];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "SHADER_COMPILE_ERROR (" << type << "):\n"
                      << infoLog << std::endl;
        }
    }

    void OpenGLShader::checkLinkErrors(uint32_t program)
    {
        int success;
        char infoLog[1024];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(program, 1024, nullptr, infoLog);
            std::cerr << "SHADER_LINK_ERROR:\n"
                      << infoLog << std::endl;
        }
    }
}