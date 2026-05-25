#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 color;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;
layout(location = 6) in vec4 boneWeights;
layout(location = 7) in ivec4 boneIndices;

layout(set = 1, binding = 0) uniform Bones {
    mat4 bones[255];
} bones;

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
    // Get bone influence masks (1.0 if valid, 0.0 if -1)
    vec4 mask = vec4(
        float(boneIndices.x >= 0),
        float(boneIndices.y >= 0),
        float(boneIndices.z >= 0),
        float(boneIndices.w >= 0)
    );

    // Get bone transforms
    mat4 boneMatX = bones.bones[max(boneIndices.x, 0)];
    mat4 boneMatY = bones.bones[max(boneIndices.y, 0)];
    mat4 boneMatZ = bones.bones[max(boneIndices.z, 0)];
    mat4 boneMatW = bones.bones[max(boneIndices.w, 0)];

    // Transform position
    vec4 skinnedPosition = 
        (boneMatX * vec4(position, 1.0)) * (boneWeights.x * mask.x) +
        (boneMatY * vec4(position, 1.0)) * (boneWeights.y * mask.y) +
        (boneMatZ * vec4(position, 1.0)) * (boneWeights.z * mask.z) +
        (boneMatW * vec4(position, 1.0)) * (boneWeights.w * mask.w);

    // Transform normals/tangents
    vec3 skinnedNormal = 
        (boneMatX * vec4(normal, 0.0)).xyz * (boneWeights.x * mask.x) +
        (boneMatY * vec4(normal, 0.0)).xyz * (boneWeights.y * mask.y) +
        (boneMatZ * vec4(normal, 0.0)).xyz * (boneWeights.z * mask.z) +
        (boneMatW * vec4(normal, 0.0)).xyz * (boneWeights.w * mask.w);

    vec3 skinnedTangent = 
        (boneMatX * vec4(tangent, 0.0)).xyz * (boneWeights.x * mask.x) +
        (boneMatY * vec4(tangent, 0.0)).xyz * (boneWeights.y * mask.y) +
        (boneMatZ * vec4(tangent, 0.0)).xyz * (boneWeights.z * mask.z) +
        (boneMatW * vec4(tangent, 0.0)).xyz * (boneWeights.w * mask.w);

    vec3 skinnedBitangent = 
        (boneMatX * vec4(bitangent, 0.0)).xyz * (boneWeights.x * mask.x) +
        (boneMatY * vec4(bitangent, 0.0)).xyz * (boneWeights.y * mask.y) +
        (boneMatZ * vec4(bitangent, 0.0)).xyz * (boneWeights.z * mask.z) +
        (boneMatW * vec4(bitangent, 0.0)).xyz * (boneWeights.w * mask.w);

    // Default to unskinned if no valid bones
    skinnedPosition = mix(vec4(position, 1.0), skinnedPosition, step(0.0, dot(mask, vec4(1.0))));
    skinnedNormal = mix(normal, skinnedNormal, step(0.0, dot(mask, vec4(1.0))));
    skinnedTangent = mix(tangent, skinnedTangent, step(0.0, dot(mask, vec4(1.0))));
    skinnedBitangent = mix(bitangent, skinnedBitangent, step(0.0, dot(mask, vec4(1.0))));

    // Transform to world space
    vec4 worldPosition = pushConstants.model * skinnedPosition;
    gl_Position = cameraConstants.projection * cameraConstants.view * worldPosition;

    fragNormal = normalize((pushConstants.normal * vec4(skinnedNormal, 0.0)).xyz);
    fragTangent = normalize((pushConstants.normal * vec4(skinnedTangent, 0.0)).xyz);
    fragBitangent = normalize((pushConstants.normal * vec4(skinnedBitangent, 0.0)).xyz);

    fragTexCoord = texCoord;
    outColor = color;
    fragWorldPos = worldPosition.xyz;
}
