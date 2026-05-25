#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec4 inColor;
layout(location = 5) in vec3 inPosition;

layout(binding = 0) uniform MaterialConstants
{
	vec4 m_DiffuseColor ;
	vec4 m_SpecularColor;
	vec4 m_AmbientColor ;

	float m_Shininess;
} materialConstants;

layout(set = 2, binding = 0) uniform CameraConstants {
	mat4 view;
	mat4 projection;
} cameraConstants;

layout(binding = 1) uniform sampler2D tex;
layout(binding = 2) uniform sampler2D specularMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D parrallaxMap;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float numer   = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return numer / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 ExtractPositionFromViewMatrix(mat4 viewMatrix) {
    mat3 rotation = mat3(viewMatrix);
    vec3 translation = vec3(-viewMatrix[3]);
    return transpose(rotation) * translation;
}

void main() 
{
    vec3 cameraPos = ExtractPositionFromViewMatrix(cameraConstants.view);
	vec3 normalMapColor = texture(normalMap, inTexcoord).rgb;
	vec3 normal;
    mat3x3 TBN = mat3x3(inTangent, inBitangent, inNormal);
    
	if(normalMapColor == vec3(1.0, 1.0, 1.0))
	{
		normal = inNormal;
	}
	else
	{
		normal = normalize(TBN * (normalMapColor * 2.0 - 1.0));
	}
    
    vec3 viewDir = normalize(cameraPos - inPosition);
    vec3 lightDir = -normalize(vec3(1.0, -1.0, -1.0));
    vec3 halfDir = normalize(lightDir + viewDir);

    vec4 albedoTex = texture(tex, inTexcoord);
    vec3 albedo = albedoTex.rgb * materialConstants.m_DiffuseColor.rgb;
    float metallic = 0.0;
    float roughness = 1.0;

    // Use green channel for AO
    float ao = inColor.g;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    vec3 Lo = vec3(0.0);

    // Base radiance
    vec3 radiance = vec3(0.08);

    float NDF = DistributionGGX(normal, halfDir, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfDir, viewDir), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / max(denominator, 0.001);
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = albedo * ao * 0.08;

    // Baked lighting from vertex color (acts like diffuse light)
    vec3 bakedLighting = albedo * (inColor.r * inColor.r * 16.0); // simulates static lightmap effect

    vec3 color = ambient + Lo + bakedLighting;

    // Brightness boost near world origin
    float centerBoostRadius = 50.0;
    float centerBoostStrength = 80.0;

    float centerDist = length(inPosition);
    float centerBoost = clamp(1.0 - (centerDist / centerBoostRadius), 0.0, 1.0);
    color += color * centerBoost * centerBoostStrength;

    // Fog
    float dist = length(inPosition - cameraPos);
    float fogStart = 6.0 * 32.0;
    float fogEnd = 11.0 * 32.0;
    float fog = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    float invT = 1.f - fog;
	float tPow = 1.f - invT * invT * invT * invT * invT;
    outColor = mix(vec4(color, albedoTex.a), vec4(0,0,0,1), tPow);
}
