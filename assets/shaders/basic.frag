#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 VertexColor;

// Updated to match OpenGLRenderer and Engine naming
uniform sampler2D u_DiffuseTexture;
uniform vec3 u_ViewPos;

void main()
{
    // --- Hardcoded Lighting (Temporary for testing) ---
    vec3 lightPos   = vec3(5.0, 10.0, 5.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Phong)
    float specularStrength = 0.5;
    int shininess = 32;
    vec3 viewDir = normalize(u_ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Texture
    vec3 textureColor = texture(u_DiffuseTexture, TexCoord).rgb;

    // Combine lighting with texture and vertex color tint
    vec3 lightingResult = (ambient + diffuse + specular);
    vec3 finalColor = lightingResult * textureColor * VertexColor;

    FragColor = vec4(finalColor, 1.0);
}