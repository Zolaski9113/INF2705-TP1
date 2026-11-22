
#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in float zOrientation;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 size;

out ATTRIB_VS_OUT
{
    vec4 color;
    vec2 size;
    float angle;
} attribOut;

uniform mat4 modelView;

void main()
{
    // Position dans le repère de la caméra
    gl_Position = modelView * vec4(position, 1.0);

    attribOut.color = color;
    attribOut.size = size;
    attribOut.angle = zOrientation;
}
