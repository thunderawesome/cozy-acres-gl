#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 VertexColor;

uniform sampler2D diffuseTexture;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Phong)
    float specularStrength = 0.8;  // How shiny the material is
    int shininess = 32;            // Higher = tighter highlight
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Texture
    vec3 textureColor = texture(diffuseTexture, TexCoord).rgb;

    // Combine with vertex color tint
    vec3 result = (ambient + diffuse + specular) * textureColor * VertexColor;

    FragColor = vec4(result, 1.0);
}