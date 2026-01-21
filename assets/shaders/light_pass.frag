#version 460 core
#extension GL_ARB_bindless_texture : require

struct VertexData
{
    float position[3];
    float normal[3];
    float tangent[3];
    float bitangent[3];
    float uv[2];
};

layout(binding = 0, std430) readonly buffer vertices {
    VertexData data[];
};

layout(binding = 1, std430) readonly buffer textures_buffer {
    sampler2D textures[];
};

layout(location = 0) uniform uint tex_index;

layout(location = 0) in vec4 in_frag_position;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_colour;

void main()
{
    vec3 colour = texture(textures[tex_index], in_uv).rgb;

    out_colour = vec4(colour, 1.0);
}


