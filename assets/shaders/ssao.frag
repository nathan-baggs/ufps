#version 460 core
#extension GL_ARB_bindless_texture : require

layout(binding = 1, std430) readonly buffer camera {
    mat4 view;
    mat4 projection;
    float camera_position[3];
    float pad;
};

layout(binding = 2, std430) readonly buffer ssao_samples {
    vec4 samples[];
};

layout(bindless_sampler, location = 0) uniform sampler2D u_normal_texture;
layout(bindless_sampler, location = 1) uniform sampler2D u_position_texture;
layout(location = 2) uniform float u_width;
layout(location = 3) uniform float u_height;
layout(location = 4) uniform uint u_sample_count;
layout(location = 5) uniform float u_radius;
layout(location = 6) uniform float u_bias;
layout(location = 7) uniform float u_power;
layout(bindless_sampler, location = 8) uniform sampler2D u_noise_texture;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 frag_colour;

void main()
{
    const vec2 uv = vec2(in_uv.x, 1.0 - in_uv.y);

    vec3 normal = normalize((view * vec4(texture(u_normal_texture, in_uv).xyz, 0.0)).xyz);
    vec3 frag_pos = (view * vec4(texture(u_position_texture, in_uv).xyz, 1.0)).xyz;

    const vec2 size = vec2(u_width, u_height);

    const int x = int(uv.x * size.x) % 4;
    const int y = int(uv.y * size.y) % 4;
    const int index = (y * 4) + x;
    const vec2 noise_scale = vec2(u_width / 4.0, u_height / 4.0);
    const vec3 rand = normalize(texture(u_noise_texture, noise_scale).xyz);

    const vec3 tangent = normalize(rand - normal * dot(rand, normal));
    const vec3 bitangent = cross(normal, tangent);
    const mat3 tbn = mat3(tangent, bitangent, normal);

    float occlusion = 0.0f;

    for (int i = 0; i < u_sample_count; ++i)
    {
        vec3 sample_pos = tbn * samples[i].xyz;
        sample_pos = frag_pos + sample_pos * u_radius;

        vec4 offset = projection * vec4(sample_pos, 1.0f);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5f + 0.5f;

        const float sample_depth = (view * vec4(texture(u_position_texture, vec2(offset.x, offset.y)).xyz, 1.0)).z;
        const float range_check = smoothstep(0.0, 1.0, u_radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= sample_pos.z + u_bias ? 1.0 : 0.0) * range_check;
    }

    occlusion = 1.0f - (occlusion / u_sample_count);
    occlusion = pow(occlusion, u_power);

    frag_colour = vec4(occlusion, 0.0, 0.0, 1.0);
}
