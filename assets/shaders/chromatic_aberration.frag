#version 460 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler, location = 0) uniform sampler2D u_input_texture;
layout(location = 1) uniform float u_red_offset;
layout(location = 2) uniform float u_green_offset;
layout(location = 3) uniform float u_blue_offset;
layout(location = 4) uniform float u_strength;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_colour;

vec3 chromatic_aberration(vec3 colour)
{
    vec2 direction = in_uv - vec2(0.5f, 0.5f);
    float strength = length(direction) * u_strength;
    float red = texture(u_input_texture, vec2(0.5f, 0.5f) + (direction * vec2(1.0 + u_red_offset * strength))).r;
    float green = texture(u_input_texture, vec2(0.5f, 0.5f) + (direction * vec2(1.0 + u_green_offset * strength))).g;
    float blue = texture(u_input_texture, vec2(0.5f, 0.5f) + (direction * vec2(1.0 + u_blue_offset * strength))).b;
    return vec3(red, green, blue);
}

void main()
{
    vec3 in_colour = texture(u_input_texture, in_uv).rgb;

    out_colour = vec4(chromatic_aberration(in_colour), 1.0);
}
