#version 460 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler, location = 0) uniform sampler2D u_mip_texture;
layout(bindless_sampler, location = 1) uniform sampler2D u_light_pass_texture;
layout(location = 2) uniform float u_mix_amount;
layout(location = 3) uniform float u_filter_radius;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_colour;

void main()
{
    float x = u_filter_radius;
    float y = u_filter_radius;

    vec3 a = texture(u_mip_texture, vec2(in_uv.x - x, in_uv.y + y)).rgb;
    vec3 b = texture(u_mip_texture, vec2(in_uv.x,     in_uv.y + y)).rgb;
    vec3 c = texture(u_mip_texture, vec2(in_uv.x + x, in_uv.y + y)).rgb;

    vec3 d = texture(u_mip_texture, vec2(in_uv.x - x, in_uv.y)).rgb;
    vec3 e = texture(u_mip_texture, vec2(in_uv.x,     in_uv.y)).rgb;
    vec3 f = texture(u_mip_texture, vec2(in_uv.x + x, in_uv.y)).rgb;

    vec3 g = texture(u_mip_texture, vec2(in_uv.x - x, in_uv.y - y)).rgb;
    vec3 h = texture(u_mip_texture, vec2(in_uv.x,     in_uv.y - y)).rgb;
    vec3 i = texture(u_mip_texture, vec2(in_uv.x + x, in_uv.y - y)).rgb;

    vec3 final_mip_colour = e*4.0;
    final_mip_colour += (b+d+f+h)*2.0;
    final_mip_colour += (a+c+g+i);
    final_mip_colour *= 1.0 / 16.0;

    vec3 light_pass_colour = texture(u_light_pass_texture, in_uv).rgb;

    out_colour = vec4(mix(light_pass_colour, final_mip_colour, u_mix_amount), 1.0f);
}
