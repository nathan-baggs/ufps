#version 460 core
#extension GL_ARB_bindless_texture : require

layout(location = 0) in flat uvec2 in_albedo_tex_bindless_handle;
layout(location = 1) in flat uvec2 in_normal_tex_bindless_handle;
layout(location = 2) in flat uvec2 in_specular_tex_bindless_handle;
layout(location = 3) in vec2 in_uv;
layout(location = 4) in vec4 in_frag_position;
layout(location = 5) in mat3 in_tbn;

layout(location = 0) out vec4 out_colour;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_pos;
layout(location = 3) out vec4 out_specular;

void main()
{
    vec3 n;
    n.xy = texture(sampler2D(in_normal_tex_bindless_handle), in_uv).rg * 2.0 - 1.0;
    n.z = sqrt(max(1.0 - dot(n.xy, n.xy), 0.0));
    n = normalize(in_tbn * n);

    out_colour = vec4(texture(sampler2D(in_albedo_tex_bindless_handle), in_uv).rgb, 1.0);
    out_normal = vec4(n, 1.0);
    out_pos = in_frag_position;
    out_specular = vec4(texture(sampler2D(in_specular_tex_bindless_handle), in_uv).r, 0.0, 0.0, 1.0);
}
