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

struct PointLight
{
    float position[3];
    float colour[3];
    float attenuation[3];
    float specular_power;
    float intensity;
    float pad;
};

layout(binding = 0, std430) readonly buffer vertices {
    VertexData data[];
};

layout(binding = 1, std430) readonly buffer textures_buffer {
    sampler2D textures[];
};

layout(binding = 2, std430) readonly buffer lights {
    float ambient_colour[3];
    uint num_point_lights;
    PointLight point_lights[];
};

layout(binding = 3, std430) readonly buffer camera {
    mat4 view;
    mat4 projection;
    float camera_position[3];
    float pad;
};

layout(location = 0) uniform uint albedo_tex_index;
layout(location = 1) uniform uint normal_tex_index;
layout(location = 2) uniform uint position_tex_index;
layout(location = 3) uniform uint specular_tex_index;

layout(location = 0) in vec4 in_frag_position;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_colour;

vec3 calculate_point_light(
    PointLight light,
    vec3 normal,
    vec3 frag_pos,
    vec3 albedo,
    float specular)
{
    vec3 point_pos = vec3(light.position[0], light.position[1], light.position[2]);
    vec3 point_colour = vec3(light.colour[0], light.colour[1], light.colour[2]) * vec3(light.intensity);
    vec3 point_attenuation = vec3(light.attenuation[0], light.attenuation[1], light.attenuation[2]);

    vec3 light_dir = normalize(point_pos - frag_pos);
    vec3 camera_pos = vec3(camera_position[0], camera_position[1], camera_position[2]);
    vec3 view_dir = normalize(camera_pos - frag_pos);

    float diffuse_factor = max(dot(normal, light_dir), 0.0);
    vec3 halfway_dir = normalize(light_dir + view_dir);

    float spec_factor = pow(max(dot(normal, halfway_dir), 0.0), light.specular_power) * specular;

    diffuse_factor *= (1.0 - spec_factor);

    float distance = length(point_pos - frag_pos);
    float att = 1.0 / (point_attenuation.x + (point_attenuation.y * distance) + (point_attenuation.z * (distance * distance)));

    vec3 diffuse_light = diffuse_factor * albedo;
    vec3 specular_light = vec3(spec_factor); // Highlight remains the color of the light

    return (diffuse_light + specular_light) * att * point_colour;
}

void main()
{
    vec3 albedo = texture(textures[albedo_tex_index], in_uv).rgb;
    vec3 normal = texture(textures[normal_tex_index], in_uv).xyz;
    vec3 frag_pos = texture(textures[position_tex_index], in_uv).xyz;
    float specular = texture(textures[specular_tex_index], in_uv).r;

    vec3 ambient = vec3(ambient_colour[0], ambient_colour[1], ambient_colour[2]);

    vec3 result = ambient * albedo;

    for (uint i = 0; i < num_point_lights; i++)
    {
        result += calculate_point_light(point_lights[i], normal, frag_pos, albedo, specular);
    }

    out_colour = vec4(result, 1.0);
}
