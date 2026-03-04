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

layout(location = 0) in vec3 in_colour;

layout(location = 0) out vec4 out_colour;

void main()
{
    out_colour = vec4(in_colour, 1.0);
}
