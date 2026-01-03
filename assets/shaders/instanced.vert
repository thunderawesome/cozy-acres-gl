#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// Attributes 3-6 are the mat4 model matrix
layout (location = 3) in mat4 aInstanceModel; 
layout (location = 7) in vec3 aInstanceColor;

out vec3 FragPos;
out vec3 Normal;
out vec3 vColor;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    FragPos = vec3(aInstanceModel * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(aInstanceModel))) * aNormal;
    vColor = aInstanceColor;
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}