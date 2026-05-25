#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(set = 2, binding = 0) uniform CameraConstants {
	mat4 view;
	mat4 projection;
} cameraConstants;

layout(location = 0) out vec3 worldDirection;

void main()
{
	const vec3 quad[4] = vec3[4](
		vec3(-1.0, -1.0, 1.0),
		vec3( -1.0, 1.0, 1.0),
		vec3( 1.0, 1.0, 1.0),
		vec3( 1.0,  -1.0, 1.0)
	);

	mat4 invViewProj = inverse(cameraConstants.projection * cameraConstants.view);
	vec4 nearPoint = invViewProj * vec4(quad[gl_VertexIndex].xy, -1.0, 1.0);
	vec4 farPoint  = invViewProj * vec4(quad[gl_VertexIndex].xy,  1.0, 1.0);

	worldDirection = normalize((farPoint.xyz / farPoint.w) - (nearPoint.xyz / nearPoint.w));
	gl_Position = vec4(quad[gl_VertexIndex], 1.0);
}
