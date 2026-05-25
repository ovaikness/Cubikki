#version 450

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D u_Scene;
layout(binding = 1) uniform sampler2D u_BloomImages[8];

layout(push_constant) uniform Params {
    float exposure;
    float bloomStrength;   // e.g. 0.04 per Jiménez’ notes
    float tentRadiusUV;    // e.g. 1.0 (radius in UV space, not pixels)
    float _pad;
} pc;

// 3×3 tent (Bartlett) weights (separable [1 2 1]/4 × [1 2 1]/4)
const float w1 = 1.0/16.0;
const float w2 = 2.0/16.0;
const float w4 = 4.0/16.0;

// COD:AW-style tent fetch: fixed radius in UV space (has “holes”, that’s fine)
vec3 TentFetch(int index, vec2 uv, vec2 texelSize, float radiusUV)
{
    vec2 o = radiusUV * texelSize; // radius in UV units
    vec3 c = vec3(0.0);
    c += texture(u_BloomImages[index], uv + o * vec2(-1.0, -1.0)).rgb * w1;
    c += texture(u_BloomImages[index], uv + o * vec2( 0.0, -1.0)).rgb * w2;
    c += texture(u_BloomImages[index], uv + o * vec2( 1.0, -1.0)).rgb * w1;

    c += texture(u_BloomImages[index], uv + o * vec2(-1.0,  0.0)).rgb * w2;
    c += texture(u_BloomImages[index], uv).rgb                           * w4;
    c += texture(u_BloomImages[index], uv + o * vec2( 1.0,  0.0)).rgb * w2;

    c += texture(u_BloomImages[index], uv + o * vec2(-1.0,  1.0)).rgb * w1;
    c += texture(u_BloomImages[index], uv + o * vec2( 0.0,  1.0)).rgb * w2;
    c += texture(u_BloomImages[index], uv + o * vec2( 1.0,  1.0)).rgb * w1;
    return c;
}

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_Scene, 0));
    vec3 hdrColor  = texture(u_Scene, texCoord).rgb;

    // Progressive “sum the chain as you upscale” in one pass approximation:
    // sample each mip with a 3×3 tent and decay contribution per level.
    float intensity = pc.bloomStrength; // e.g. 0.04 (no explicit threshold)
    vec3 bloom = vec3(0.0);
    for (int i = 0; i < 8; ++i) {
        bloom += TentFetch(i, texCoord, texelSize, pc.tentRadiusUV) * intensity;
        intensity *= 0.5; // fade higher levels (tweak to taste)
    }

    hdrColor += bloom;

    // Simple filmic-ish tonemap used in the slides’ wrap-up
    float exposure = pc.exposure; // 1.0 by default
    vec3 mapped = (hdrColor * exposure) / (hdrColor * exposure + vec3(1.0));

    outColor = vec4(mapped, 1.0);
}
