#version 460 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler, location = 0) uniform sampler2D u_input_texture;
layout(location = 1) uniform vec2 u_src_resolution;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_colour;

void main()
{
    vec2 src_texel_size = 1.0f / u_src_resolution;
    float x = src_texel_size.x;
    float y = src_texel_size.y;

    vec3 a = texture(u_input_texture, vec2(in_uv.x - 2*x, in_uv.y + 2*y)).rgb;
    vec3 b = texture(u_input_texture, vec2(in_uv.x,       in_uv.y + 2*y)).rgb;
    vec3 c = texture(u_input_texture, vec2(in_uv.x + 2*x, in_uv.y + 2*y)).rgb;

    vec3 d = texture(u_input_texture, vec2(in_uv.x - 2*x, in_uv.y)).rgb;
    vec3 e = texture(u_input_texture, vec2(in_uv.x,       in_uv.y)).rgb;
    vec3 f = texture(u_input_texture, vec2(in_uv.x + 2*x, in_uv.y)).rgb;

    vec3 g = texture(u_input_texture, vec2(in_uv.x - 2*x, in_uv.y - 2*y)).rgb;
    vec3 h = texture(u_input_texture, vec2(in_uv.x,       in_uv.y - 2*y)).rgb;
    vec3 i = texture(u_input_texture, vec2(in_uv.x + 2*x, in_uv.y - 2*y)).rgb;

    vec3 j = texture(u_input_texture, vec2(in_uv.x - x, in_uv.y + y)).rgb;
    vec3 k = texture(u_input_texture, vec2(in_uv.x + x, in_uv.y + y)).rgb;
    vec3 l = texture(u_input_texture, vec2(in_uv.x - x, in_uv.y - y)).rgb;
    vec3 m = texture(u_input_texture, vec2(in_uv.x + x, in_uv.y - y)).rgb;

    vec3 colour = e*0.125;
    colour += (a+c+g+i)*0.03125;
    colour += (b+d+f+h)*0.0625;
    colour += (j+k+l+m)*0.125;

    out_colour = vec4(colour, 1.0);
}
