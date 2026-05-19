#version 460 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler, location = 0) uniform sampler2D u_input_texture;
layout(location = 1) uniform float u_red_offset;
layout(location = 2) uniform float u_green_offset;
layout(location = 3) uniform float u_blue_offset;
layout(location = 4) uniform float u_strength;
layout(location = 5) uniform vec3 u_vignette_colour;
layout(location = 6) uniform float u_vignette_strength;
layout(location = 7) uniform float u_vignette_feather;
layout(location = 8) uniform float u_film_grain_strength;
layout(location = 9) uniform float u_frame_time;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_colour;

vec3 chromatic_aberration()
{
    vec2 direction = in_uv - vec2(0.5f, 0.5f);
    float dist = length(direction);
    float mask = smoothstep(u_strength, u_strength + 0.2, dist);
    vec2 shift = direction * mask;

    float red = texture(u_input_texture, in_uv + (shift * u_red_offset)).r;
    float green = texture(u_input_texture, in_uv + (shift * u_green_offset)).g;
    float blue = texture(u_input_texture, in_uv + (shift * u_blue_offset)).b;
    return vec3(red, green, blue);
}

vec3 vignette(vec3 colour)
{
    vec2 direction = in_uv - vec2(0.5f, 0.5f);
    float dist = length(direction);
    float vignette_amount = smoothstep(u_vignette_strength, u_vignette_strength + u_vignette_feather, dist);

    return mix(colour, u_vignette_colour, vignette_amount);
}

vec3 film_grain(vec3 colour)
{
    float rand = fract(10000 * sin((in_uv.x + in_uv.y * u_frame_time) * 3.14 / 180.0));
    rand *= u_film_grain_strength;

    return colour += rand;
}

void main()
{
    vec3 colour = chromatic_aberration();
    colour = vignette(colour);
    colour = film_grain(colour);
    out_colour = vec4(colour, 1.0);
}
