#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 VertexColor;

// Directional Light
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float ambient;
    float diffuse;
    float specular;
};

// Point Light
struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

#define MAX_POINT_LIGHTS 4

uniform DirectionalLight u_DirLight;
uniform int u_NumPointLights;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform vec3 u_ViewPos;
uniform sampler2D u_DiffuseTexture;

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    
    // Ambient
    vec3 ambient = light.ambient * light.color;
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * light.color;
    
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = light.specular * spec * light.color;
    
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    vec3 ambient = 0.1 * light.color * attenuation;
    vec3 diffuse = diff * light.color * attenuation;
    vec3 specular = spec * 0.5 * light.color * attenuation;
    
    return ambient + diffuse + specular;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(u_ViewPos - FragPos);
    
    // Directional lighting
    vec3 result = CalcDirectionalLight(u_DirLight, norm, viewDir);
    
    // Point lights
    for(int i = 0; i < u_NumPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += CalcPointLight(u_PointLights[i], norm, FragPos, viewDir);
    }
    
    // Apply lighting to texture and vertex color
    vec3 textureColor = texture(u_DiffuseTexture, TexCoord).rgb;
    vec3 finalColor = result * textureColor * VertexColor;
    
    FragColor = vec4(finalColor, 1.0);
}