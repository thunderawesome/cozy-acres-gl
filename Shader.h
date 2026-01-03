#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    unsigned int ID;

    Shader(const char *vertexSource, const char *fragmentSource);
    Shader();

    void use() const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setInt(const std::string &name, int value) const;

private:
    void checkCompileErrors(unsigned int shader, const std::string &type);
    void checkLinkErrors(unsigned int program);
};