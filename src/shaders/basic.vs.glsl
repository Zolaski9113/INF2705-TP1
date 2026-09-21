#version 330 core

// TODO: Définir les entrées et sorties pour donner une position
//       et couleur à chaque vertex.

layout(location = 0) in vec2 aPos;     // attribut 0, 2 floats
layout(location = 1) in vec3 aRgb;    // ↔ attribut 1, 3 float
out vec4 laCouleur;

void main(){
    gl_Position = vec4(aPos, 0.0, 1.0);
    laCouleur = vec4(aRgb, 1.0);
}