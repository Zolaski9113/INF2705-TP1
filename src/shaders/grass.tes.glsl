#version 400 core

// TODO: À remplir
layout(triangles, equal_spacing, ccw, point_mode) in;

out ATTRIBS_TES_OUT
{
    // TODO: Ajouter les attributs si nécessaire
    vec3 worldPos;
} attribsOut;

uniform mat4 mvp;

void main()
{
    // TODO: Effectuer l'interpolation des points avec gl_TessCoord
    vec4 pos = gl_TessCoord.x * gl_in[0].gl_Position
             + gl_TessCoord.y * gl_in[1].gl_Position
             + gl_TessCoord.z * gl_in[2].gl_Position;

    attribsOut.worldPos = pos.xyz;
    gl_Position = pos;
}
