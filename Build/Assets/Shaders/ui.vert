
#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 color;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec3 outPos;

layout(set = 1, binding = 0) uniform CameraConstants {
	mat4 view;
	mat4 projection;
} cameraConstants;

void main() {
	vec4 worldPos = vec4(position, 1.0);
	vec4 viewPos = cameraConstants.view * worldPos;
	gl_Position = cameraConstants.projection * viewPos;

	outTexCoord = texCoord;
    outColor = color;
    outPos = worldPos.xyz;
}
