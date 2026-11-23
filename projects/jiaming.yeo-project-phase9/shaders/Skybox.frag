#version 460 core

in vec2 uv;
out vec4 FragColor;

uniform vec3 uSkyColor;
uniform vec3 uGroundColor;

void main()
{
    float vertical = 1.0 - uv.y;
    vec3 color = mix(uGroundColor, uSkyColor, vertical);
    
    FragColor = vec4(color, 1.0);
}