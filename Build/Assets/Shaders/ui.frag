#version 450

layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 inTexCoords;
layout(location = 2) in vec3 inPos;
layout(binding = 1) uniform sampler2D texSampler;


layout(set = 2, binding = 0) uniform WidgetConstants {
    vec2 m_Position;
    vec2 m_HalfExtents;
    vec2 m_IBasis;
    vec2 m_JBasis;
} widgetConstants;

bool isInside(vec2 pos) {
    vec2 localPos = pos - widgetConstants.m_Position;
    float lengthInIBasis = dot(localPos, widgetConstants.m_IBasis);
    float lengthInJBasis = dot(localPos, widgetConstants.m_JBasis);

    vec2 localCoords = vec2(lengthInIBasis, lengthInJBasis);

    if (abs(lengthInIBasis) > widgetConstants.m_HalfExtents.x) {
        return false;
    }

    if (abs(lengthInJBasis) > widgetConstants.m_HalfExtents.y) {
        return false;
    }

    return true;
}

void main() {
    //if (!isInside(inPos.xy)) {
    //    discard;
    //}

    outColor = texture(texSampler, inTexCoords) * inColor;
}