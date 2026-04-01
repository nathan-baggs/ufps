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

layout(binding = 2, std430) readonly buffer average_buffer {
    float average;
};

layout(location = 0) uniform uint u_input_tex_index;
layout(location = 1) uniform float u_P;
layout(location = 2) uniform float u_a;
layout(location = 3) uniform float u_m;
layout(location = 4) uniform float u_l;
layout(location = 5) uniform float u_c;
layout(location = 6) uniform float u_b;
layout(location = 7) uniform float u_gamma;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_colour;

vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b)
{
    float l0 = ((P - m) * l) / a;
    float L0 = m - m / a;
    float L1 = m + (1.0 - m) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    vec3 w0 = vec3(1.0 - smoothstep(0.0, m, x));
    vec3 w2 = vec3(step(m + l0, x));
    vec3 w1 = vec3(1.0 - w0 - w2);

    vec3 T = vec3(m * pow(x / m, vec3(c)) + b);
    vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
    vec3 L = vec3(m + a * (x - m));

    return T * w0 + L * w1 + S * w2;
}

void main()
{
    vec3 in_colour = texture(textures[u_input_tex_index], in_uv).rgb;

    in_colour *= (0.18 / max(average, 0.0001));

    vec3 tone_mapped_colour = uchimura(in_colour, u_P, u_a, u_m, u_l, u_c, u_b);

    vec3 gamma_corrected = pow(tone_mapped_colour, vec3(1.0 / u_gamma));

    out_colour = vec4(gamma_corrected, 1.0);
}
