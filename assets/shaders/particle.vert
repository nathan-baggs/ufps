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
layout(location = 1) out vec4 out_colour;

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

vec3 get_particle_velocity(uint index)
{
    return vec3(
        particles[index].velocity[0],
        particles[index].velocity[1],
        particles[index].velocity[2]);

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
        mat4 model = mat4(1.0f);

        vec3 x_direction = normalize(get_particle_velocity(gl_InstanceID));
        vec3 reference = abs(x_direction.y) < 0.99f
            ? vec3(0.0f, 1.0f, 0.0f)
            : vec3(1.0f, 0.0f, 0.0f);
        vec3 tangent = normalize(cross(x_direction, reference));
        vec3 bitangent = cross(x_direction, tangent);

        model[0] = vec4(x_direction * vec3(0.05f), 0.0f);
        model[1] = vec4(tangent * vec3(0.003f), 0.0f);
        model[2] = vec4(bitangent * vec3(0.003f), 0.0f);
        model[3] = vec4(get_particle_position(gl_InstanceID), 1.0);

        vec3 pos = get_position(gl_VertexID);

        vec4 out_frag_position = model * vec4(pos, 1.0);

        float alpha = pos.x < 0.0 ? 0.0f : 1.0f;
        out_colour = vec4(1.0f, 1.0f, 1.0f, alpha);

        gl_Position = projection * view * out_frag_position;
    }
}


