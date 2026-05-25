#version 450

layout(set = 0, binding = 0) uniform sampler2D u_Image;

layout(location = 0) in vec2 v_UV;
layout(location = 0) out vec4 o_Color;

const int blurRadius = 2;
const int weightCount = 2 * blurRadius + 1;

const float weight[weightCount] = float[](
    0.06136f, 0.24477f, 0.38774f, 0.24477f, 0.06136f
);

vec3 FetchColor(vec2 texCoord, vec2 texelSize)
{
    vec3 tl = texture(u_Image, texCoord + vec2(-2, -2) * texelSize).rgb;
    vec3 tc = texture(u_Image, texCoord + vec2( 0, -2) * texelSize).rgb;
    vec3 tr = texture(u_Image, texCoord + vec2( 2, -2) * texelSize).rgb;
    vec3 cl = texture(u_Image, texCoord + vec2(-2,  0) * texelSize).rgb;
    vec3 cc = texture(u_Image, texCoord).rgb;
    vec3 cr = texture(u_Image, texCoord + vec2( 2,  0) * texelSize).rgb;
    vec3 bl = texture(u_Image, texCoord + vec2(-2,  2) * texelSize).rgb;
    vec3 bc = texture(u_Image, texCoord + vec2( 0,  2) * texelSize).rgb;
    vec3 br = texture(u_Image, texCoord + vec2( 2,  2) * texelSize).rgb;

    vec3 tlSample = (tl + tc + cl + cc) * 0.25 * 0.125;
    vec3 trSample = (tr + tc + cr + cc) * 0.25 * 0.125;
    vec3 blSample = (bl + bc + cl + cc) * 0.25 * 0.125;
    vec3 brSample = (br + bc + cr + cc) * 0.25 * 0.125;

    vec3 ctl = texture(u_Image, texCoord + vec2(-1, -1) * texelSize).rgb;
    vec3 ctr = texture(u_Image, texCoord + vec2( 1, -1) * texelSize).rgb;
    vec3 cbl = texture(u_Image, texCoord + vec2(-1,  1) * texelSize).rgb;
    vec3 cbr = texture(u_Image, texCoord + vec2( 1,  1) * texelSize).rgb;

    vec3 centerSample = (ctl + ctr + cbl + cbr) * 0.25 * 0.5;

    return tlSample + trSample + blSample + brSample + centerSample;
}

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_Image, 0));
    vec4 color = vec4(FetchColor(v_UV, texelSize), 1.0);
    color.r = max(color.r, 0.0);
    color.g = max(color.g, 0.0);
    color.b = max(color.b, 0.0);
    color.a = max(color.a, 0.0);
    o_Color = vec4(color.rgb,1.0);
}
