#version 330 core

// TODO: Définir les entrées et sorties pour donner une position
//       et couleur à chaque vertex.
//       Les vertices sont transformées à l'aide d'une matrice mvp
//       pour les placer à l'écran.

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

uniform mat4 mvp;

out vec3 frag_color;

void main()
{
    gl_Position = mvp * vec4(in_position, 1.0);
    frag_color = in_color;
}
