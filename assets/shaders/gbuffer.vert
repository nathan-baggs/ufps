#version 460 core

struct VertexData
{
    float position[3];
    float normal[3];
    float tangent[3];
    float bitangent[3];
    float uv[2];
};

struct ObjectData
{
    mat4 model;
    uint albedo_index;
    uint normal_index;
    uint specular_index;
    uint padding;
};

layout(binding = 0, std430) readonly buffer vertices {
    VertexData data[];
};

layout(binding = 1, std430) readonly buffer camera {
    mat4 view;
    mat4 projection;
    float camera_position[3];
    float pad;
};

layout(binding = 2, std430) readonly buffer objects {
    ObjectData object_data[];
};

vec3 get_position(uint index)
{
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]);
}

vec3 get_normal(uint index)
{
    return vec3(
        data[index].normal[0],
        data[index].normal[1],
        data[index].normal[2]);
}

vec3 get_tangent(uint index)
{
    return vec3(
        data[index].tangent[0],
        data[index].tangent[1],
        data[index].tangent[2]);
}

vec3 get_bitangent(uint index)
{
    return vec3(
        data[index].bitangent[0],
        data[index].bitangent[1],
        data[index].bitangent[2]);
}

vec2 get_uv(uint index)
{
    return vec2(
        data[index].uv[0],
        data[index].uv[1]);
}

layout(location = 0) out flat uint out_albedo_index;
layout(location = 1) out flat uint out_normal_index;
layout(location = 2) out flat uint out_specular_index;
layout(location = 3) out vec2 out_uv;
layout(location = 4) out vec4 out_frag_position;
layout(location = 5) out mat3 out_tbn;

void main()
{
    mat3 normal_mat = transpose(inverse(mat3(object_data[gl_DrawID].model)));

    out_frag_position = object_data[gl_DrawID].model * vec4(get_position(gl_VertexID), 1.0);
    gl_Position = projection * view * out_frag_position;

    out_albedo_index = object_data[gl_DrawID].albedo_index;
    out_normal_index = object_data[gl_DrawID].normal_index;
    out_specular_index = object_data[gl_DrawID].specular_index;
    out_uv = get_uv(gl_VertexID);

    vec3 t = normalize(normal_mat * get_tangent(gl_VertexID));
    vec3 b = normalize(normal_mat * get_bitangent(gl_VertexID));
    vec3 n = normalize(normal_mat * get_normal(gl_VertexID));
    out_tbn = mat3(t, b, n);
}
