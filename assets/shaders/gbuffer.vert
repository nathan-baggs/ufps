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
    uint material_index;
};

struct MaterialData
{
    uint albedo_index;
    uint normal_index;
    uint specular_index;
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

layout(binding = 3, std430) readonly buffer materials {
    MaterialData material_data[];
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

layout (location = 0) out flat uint out_material_index;
layout (location = 1) out vec2 out_uv;
layout (location = 2) out vec4 out_frag_position;
layout (location = 3) out mat3 out_tbn;

void main()
{
    mat3 normal_mat = transpose(inverse(mat3(object_data[gl_DrawID].model)));

    out_frag_position = object_data[gl_DrawID].model * vec4(get_position(gl_VertexID), 1.0);
    gl_Position = projection * view * out_frag_position;
    out_material_index = object_data[gl_DrawID].material_index;
    out_uv = get_uv(gl_VertexID);

    vec3 t = normalize(vec3(object_data[gl_DrawID].model * vec4(get_tangent(gl_VertexID), 0.0)));
    vec3 b = normalize(vec3(object_data[gl_DrawID].model * vec4(get_bitangent(gl_VertexID), 0.0)));
    vec3 n = normalize(vec3(object_data[gl_DrawID].model * vec4(get_normal(gl_VertexID), 0.0)));
    out_tbn = mat3(t, b, n);
}


