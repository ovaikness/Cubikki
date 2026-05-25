#version 450
#extension GL_KHR_vulkan_glsl : enable

vec2 quad[] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0,  1.0)
);

vec2 quadUVs[] = vec2[](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0)
);

layout(location = 0) out vec2 uv;

void main() {
	gl_Position = vec4(quad[gl_VertexIndex], 0.0, 1.0);
	uv = quadUVs[gl_VertexIndex];
}
