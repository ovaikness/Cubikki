#version 450

layout(location = 0) out vec2 v_UV;

void main()
{
    vec2 positions[6] = vec2[](
        vec2(-1.0, -1.0), // bottom-left
        vec2( 1.0, -1.0), // bottom-right
        vec2(-1.0,  1.0), // top-left

        vec2(-1.0,  1.0), // top-left
        vec2( 1.0, -1.0), // bottom-right
        vec2( 1.0,  1.0)  // top-right
    );

    vec2 uvs[6] = vec2[](
        vec2(0.0, 0.0), // bottom-left
        vec2(1.0, 0.0), // bottom-right
        vec2(0.0, 1.0), // top-left

        vec2(0.0, 1.0), // top-left
        vec2(1.0, 0.0), // bottom-right
        vec2(1.0, 1.0)  // top-right
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    v_UV = uvs[gl_VertexIndex];
}
