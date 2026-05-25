#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 direction;

layout(location = 0) out vec4 outColor;

void main() 
{
    float dotUp = abs(dot(direction, vec3(0,0,1)));

    vec4 horizontalColor = vec4(0.02,0.0,0.08,1.0);
    vec4 verticalColor = vec4(0.15,0.0,0.5,1.0);

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    color += verticalColor * (dotUp);
    color += horizontalColor * (1.0 - dotUp);
    color += vec4(0.0, 0.0, 0.0, 1.0) * max(-1.0 - dotUp, 0.0);

    outColor = color;
}
