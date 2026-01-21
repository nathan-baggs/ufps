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

layout(binding = 4, std430) readonly buffer textures_buffer {
    sampler2D textures[];
};

layout(location = 0) in flat uint in_material_index;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_frag_position;
layout(location = 3) in mat3 in_tbn;

layout(location = 0) out vec4 out_colour;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_pos;
layout(location = 3) out vec4 out_specular;

void main()
{
    uint albedo_tex_index = material_data[in_material_index].albedo_index;
    uint normal_tex_index = material_data[in_material_index].normal_index;
    uint specular_tex_index = material_data[in_material_index].specular_index;

    vec3 n = texture(textures[normal_tex_index], in_uv).xyz;
    n = (n * 2.0) - 1.0;
    n = normalize(in_tbn * n);

    out_colour = vec4(texture(textures[albedo_tex_index], in_uv).rgb, 1.0);
    out_normal = vec4(n, 1.0);
    out_pos = in_frag_position;
    out_specular = vec4(texture(textures[specular_tex_index], in_uv).r, 0.0, 0.0, 1.0);
}

