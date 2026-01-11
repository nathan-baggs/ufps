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

layout(binding = 4, std430) readonly buffer lights {
    float ambient_colour[3];
    float point_light_pos[3];
    float point_light_colour[3];
    float point_light_attenuation[3];
    float point_light_specular_power;
};

layout(binding = 5, std430) readonly buffer textures_buffer {
    sampler2D textures[];
};

layout(location = 0) in flat uint in_material_index;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_frag_position;
layout(location = 3) in mat3 in_tbn;

layout(location = 0) out vec4 out_colour;

vec3 calc_point(vec3 frag_position, vec3 n)
{
    uint specular_tex_index = material_data[in_material_index].specular_index;

    vec3 pos = vec3(point_light_pos[0], point_light_pos[1], point_light_pos[2]);
    vec3 colour = vec3(point_light_colour[0], point_light_colour[1], point_light_colour[2]);
    vec3 attenuation = vec3(point_light_attenuation[0], point_light_attenuation[1], point_light_attenuation[2]);

    float distance = length(pos - frag_position);
    float att = 1.0 / (attenuation.x + (attenuation.y * distance) + (attenuation.z * (distance * distance)));

    vec3 light_dir = normalize(pos - frag_position);
    float diff = max(dot(n, light_dir), 0.0);

    vec3 camera_pos = vec3(camera_position[0], camera_position[1], camera_position[2]);

    vec3 reflect_dir = reflect(-light_dir, n);
    float spec = pow(max(dot(normalize(camera_pos - frag_position), reflect_dir), 0.0), point_light_specular_power) * texture(textures[specular_tex_index], in_uv).r;

    return (diff + spec) * att * colour;
}

void main()
{
    uint albedo_tex_index = material_data[in_material_index].albedo_index;
    uint normal_tex_index = material_data[in_material_index].normal_index;

    vec3 n = texture(textures[normal_tex_index], in_uv).xyz;
    n = (n * 2.0) - 1.0;
    n = normalize(in_tbn * n);

    vec3 albedo = texture(textures[albedo_tex_index], in_uv).rgb;
    vec3 amb_colour = vec3(ambient_colour[0], ambient_colour[1], ambient_colour[2]);
    vec3 point_colour = calc_point(in_frag_position.xyz, n);

    out_colour = vec4(albedo * (amb_colour + point_colour), 1.0);
}

