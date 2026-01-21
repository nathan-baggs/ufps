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

layout(binding = 2, std430) readonly buffer lights {
    float ambient_colour[3];
    float point_light_pos[3];
    float point_light_colour[3];
    float point_light_attenuation[3];
    float point_light_specular_power;
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

void main()
{
    vec3 albedo = texture(textures[albedo_tex_index], in_uv).rgb;
    vec3 normal = texture(textures[normal_tex_index], in_uv).xyz;
    vec3 frag_pos = texture(textures[position_tex_index], in_uv).xyz;
    float specular = texture(textures[specular_tex_index], in_uv).r;

    vec3 point_pos = vec3(point_light_pos[0], point_light_pos[1], point_light_pos[2]);
    vec3 point_colour = vec3(point_light_colour[0], point_light_colour[1], point_light_colour[2]);
    vec3 point_attenuation = vec3(point_light_attenuation[0], point_light_attenuation[1], point_light_attenuation[2]);

    vec3 ambient = vec3(ambient_colour[0], ambient_colour[1], ambient_colour[2]);
    vec3 light_dir = normalize(point_pos - frag_pos);
    float diffuse = max(dot(normal, light_dir), 0.0);

    vec3 camera_pos = vec3(camera_position[0], camera_position[1], camera_position[2]);

    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(normalize(camera_pos - frag_pos), reflect_dir), 0.0), point_light_specular_power) * specular;

    float distance = length(point_pos - frag_pos);
    float att = 1.0 / (point_attenuation.x + (point_attenuation.y * distance) + (point_attenuation.z * (distance * distance)));
    
    vec3 lighting = (ambient * albedo) + ((diffuse + spec) * att) * point_colour * albedo;

    out_colour = vec4(lighting, 1.0);
}


