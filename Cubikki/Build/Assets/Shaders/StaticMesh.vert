#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 color;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;

layout(set = 2, binding = 0) uniform CameraConstants {
	mat4 view;
	mat4 projection;
} cameraConstants;

layout(push_constant) uniform PushConstants {
	mat4 model;
    mat4 normal;
} pushConstants;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragTangent;
layout(location = 3) out vec3 fragBitangent;
layout(location = 4) out vec4 outColor;
layout(location = 5) out vec3 fragWorldPos;

void main() 
{
    // Calculate final position by transforming the vertex position
    vec4 worldPosition = pushConstants.model * vec4(position, 1.0);
    gl_Position = cameraConstants.projection * cameraConstants.view * worldPosition;

    vec3 worldNormal = normalize((pushConstants.normal * vec4(normal, 0.0)).xyz);
    vec3 worldTangent = normalize((pushConstants.normal * vec4(tangent, 0.0)).xyz);
    vec3 worldBitangent = normalize((pushConstants.normal * vec4(bitangent, 0.0)).xyz);

    // Pass through additional attributes
    fragTexCoord = texCoord;
    fragNormal = worldNormal;
    fragTangent = worldTangent;
    fragBitangent = worldBitangent;
    outColor = color;
    fragWorldPos = worldPosition.xyz;
}