#version 460 core

struct LineData
{
    float position[3];
    float colour[3];
};

layout(binding = 0, std430) readonly buffer lines {
    LineData data[];
};

layout(binding = 1, std430) readonly buffer camera {
    mat4 view;
    mat4 projection;
    float camera_position[3];
    float pad;
};

vec3 get_position(uint index)
{
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]);
}

vec3 get_colour(uint index)
{
    return vec3(
        data[index].colour[0],
        data[index].colour[1],
        data[index].colour[2]);
}

layout(location = 0) out vec3 out_colour;

void main()
{
    vec4 frag_position = vec4(get_position(gl_VertexID), 1.0);
    gl_Position = projection * view * frag_position;
    out_colour = get_colour(gl_VertexID);
}
