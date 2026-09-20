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

struct Particle
{
    float position[3];
    float life;
    float velocity[3];
    float pad;
};

layout(binding = 2, std430) readonly buffer particle_buffer {
    Particle particles[];
};

layout(location = 0) in flat uint out_instance_id;

layout(location = 0) out vec4 out_colour;

void main()
{
    out_colour = vec4(1.0f);
}


