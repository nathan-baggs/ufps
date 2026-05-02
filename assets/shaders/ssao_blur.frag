#version 460 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler, location = 0) uniform sampler2D u_ssao_texture;
layout(bindless_sampler, location = 1) uniform sampler2D u_depth_texture;
layout(location = 2) uniform float u_width;
layout(location = 3) uniform float u_height;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 frag_colour;

void main()
{
    vec2 texel_size = 1.0 / vec2(u_width, u_height);

    float center_depth = texture(u_depth_texture, in_uv).r;

    float result = 0.0;
    float total_weight = 0.0;

    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texel_size;

            float sample_ao = texture(u_ssao_texture, in_uv + offset).r;
            float sample_depth = texture(u_depth_texture, in_uv + offset).r;

            float depth_diff = abs(center_depth - sample_depth);
            float weight = step(depth_diff, 0.05);

            result += sample_ao * weight;
            total_weight += weight;
        }
    }

    float final_ao = result / (total_weight + 0.0001f);

    frag_colour = vec4(final_ao, 0.0f, 0.0f, 1.0f);
}
