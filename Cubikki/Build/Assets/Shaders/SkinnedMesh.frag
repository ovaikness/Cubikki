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

layout(set = 3, binding = 0) uniform LightingConstants
{
    vec3 m_BlockLightDir;
    float m_BlockLightIntensity;
} lightConstants;

layout(binding = 1) uniform sampler2D tex;
layout(binding = 2) uniform sampler2D specularMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D parrallaxMap;

const float PI = 3.14159265359;

vec3 quantizedPosition(vec3 inPosition, vec3 min, vec3 max, vec3 quantization)
{
	vec3 range = max - min;
	vec3 quantized = (inPosition - min) / range;
	quantized = floor(quantized * quantization) / quantization;
	return min + range * quantized;
}

//How are the normals distributed given the roughness
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
    mat3 rotation = mat3(viewMatrix); // Extract 3x3 rotation
    vec3 translation = vec3(-viewMatrix[3]); // Inverse of translation
    return transpose(rotation) * translation; // Apply inverse rotation
}

vec3 CalculateDirectionalLightContribution(
    vec3 albedo,
    vec3 normal,

    vec3 viewDir,
    vec3 lightDir,
    vec3 halfDir,

    float metallic,
    float roughness,
    float ao
)
{
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    vec3 Lo = vec3(0.0);
    
    vec3 radiance = vec3(10.0);
    
    //Lighting contribution
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

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main() 
{
    vec3 cameraPos = ExtractPositionFromViewMatrix(cameraConstants.view);
	vec3 normalMapColor = texture(normalMap, inTexcoord).rgb;
	vec3 normal;
	if(normalMapColor == vec3(1.0, 1.0, 1.0))
	{
		normal = inNormal;
	}
	else
	{
		normal = normalize(normalMapColor * 2.0 - 1.0);
	}
    
    vec3 viewDir = normalize(cameraPos - inPosition);
    vec3 lightDir = -normalize(vec3(3.0, -3.0, -1.0));
    vec3 halfDir = normalize(lightDir + viewDir);
    mat3x3 TBN = mat3x3(inTangent,
						inBitangent,
						inNormal);
    
    vec3 albedo = texture(tex, inTexcoord).rgb * materialConstants.m_DiffuseColor.rgb;
    float metallic = 0.0;
    float roughness = 0.5;
    float ao = 1.0;
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    vec3 Lo = vec3(0.0);
    
    vec3 radiance = vec3(10.0);
    
    Lo += CalculateDirectionalLightContribution(albedo, normal, viewDir, lightDir, halfDir, metallic, roughness, ao);
    
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
    outColor = vec4(color,1.0);
}
