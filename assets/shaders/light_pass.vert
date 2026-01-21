#version 460 core
    
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

vec3 get_position(uint index)
{
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]);
}

vec2 get_uv(uint index)
{
    return vec2(
        data[index].uv[0],
        data[index].uv[1]);
}

layout (location = 0) out vec4 out_frag_position;
layout (location = 1) out vec2 out_uv;

void main()
{
    gl_Position = vec4(get_position(gl_VertexID), 1.0);
    out_uv = get_uv(gl_VertexID);
}

