
#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in ATTRIB_VS_OUT
{
    vec4 color;
    vec2 size;
    float angle;
} attribIn[];

out ATTRIB_GS_OUT
{
    vec4 color;
    vec2 texCoord;
} attribOut;

uniform mat4 projection;

void emitCorner(vec2 corner, vec2 uv)
{
    float angle = attribIn[0].angle;

    mat2 rotation = mat2(
        cos(angle), sin(angle),
        -sin(angle), cos(angle)
    );

    vec2 offset = rotation
                * (corner * attribIn[0].size);

    gl_Position = projection * (
        gl_in[0].gl_Position
        + vec4(offset, 0.0, 0.0)
    );

    attribOut.color = attribIn[0].color;
    attribOut.texCoord = uv;

    EmitVertex();
}

void main()
{
    emitCorner(
        vec2(-0.5, -0.5),
        vec2(0.0, 0.0)
    );

    emitCorner(
        vec2(0.5, -0.5),
        vec2(1.0, 0.0)
    );

    emitCorner(
        vec2(-0.5, 0.5),
        vec2(0.0, 1.0)
    );

    emitCorner(
        vec2(0.5, 0.5),
        vec2(1.0, 1.0)
    );

    EndPrimitive();
}
