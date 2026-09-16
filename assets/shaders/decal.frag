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

layout(binding = 1, std430) readonly buffer camera {
    mat4 view;
    mat4 projection;
    float camera_position[3];
    float pad;
};

layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 inv_model;
layout(bindless_sampler, location = 2) uniform sampler2D u_position_texture;
layout(bindless_sampler, location = 3) uniform sampler2D u_decal_texture;

layout(location = 0) out vec4 out_colour;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_pos;
layout(location = 3) out vec4 out_specular;
layout(location = 4) out vec4 out_emissive;

void main()
{
    vec3 world_frag_pos = texelFetch(u_position_texture, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 decal_frag_pos = (inv_model * vec4(world_frag_pos, 1.0f)).xyz;

    if (
        clamp(decal_frag_pos.x, -1.0f, 1.0f) != decal_frag_pos.x ||
        clamp(decal_frag_pos.y, -1.0f, 1.0f) != decal_frag_pos.y ||
        clamp(decal_frag_pos.z, -1.0f, 1.0f) != decal_frag_pos.z)
    {
        discard;
    }

    vec2 decal_uv = vec2(decal_frag_pos.x, decal_frag_pos.z);
    decal_uv = (decal_uv + vec2(1.0f)) / vec2(2.0f);

    vec4 decal_colour = texture(u_decal_texture, decal_uv);
    if (decal_colour.a < 0.001f)
    {
        discard;
    }

    out_colour = texture(u_decal_texture, decal_uv);
    out_normal = vec4(0.0f);
    out_pos = vec4(0.0f);
    out_specular = vec4(0.0f);
    out_emissive = vec4(0.0f);
}

