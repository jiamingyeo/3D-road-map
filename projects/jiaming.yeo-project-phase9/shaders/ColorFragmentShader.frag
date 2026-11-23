#version 460 core

in vec3 outColor;
in vec3 outNormal;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform float uAmbientLevel;

out vec4 FragColor;

void main() {
    vec3 normal = normalize(outNormal);
    vec3 ambient = uAmbientLevel * uLightColor;
    float diffuseFactor = max(dot(normal, uLightDir), 0.0);
    vec3 diffuse = diffuseFactor * uLightColor;
    vec3 lighting = ambient + diffuse;
    vec3 finalColor = lighting * outColor;
    
    FragColor = vec4(finalColor, 1.0);
}