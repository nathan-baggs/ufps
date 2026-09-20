#version 460 core

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

layout(location = 0) out flat uint out_instance_id;

vec3 get_position(uint index)
{
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]);
}

vec3 get_particle_position(uint index)
{
    return vec3(
        particles[index].position[0],
        particles[index].position[1],
        particles[index].position[2]);

}

float get_life(uint index)
{
    return particles[index].life;
}

void main()
{
    out_instance_id = gl_InstanceID;

    if (get_life(gl_InstanceID) <= 0.0f)
    {
        gl_Position = vec4(100.0f);
    }
    else
    {
        mat4 model = mat4(0.01f);
        model[3] = vec4(get_particle_position(gl_InstanceID), 1.0);

        vec4 out_frag_position = model * vec4(get_position(gl_VertexID), 1.0);
        gl_Position = projection * view * out_frag_position;
    }
}


