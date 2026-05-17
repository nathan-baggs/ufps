#version 460 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler, location = 0) uniform sampler2D u_input_texture;
layout(location = 1) uniform float u_filter_radius;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_colour;

void main()
{
    float x = u_filter_radius;
    float y = u_filter_radius;

    vec3 a = texture(u_input_texture, vec2(in_uv.x - x, in_uv.y + y)).rgb;
    vec3 b = texture(u_input_texture, vec2(in_uv.x,     in_uv.y + y)).rgb;
    vec3 c = texture(u_input_texture, vec2(in_uv.x + x, in_uv.y + y)).rgb;

    vec3 d = texture(u_input_texture, vec2(in_uv.x - x, in_uv.y)).rgb;
    vec3 e = texture(u_input_texture, vec2(in_uv.x,     in_uv.y)).rgb;
    vec3 f = texture(u_input_texture, vec2(in_uv.x + x, in_uv.y)).rgb;

    vec3 g = texture(u_input_texture, vec2(in_uv.x - x, in_uv.y - y)).rgb;
    vec3 h = texture(u_input_texture, vec2(in_uv.x,     in_uv.y - y)).rgb;
    vec3 i = texture(u_input_texture, vec2(in_uv.x + x, in_uv.y - y)).rgb;

    vec3 colour = e*4.0;
    colour += (b+d+f+h)*2.0;
    colour += (a+c+g+i);
    colour *= 1.0 / 16.0;

    out_colour = vec4(colour, 1.0f);
}
