#version 400 core

// point_mode : chaque sommet généré devient un point (la base d'un brin) au lieu de triangles
layout(triangles, equal_spacing, ccw, point_mode) in;

void main()
{
    // Coordonnées barycentriques : interpolation des 3 sommets de la patch (position monde)
    gl_Position = gl_TessCoord.x * gl_in[0].gl_Position
                + gl_TessCoord.y * gl_in[1].gl_Position
                + gl_TessCoord.z * gl_in[2].gl_Position;
}
