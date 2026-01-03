#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 vColor;

void main() {
    // 1. Simple Ambient Light (so shadows aren't pitch black)
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
  	
    // 2. Simple Directional Light (Sunlight from above)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // 3. Combine with the instance color
    vec3 result = (ambient + diffuse) * vColor;
    FragColor = vec4(result, 1.0);
}